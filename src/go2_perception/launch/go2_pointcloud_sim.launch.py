from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='go2_perception', executable='cloud_accumulation',
            remappings=[
                ('/utlidar/cloud_deskewed', '/unitree_lidar/points'), 
                ('/utlidar/cloud_accumulated', '/trans_cloud')
            ],
            name='cloud_accumulation_node'
        ),

        Node(
            package='go2_perception', executable='pointcloud_to_laserscan_node',
            remappings=[
                ('cloud_in', '/unitree_lidar/points'),  
                ('scan', '/scan')
            ],
            parameters=[{
                'target_frame': 'base_link',        
                'transform_tolerance': 0.2,
                'min_height': -0.1,                # 只过滤地面
                'max_height': 2.0,                 # 扩大有效高度 → 范围立刻变大
                'angle_min': -3.14159,
                'angle_max':  3.14159,
                'angle_increment': 0.0087,
                'scan_time': 0.1,
                'range_min': 0.3,                  # 过滤太近的乱点
                'range_max': 5.0,                 # 距离拉远
                'use_inf': True,
                'inf_epsilon': 1.0
            }],
            name='pointcloud_to_laserscan_node'
        )
    ])