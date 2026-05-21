from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():

    slam_config = os.path.join(
        get_package_share_directory("go2_slam"),
        "config",
        "mapper_params_online_async.yaml"
    )

    # ✅✅✅ 终极完美 SLAM 配置（兼容 CHAMP 四足机器人）
    slam_node = Node(
        package="slam_toolbox",
        executable="async_slam_toolbox_node",
        name="slam_toolbox",
        output="screen",
        # 🔥🔥🔥 关键：强制订阅 odom 并兼容 QoS
        remappings=[
            ("scan", "/scan"),
            ("odom", "/odom"),
        ],
        parameters=[
            slam_config,
            {"use_sim_time": True},
            # 👇 👇 👇 这是救你命的参数！！！
            {"odom_qos": "SENSOR_DATA"},  # 适配 CHAMP / Go2 的 odom
        ]
    )

    return LaunchDescription([slam_node])
