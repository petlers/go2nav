/*
 description: 3D点云压扁成2D
 */

#include "pointcloud_to_laserscan/pointcloud_to_laserscan_node.hpp"

#include <chrono>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include "sensor_msgs/point_cloud2_iterator.hpp"
#include "tf2_sensor_msgs/tf2_sensor_msgs.hpp"
#include "tf2_ros/create_timer_ros.h"
#include "tf2/LinearMath/Transform.h"
#include "geometry_msgs/msg/transform_stamped.hpp"

using std::placeholders::_1;

namespace pointcloud_to_laserscan
{

PointCloudToLaserScanNode::PointCloudToLaserScanNode(const rclcpp::NodeOptions & options)
: rclcpp::Node("pointcloud_to_laserscan", options)
{

  target_frame_ = this->declare_parameter("target_frame", "");
  tolerance_ = this->declare_parameter("transform_tolerance", 0.5);
  input_queue_size_ = this->declare_parameter("queue_size", 50);
  min_height_ = this->declare_parameter("min_height", std::numeric_limits<double>::min());
  max_height_ = this->declare_parameter("max_height", std::numeric_limits<double>::max());
  angle_min_ = this->declare_parameter("angle_min", -M_PI);
  angle_max_ = this->declare_parameter("angle_max", M_PI);
  angle_increment_ = this->declare_parameter("angle_increment", M_PI / 180.0);
  scan_time_ = this->declare_parameter("scan_time", 1.0 / 10.0);
  range_min_ = this->declare_parameter("range_min", 0.0);
  range_max_ = this->declare_parameter("range_max", std::numeric_limits<double>::max());
  inf_epsilon_ = this->declare_parameter("inf_epsilon", 1.0);
  use_inf_ = this->declare_parameter("use_inf", true);

  rclcpp::QoS qos = rclcpp::SensorDataQoS();
  qos.reliable();
  qos.durability_volatile();

  pub_ = this->create_publisher<sensor_msgs::msg::LaserScan>("scan", qos);

  // 初始化订阅（支持 remap！）
  sub_.subscribe(this, "cloud_in", qos.get_rmw_qos_profile());

  if (!target_frame_.empty()) {
    tf2_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
    auto timer_interface = std::make_shared<tf2_ros::CreateTimerROS>(
      this->get_node_base_interface(), this->get_node_timers_interface());
    tf2_->setCreateTimerInterface(timer_interface);
    tf2_listener_ = std::make_unique<tf2_ros::TransformListener>(*tf2_);
    message_filter_ = std::make_unique<MessageFilter>(sub_, *tf2_, target_frame_, input_queue_size_,
      this->get_node_logging_interface(), this->get_node_clock_interface());
    message_filter_->registerCallback(std::bind(&PointCloudToLaserScanNode::cloudCallback, this, _1));
  } else {
    sub_.registerCallback(std::bind(&PointCloudToLaserScanNode::cloudCallback, this, _1));
  }

  alive_.store(true);
  subscription_listener_thread_ = std::thread(
    std::bind(&PointCloudToLaserScanNode::subscriptionListenerThreadLoop, this));
}

PointCloudToLaserScanNode::~PointCloudToLaserScanNode()
{
  alive_.store(false);
  if (subscription_listener_thread_.joinable()) {
    subscription_listener_thread_.join();
  }
  sub_.unsubscribe();
}

void PointCloudToLaserScanNode::subscriptionListenerThreadLoop()
{
  rclcpp::Context::SharedPtr context = this->get_node_base_interface()->get_context();
  const std::chrono::milliseconds timeout(100);

  while (rclcpp::ok(context) && alive_.load()) {
    // 永远订阅，不做懒加载 → 保证 remap 生效
    if (!sub_.getSubscriber()) {
      rclcpp::SensorDataQoS qos;
      qos.keep_last(input_queue_size_);
      qos.reliable();
      sub_.subscribe(this, "cloud_in", qos.get_rmw_qos_profile());
    }

    rclcpp::Event::SharedPtr event = this->get_graph_event();
    this->wait_for_graph_change(event, timeout);
  }

  sub_.unsubscribe();
}

void PointCloudToLaserScanNode::cloudCallback(sensor_msgs::msg::PointCloud2::ConstSharedPtr cloud_msg)
{
  auto scan_msg = std::make_unique<sensor_msgs::msg::LaserScan>();
  scan_msg->header.stamp = cloud_msg->header.stamp;
  scan_msg->header.frame_id = target_frame_.empty() ? cloud_msg->header.frame_id : target_frame_;

  if (!target_frame_.empty()) {
    scan_msg->header.frame_id = target_frame_;
  }

  scan_msg->angle_min = angle_min_;
  scan_msg->angle_max = angle_max_;
  scan_msg->angle_increment = angle_increment_;
  scan_msg->time_increment = 0.0;
  scan_msg->scan_time = scan_time_;
  scan_msg->range_min = range_min_;
  scan_msg->range_max = range_max_;

  uint32_t ranges_size = std::ceil(
    (scan_msg->angle_max - scan_msg->angle_min) / scan_msg->angle_increment);

  if (use_inf_) {
    scan_msg->ranges.assign(ranges_size, std::numeric_limits<double>::infinity());
  } else {
    scan_msg->ranges.assign(ranges_size, scan_msg->range_max + inf_epsilon_);
  }

  const int max_retries = 3;
  if (scan_msg->header.frame_id != cloud_msg->header.frame_id)
  {
    for (int i = 0; i < max_retries; ++i)
    {
      try {
        auto cloud_transformed = std::make_shared<sensor_msgs::msg::PointCloud2>();
        tf2_->transform(*cloud_msg, *cloud_transformed, scan_msg->header.frame_id, tf2::durationFromSec(tolerance_));
        cloud_msg = cloud_transformed;
        break;
      }
      catch (const tf2::TransformException& ex)
      {
        if (i == max_retries - 1) {
          RCLCPP_ERROR(this->get_logger(), "Transform failed: %s", ex.what());
          return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }
  }

  for (sensor_msgs::PointCloud2ConstIterator<float> iter_x(*cloud_msg, "x"),
    iter_y(*cloud_msg, "y"), iter_z(*cloud_msg, "z");
    iter_x != iter_x.end(); ++iter_x, ++iter_y, ++iter_z)
  {
    if (std::isnan(*iter_x) || std::isnan(*iter_y) || std::isnan(*iter_z)) continue;
    if (*iter_z > max_height_ || *iter_z < min_height_) continue;

    double range = hypot(*iter_x, *iter_y);
    if (range < range_min_ || range > range_max_) continue;

    double angle = atan2(*iter_y, *iter_x);
    if (angle < scan_msg->angle_min || angle > scan_msg->angle_max) continue;

    int index = static_cast<int>((angle - scan_msg->angle_min) / scan_msg->angle_increment);
    index = std::clamp(index, 0, (int)scan_msg->ranges.size()-1);

    if (range < scan_msg->ranges[index]) {
      scan_msg->ranges[index] = range;
    }
  }

  pub_->publish(std::move(scan_msg));
}

}  // namespace pointcloud_to_laserscan

#include "rclcpp_components/register_node_macro.hpp"
RCLCPP_COMPONENTS_REGISTER_NODE(pointcloud_to_laserscan::PointCloudToLaserScanNode)
