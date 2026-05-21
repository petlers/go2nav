from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():

    go2_core_pkg = get_package_share_directory("go2_core")
    go2_slam_pkg = get_package_share_directory("go2_slam")
    go2_perception_pkg = get_package_share_directory("go2_perception")
    unitree_go2_sim_pkg = get_package_share_directory("unitree_go2_sim")

    use_sim_time = LaunchConfiguration('use_sim_time')
    declare_use_sim_time = DeclareLaunchArgument(
        'use_sim_time', default_value='true'
    )

    # # 里程计融合imu
    # go2_robot_localization = IncludeLaunchDescription(
    #         PythonLaunchDescriptionSource(
    #             os.path.join(go2_core_pkg, "launch", "go2_robot_localization.launch.py")
    #         )
    # )


    unitree_go2_sim_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(unitree_go2_sim_pkg, "launch", "unitree_go2_launch.py")
        ),
        launch_arguments=[
            ("use_sim_time", use_sim_time)
        ]
    )

    # 2. 启动雷达转激光
    go2_perception_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(go2_perception_pkg, "launch", "go2_pointcloud_sim.launch.py")
        ),
        launch_arguments=[
            ("use_sim_time", use_sim_time)
        ]
    )

    # 3. 启动 Cartographer
    go2_cartographer_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(go2_slam_pkg,"launch", "go2_cartographer.launch.py")
        ),
        launch_arguments=[
            ("use_sim_time", use_sim_time)
        ]
    )

    ld = LaunchDescription()
    ld.add_action(declare_use_sim_time)
    ld.add_action(unitree_go2_sim_launch)
    ld.add_action(go2_perception_launch)
    ld.add_action(go2_cartographer_launch)
    # ld.add_action(go2_robot_localization)
    return ld
