#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include "unitree_go/msg/sport_mode_state.hpp"
#include "unitree_go/msg/low_state.hpp"

class Driver : public rclcpp::Node
{
public:
    Driver() : Node("driver"), body_height_(0.30)
    {
        // Publishers
        odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>("odom", 10);
        joint_state_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("joint_states", 10);

        // TF broadcaster
        tf_bro_ = std::make_unique<tf2_ros::TransformBroadcaster>(this);

        // Subscribers
        state_sub_ = this->create_subscription<unitree_go::msg::SportModeState>(
            "sportmodestate", 10,
            std::bind(&Driver::state_callback, this, std::placeholders::_1));

        pose_sub_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
            "body_pose", 10,
            std::bind(&Driver::pose_callback, this, std::placeholders::_1));

        low_state_sub_ = this->create_subscription<unitree_go::msg::LowState>(
            "lowstate", 10,
            std::bind(&Driver::low_state_cb, this, std::placeholders::_1));
    }

private:
    // Publishers
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_state_pub_;

    // TF broadcaster
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_bro_;

    // Subscribers
    rclcpp::Subscription<unitree_go::msg::SportModeState>::SharedPtr state_sub_;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr pose_sub_;
    rclcpp::Subscription<unitree_go::msg::LowState>::SharedPtr low_state_sub_;

    // Member variables
    double body_height_;

    // Callback for sport mode state
    void state_callback(const unitree_go::msg::SportModeState::SharedPtr state_msg)
    {
        body_height_ = state_msg->body_height + 0.057 - 0.046825;

        // Publish odometry
        auto odom_msg = nav_msgs::msg::Odometry();
        odom_msg.header.stamp = this->now();
        odom_msg.header.frame_id = "odom";
        odom_msg.child_frame_id = "base_link";
        odom_msg.pose.pose.position.x = state_msg->position[0];
        odom_msg.pose.pose.position.y = state_msg->position[1];
        odom_msg.pose.pose.position.z = body_height_;
        odom_pub_->publish(odom_msg);
    }

    // Callback for body pose
    void pose_callback(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
    {
        geometry_msgs::msg::TransformStamped tf;
        tf.header = msg->header;
        tf.child_frame_id = "base_link";
        tf.transform.translation.x = msg->pose.position.x;
        tf.transform.translation.y = msg->pose.position.y;
        tf.transform.translation.z = msg->pose.position.z - body_height_;
        tf.transform.rotation = msg->pose.orientation;
        tf_bro_->sendTransform(tf);
    }

    // Callback for low state (joint states)
    void low_state_cb(const unitree_go::msg::LowState::SharedPtr msg)
    {
        auto joint_state = sensor_msgs::msg::JointState();
        joint_state.header.stamp = this->now();

        // Fill joint names and positions from low state
        // GO2 has 12 joints: 4 legs x 3 joints each
        const char* joint_names[12] = {
            "FR_hip_joint", "FR_thigh_joint", "FR_calf_joint",
            "FL_hip_joint", "FL_thigh_joint", "FL_calf_joint",
            "RR_hip_joint", "RR_thigh_joint", "RR_calf_joint",
            "RL_hip_joint", "RL_thigh_joint", "RL_calf_joint"
        };

        for (size_t i = 0; i < 12; i++) {
            joint_state.name.push_back(joint_names[i]);
            if (i < msg->motor_state.size()) {
                joint_state.position.push_back(msg->motor_state[i].q);
                joint_state.velocity.push_back(msg->motor_state[i].dq);
                joint_state.effort.push_back(msg->motor_state[i].tau_est);
            } else {
                joint_state.position.push_back(0.0);
                joint_state.velocity.push_back(0.0);
                joint_state.effort.push_back(0.0);
            }
        }

        joint_state_pub_->publish(joint_state);
    }
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Driver>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}