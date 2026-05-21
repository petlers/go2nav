from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():

    go2_slam_pkg = get_package_share_directory("go2_slam")
    config_dir = os.path.join(go2_slam_pkg, "config")
    
    use_sim_time = LaunchConfiguration('use_sim_time')

    cartographer_node = Node(
        package="cartographer_ros",
        executable="cartographer_node",
        name="cartographer_node",
        output="screen",
        parameters=[{'use_sim_time': use_sim_time}],
        arguments=[
            "-configuration_directory", "/home/phj/Desktop/go2nav_ws/src/go2_slam/config",
            "-configuration_basename", "backpack_2d.lua",
        ],
        remappings=[
            ("scan", "/scan"),
            ("imu", "/imu"),
            # ("odom", "/odom"),
        ]
    )

    occupancy_grid_node = Node(
        package="cartographer_ros",
        executable="cartographer_occupancy_grid_node",
        name="cartographer_occupancy_grid_node",
        output="screen",
        parameters=[{'use_sim_time': use_sim_time}],
        arguments=[
            "-resolution", "0.03",
            "-publish_period_sec", "0.5"
        ]
    )

    return LaunchDescription([
        cartographer_node,
        occupancy_grid_node
    ])