from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
from launch.substitutions import LaunchConfiguration
import os

def generate_launch_description():
    get_nav2_pkg = get_package_share_directory("go2_navigation2")
    get_bringup_pkg = get_package_share_directory("nav2_bringup")
    unitree_go2_sim_pkg = get_package_share_directory("unitree_go2_sim")
    go2_perception_pkg = get_package_share_directory("go2_perception")

    use_sim_time = LaunchConfiguration('use_sim_time', default='true')
    
    default_map = '/home/phj/Desktop/go2nav_ws2/src/go2_navigation2/maps/new_map.yaml'
    
    declare_map = DeclareLaunchArgument(
        'map',
        default_value=default_map,
        description='地图 yaml 路径',
    )
    map_yaml_path = LaunchConfiguration('map')
    
    nav2_param_path = LaunchConfiguration(
        'params_file', 
        default=os.path.join(get_nav2_pkg, 'config', 'nav2_params.yaml')
    )
    rviz_config_dir = os.path.join(get_bringup_pkg, 'rviz', 'nav2_default_view.rviz')

    unitree_go2_sim_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(unitree_go2_sim_pkg, "launch", "unitree_go2_launch.py")
        ),
        launch_arguments=[("use_sim_time", use_sim_time),
                          ("rviz", "false")]
    )

    go2_perception_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(go2_perception_pkg, "launch", "go2_pointcloud_sim.launch.py")
        ),
        launch_arguments=[("use_sim_time", use_sim_time)]
    )

    map_server = Node(
        package='nav2_map_server',
        executable='map_server',
        name='map_server',
        output='screen',
        parameters=[{
            'yaml_filename': map_yaml_path,
            'use_sim_time': use_sim_time,
        }]
    )

    amcl = Node(
        package='nav2_amcl',
        executable='amcl',
        name='amcl',
        output='screen',
        parameters=[nav2_param_path, {'use_sim_time': use_sim_time}]
    )

    # 5. 生命周期管理器（只管理 map_server + amcl）
    lifecycle_manager_map_amcl = Node(
        package='nav2_lifecycle_manager',
        executable='lifecycle_manager',
        name='lifecycle_manager_map_amcl',
        output='screen',
        parameters=[{
            'use_sim_time': use_sim_time,
            'autostart': True,
            'node_names': ['map_server', 'amcl']
        }]
    )

    controller_server = Node(
        package='nav2_controller',
        executable='controller_server',
        name='controller_server',
        output='screen',
        parameters=[nav2_param_path, {'use_sim_time': use_sim_time}],
        remappings=[('cmd_vel', 'cmd_vel_nav')]
    )

    smoother_server = Node(
        package='nav2_smoother',
        executable='smoother_server',
        name='smoother_server',
        output='screen',
        parameters=[nav2_param_path, {'use_sim_time': use_sim_time}]
    )

    planner_server = Node(
        package='nav2_planner',
        executable='planner_server',
        name='planner_server',
        output='screen',
        parameters=[nav2_param_path, {'use_sim_time': use_sim_time}]
    )

    behavior_server = Node(
        package='nav2_behaviors',
        executable='behavior_server',
        name='behavior_server',
        output='screen',
        parameters=[nav2_param_path, {'use_sim_time': use_sim_time}]
    )

    bt_navigator = Node(
        package='nav2_bt_navigator',
        executable='bt_navigator',
        name='bt_navigator',
        output='screen',
        parameters=[nav2_param_path, {'use_sim_time': use_sim_time}]
    )

    waypoint_follower = Node(
        package='nav2_waypoint_follower',
        executable='waypoint_follower',
        name='waypoint_follower',
        output='screen',
        parameters=[nav2_param_path, {'use_sim_time': use_sim_time}]
    )

    velocity_smoother = Node(
        package='nav2_velocity_smoother',
        executable='velocity_smoother',
        name='velocity_smoother',
        output='screen',
        parameters=[nav2_param_path, {'use_sim_time': use_sim_time}],
        remappings=[('cmd_vel', 'cmd_vel_nav'), ('cmd_vel_smoothed', 'cmd_vel')]
    )

    lifecycle_manager_navigation = Node(
        package='nav2_lifecycle_manager',
        executable='lifecycle_manager',
        name='lifecycle_manager_navigation',
        output='screen',
        parameters=[{
            'use_sim_time': use_sim_time,
            'autostart': True,
            'node_names': [
                'controller_server',
                'smoother_server',
                'planner_server',
                'behavior_server',
                'bt_navigator',
                'waypoint_follower',
                'velocity_smoother'
            ]
        }]
    )

    rviz2 = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config_dir],
        parameters=[{'use_sim_time': use_sim_time}],
        output='screen'
    )

    return LaunchDescription([
        declare_map,
        unitree_go2_sim_launch,
        go2_perception_launch,
        map_server,
        amcl,
        lifecycle_manager_map_amcl,
        controller_server,
        smoother_server,
        planner_server,
        behavior_server,
        bt_navigator,
        waypoint_follower,
        velocity_smoother,
        lifecycle_manager_navigation,
        rviz2,
    ])