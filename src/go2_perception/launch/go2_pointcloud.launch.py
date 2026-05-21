from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='go2_perception', executable='cloud_accumulation',
            remappings=[
                ('/utlidar/cloud_accumulated', '/trans_cloud')
            ],
            name='cloud_accumulation_node'
        ),

        Node(
            package='go2_perception', executable='pointcloud_to_laserscan_node',
            remappings=[
                ('cloud_in', '/trans_cloud'),
                ('scan', '/scan')
            ],
            parameters=[{
                'target_frame': 'base_link',          # 正确
                'transform_tolerance': 0.2,
                'min_height': -0.3,                  # ✅ 必须改大！
                'max_height': 1.5,                   # ✅ 必须改大！
                'angle_min': -3.14159,
                'angle_max':  3.14159,
                'angle_increment': 0.0087,
                'scan_time': 0.1,
                'range_min': 0.3,                    # ✅ 过滤车身噪点
                'range_max': 10.0,
                'use_inf': False,                    # ✅ 正确
                'inf_epsilon': 0.1
            }],
            name='pointcloud_to_laserscan_node'
        )
    ])
