# Go2 机器人 SLAM 建图与 Nav2 导航使用文档

## 一、项目方案介绍
本项目为四足机器人 Go2 基于 ROS2 的 SLAM 建图与自主导航方案，内置两套成熟建图算法，适配**真机 / Gazebo 仿真**双环境，支持多传感器融合稳定定位导航。

### 支持建图方案
- `slam_toolbox` 建图 + Nav2 导航
- `Cartographer` 建图 + Nav2 导航

### 支持传感配置
- 纯激光雷达建图
- **激光雷达 + 里程计(Odom) + IMU 融合 EKF 建图**（推荐，定位更稳定）

---

## 二、多传感器融合配置
**配置文件路径**
```plaintext
src/go2_slam/config/backpack_2d.lua
```
## 二、多传感器融合配置
**融合开启核心参数**
```lua
use_odometry = true           -- 启用里程计数据
use_pose_extrapolator = true  -- 启用位姿外插补偿
use_imu_data = true           -- 启用IMU惯性数据
```

## 三、完整运行流程（真机环境）
**步骤 1：启动机器人驱动与 SLAM 建图节点**
```bash
ros2 launch go2_core go2_start.launch.py
```
**步骤 2：键盘遥控建图**
```bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard
```
**步骤 3：保存地图**
```bash
mkdir -p ~/go2_maps
ros2 run nav2_map_server map_saver_cli -f ~/go2_maps/my_map
```
**步骤 4：启动 Nav2 自主导航**
- 关闭建图节点后执行：
```bash
ros2 launch go2_navigation2 go2_nav2.launch.py
```



