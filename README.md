Go2 机器人 SLAM 建图与 Nav2 导航使用文档
适用平台：ROS2 Humble / Iron | Unitree Go2 四足机器人
运行环境：真机 / Gazebo 仿真
一、项目方案介绍
本项目为四足机器人 Go2 基于 ROS2 的 SLAM 建图与自主导航方案，内置两套成熟建图算法，适配真机与 Gazebo 仿真双环境，支持多传感器融合定位导航。
支持建图方案
slam_toolbox 建图 + Nav2 导航
Cartographer 建图 + Nav2 导航
支持传感配置
纯激光雷达建图
激光雷达 + 里程计 (Odom) + IMU 融合 EKF 建图（推荐，定位更稳定）
二、多传感器融合配置
配置文件路径：
plaintext
src/go2_slam/config/backpack_2d.lua
开启多传感器融合核心参数：
lua
use_odometry = true        -- 启用里程计数据
use_pose_extrapolator = true  -- 启用位姿外插补偿
use_imu_data = true        -- 启用IMU惯性数据
三、完整运行流程（真机环境）
步骤 1：启动机器人基础驱动与 SLAM 建图节点
bash
运行
ros2 launch go2_core go2_start.launch.py
步骤 2：键盘遥控机器人移动建图
bash
运行
ros2 run teleop_twist_keyboard teleop_twist_keyboard
✅ 操作建议：匀速慢速移动，减少原地快速旋转，降低地图拖影畸变。
步骤 3：保存建好的地图
bash
运行
mkdir -p ~/go2_maps
ros2 run nav2_map_server map_saver_cli -f ~/go2_maps/my_map
步骤 4：启动 Nav2 自主导航
先关闭建图节点，再执行：
bash
运行
ros2 launch go2_navigation2 go2_nav2.launch.py
四、完整运行流程（Gazebo 仿真环境）
步骤 1：启动仿真环境
bash
运行
ros2 launch go2_core go2_sim.launch.py
步骤 2：键盘遥控仿真机器人建图
bash
运行
ros2 run teleop_twist_keyboard teleop_twist_keyboard
步骤 3（可选）：查看 TF 坐标变换关系
bash
运行
ros2 run tf2_tools view_frames
步骤 4：仿真环境保存地图
bash
运行
ros2 run nav2_map_server map_saver_cli -f /home/phj/Desktop/go2nav_ws2/src/go2_navigation2/maps/new_map
步骤 5：启动仿真环境 Nav2 导航
bash
运行
ros2 launch go2_navigation2 go2_nav2_sim.launch.py
五、现存已知问题
问题 1：建图障碍物拖影
机器人移动建图过程中，存在角速度旋转运动时，激光匹配滞后，环境障碍物出现明显拖影、地图畸变。
问题 2：导航终点漂移踱步
机器人自主导航到达目标点后，定位不稳定，存在小范围位置漂移，表现为机器人原地来回跺脚、小幅抖动，无法精准定点静止。
