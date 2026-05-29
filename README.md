Go2 机器人 SLAM 建图与 Nav2 导航使用文档
一、项目方案介绍
本项目为四足机器人 Go2 基于 ROS2 的 SLAM 建图与自主导航方案，内置两套成熟建图算法，适配真机与 Gazebo 仿真双环境，支持多传感器融合定位导航。
支持两套建图方案：
- slam_toolbox 建图 + Nav2 导航
- Cartographer 建图 + Nav2 导航
支持传感配置：
- 纯激光雷达建图
- 激光雷达 + 里程计(Odom) + IMU 融合 EKF 建图（推荐，定位更稳定）
二、多传感器融合配置
配置文件路径：src/go2_slam/config/backpack_2d.lua
开启多传感器融合核心参数：
use_odometry = true        -- 启用里程计数据
use_pose_extrapolator = true  -- 启用位姿外插补偿
use_imu_data = true        -- 启用IMU惯性数据
三、完整运行流程（真机环境）
步骤1：启动机器人基础驱动与SLAM建图节点
终端输入指令，启动Go2机器人驱动、传感器、SLAM建图功能：
ros2 launch go2_core go2_start.launch.py
步骤2：键盘遥控机器人移动建图
新开终端，启动键盘控制节点，操控机器人缓慢移动，扫描完整环境地图：
ros2 run teleop_twist_keyboard teleop_twist_keyboard
操作建议：匀速慢速移动，减少原地快速旋转，降低地图拖影畸变。
步骤3：保存建好的地图
环境扫描完成、地图无明显畸变后，新建地图文件夹并保存地图文件：
mkdir -p ~/go2_maps
ros2 run nav2_map_server map_saver_cli -f ~/go2_maps/my_map
步骤4：启动Nav2自主导航
关闭建图节点，新开终端启动导航功能，加载保存的地图实现自主定点导航：
ros2 launch go2_navigation2 go2_nav2.launch.py
四、完整运行流程（Gazebo仿真环境）
步骤1：启动仿真环境
启动Gazebo仿真器、虚拟Go2机器人及全套虚拟传感器：
ros2 launch go2_core go2_sim.launch.py
步骤2：键盘遥控仿真机器人建图
ros2 run teleop_twist_keyboard teleop_twist_keyboard
步骤3（可选）：查看TF坐标变换关系
排查坐标系转换异常、定位错位问题：
ros2 run tf2_tools view_frames
步骤4：仿真环境保存地图
ros2 run nav2_map_server map_saver_cli -f /home/phj/Desktop/go2nav_ws2/src/go2_navigation2/maps/new_map
步骤5：启动仿真环境Nav2导航
ros2 launch go2_navigation2 go2_nav2_sim.launch.py
五、现存已知问题
问题1：建图障碍物拖影
机器人移动建图过程中，存在角速度旋转运动时，激光匹配滞后，环境障碍物出现明显拖影、地图畸变。
问题2：导航终点漂移踱步
机器人自主导航到达目标点后，定位不稳定，存在小范围位置漂移，表现为机器人原地来回跺脚、小幅抖动，无法精准定点静止。
