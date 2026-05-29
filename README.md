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

