cmake_minimum_required(VERSION 3.8)
project(go2_perception)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)

find_package(laser_geometry REQUIRED)
find_package(message_filters REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_sensor_msgs REQUIRED)

include_directories(
  include
)

# ====================== 组件模式（正确写法，你的代码本来就该这样） ======================
add_library(pointcloud_to_laserscan SHARED
  src/pointcloud_to_laserscan_node.cpp
)
target_compile_definitions(pointcloud_to_laserscan
  PRIVATE "POINTCLOUD_TO_LASERSCAN_BUILDING_DLL"
)
ament_target_dependencies(pointcloud_to_laserscan
  laser_geometry
  message_filters
  rclcpp
  rclcpp_components
  sensor_msgs
  tf2
  tf2_ros
  tf2_sensor_msgs
)

# 这一行会自动生成可执行文件！！！不需要你自己 add_executable！
rclcpp_components_register_node(pointcloud_to_laserscan
  PLUGIN "pointcloud_to_laserscan::PointCloudToLaserScanNode"
  EXECUTABLE pointcloud_to_laserscan_node
)

install(TARGETS
  pointcloud_to_laserscan
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib
)

# ====================== 另一个节点（正常） ======================
add_executable(cloud_accumulation
  src/cloud_accumulation.cpp
)
ament_target_dependencies(cloud_accumulation
  laser_geometry
  message_filters
  rclcpp
  rclcpp_components
  sensor_msgs
  tf2
  tf2_ros
  tf2_sensor_msgs
)

install(TARGETS
  cloud_accumulation
  DESTINATION lib/${PROJECT_NAME}
)

# 安装 launch
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
