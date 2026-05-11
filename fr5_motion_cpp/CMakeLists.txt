cmake_minimum_required(VERSION 3.8)
project(fr5_motion_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(geometry_msgs REQUIRED)

add_executable(fr5_movegroup_node src/fr5_movegroup_node.cpp)
target_compile_features(fr5_movegroup_node PUBLIC cxx_std_17)
ament_target_dependencies(fr5_movegroup_node
  rclcpp
  moveit_ros_planning_interface
  geometry_msgs
)

install(TARGETS fr5_movegroup_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
