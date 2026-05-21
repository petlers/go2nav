/* 点云累加 + 高度过滤 */

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "vector"
#include "cstring"

using namespace std::chrono_literals;

class CloudAccumulator : public rclcpp::Node
{
public:
    CloudAccumulator() : Node("cloud_accumulator")
    {
        RCLCPP_INFO(this->get_logger(),"Cloud accumulator node started.");

        rclcpp::QoS qos = rclcpp::SensorDataQoS();
        qos.reliable();

        sub_ = this->create_subscription<sensor_msgs::msg::PointCloud2>(
            "/utlidar/cloud_deskewed",
            rclcpp::SensorDataQoS(),
            std::bind(&CloudAccumulator::cloud_callback, this, std::placeholders::_1)

        );

        pub_ = this->create_publisher<sensor_msgs::msg::PointCloud2>(
            "/utlidar/cloud_accumulated",
            qos
        );

        timer_ = this->create_wall_timer(
            25ms,
            std::bind(&CloudAccumulator::timer_callback, this)
        );

    }

private:
    rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr sub_;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;

    std::vector<sensor_msgs::msg::PointCloud2::ConstSharedPtr> clouds_;
    sensor_msgs::msg::PointCloud2 accumulated_cloud_;

    const size_t max_clouds_ = 25;
    const float min_height = 0.2f;
    const float max_height = 0.8f;






    void cloud_callback(const sensor_msgs::msg::PointCloud2::ConstSharedPtr cloud_msg)
    {
        // 推送新的点云数据
        clouds_.push_back(cloud_msg);
        // 如果点云的数量超过max_clouds_，则删除最老的点云数据
        if (clouds_.size() > max_clouds_)
        {
            clouds_.erase(clouds_.begin());
        }
        // 如果没有点云数据，则返回
        if (clouds_.empty())
        {
            return;
        }

        // 合并所有点云（合理帧数，保证不稀疏也不滞后导致虚影）
        auto merged_cloud = merged_clouds();

        // 高度过滤
        auto filtered_cloud = filter_cloud(*merged_cloud);

        // 更新积累的点云
        accumulated_cloud_ = filtered_cloud;
        accumulated_cloud_.header.frame_id = "odom";

    }


    std::shared_ptr<sensor_msgs::msg::PointCloud2> merged_clouds()
    {
        auto merged_cloud = std::make_shared<sensor_msgs::msg::PointCloud2>();

        *merged_cloud = *clouds_[0];

        for (size_t i = 1; i < clouds_.size(); i++)
        {
            merged_cloud->width += clouds_[i] -> width;
            merged_cloud->row_step += clouds_[i] -> row_step;
            merged_cloud->data.insert(merged_cloud->data.end(),
                    clouds_[i]->data.begin(), 
                    clouds_[i]->data.end()
                );

        }
        return merged_cloud;

    }

    sensor_msgs::msg::PointCloud2 filter_cloud(const sensor_msgs::msg::PointCloud2 &cloud)
    {
        sensor_msgs::msg::PointCloud2 filtered_cloud;
        filtered_cloud.header = cloud.header;
        filtered_cloud.height = 1;
        filtered_cloud.fields = cloud.fields;
        filtered_cloud.is_bigendian = cloud.is_bigendian;
        filtered_cloud.point_step = cloud.point_step;
        filtered_cloud.row_step = 0;
        filtered_cloud.is_dense = false;

        for (size_t i = 0; i<cloud.width * cloud.height; i++)
        {
            float z;
            memcpy(&z, &cloud.data[i * cloud.point_step + cloud.fields[2].offset], sizeof(float));

            if (z >= min_height && z <= max_height)
            {
                filtered_cloud.data.insert(filtered_cloud.data.end(), &cloud.data[i * cloud.point_step],&cloud.data[(i+1) * cloud.point_step]);
                filtered_cloud.row_step += cloud.point_step;
                filtered_cloud.width++;
            }
        }
        return filtered_cloud;

    }

    void timer_callback()
    {
        if(!accumulated_cloud_.data.empty())
        {
            accumulated_cloud_.header.stamp = clouds_.back()->header.stamp;
            pub_->publish(accumulated_cloud_); 
        }
    }


};

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CloudAccumulator>());
    rclcpp::shutdown();

    return 0;
}