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
#### 关闭建图节点后执行：
```bash
ros2 launch go2_navigation2 go2_nav2.launch.py
```
## 四、完整运行流程（Gazebo 仿真环境）
**步骤 1：启动仿真环境**
```bash
ros2 launch go2_core go2_sim.launch.py
```

**步骤 2：键盘遥控仿真建图**
```bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard
```

**步骤 3（可选）：查看 TF 坐标变换**
```bash
ros2 run tf2_tools view_frames
```

**步骤 4：仿真环境保存地图**
```bash
ros2 run nav2_map_server map_saver_cli -f /home/phj/Desktop/go2nav_ws2/src/go2_navigation2/maps/new_map
```

**步骤 5：启动仿真 Nav2 导航**
```bash
ros2 launch go2_navigation2 go2_nav2_sim.launch.py
```
## 五、现存已知问题
###  问题 1：建图障碍物拖影
机器人原地旋转、存在角速度运动时，激光匹配存在滞后，导致环境障碍物拖影、地图畸变。
### 问题 2：导航终点漂移踱步
机器人到达导航终点后定位抖动不稳定，出现小幅漂移、原地跺脚抖动，无法完全静止定点。

