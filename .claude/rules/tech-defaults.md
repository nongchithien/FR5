# Tech Defaults — Công nghệ và cấu hình mặc định

## Stack công nghệ

| Component | Technology | Version |
|-----------|-----------|---------|
| OS | Ubuntu | 22.04 LTS |
| ROS | ROS 2 | Humble Hawksbill |
| Motion Planning | MoveIt 2 | latest |
| Controller | ros2_control | latest |
| Build | colcon + ament_cmake | latest |
| C++ | GNU C++ | C++17 |
| Python | Python 3 | 3.10+ |
| Robot SDK | libfairino | v2.2.6 |

## Dependencies mặc định (package.xml / CMakeLists.txt)

### fairino_hardware
```
rclcpp
std_msgs
hardware_interface
pluginlib
rclcpp_lifecycle
fairino_msgs
```

### fairino5_v6_moveit2_config
```
moveit_ros_planning_interface
moveit_ros_move_group
moveit_kinematics
moveit_planners_ompl
moveit_servo
joint_state_publisher
robot_state_publisher
controller_manager
joint_trajectory_controller
joint_state_broadcaster
```

## Cấu hình mặc định

### ros2_controllers.yaml
- Controller: `JointTrajectoryController`
- Update rate: `100 Hz`
- Command interface: `position`
- State interface: `position`

### Hardware Interface
- IP: `192.168.57.2` (macro `CONTROLLER_IP_ADDRESS`)
- ServoJ cycle: `0.008s`
- Control mode: `0` (position control)
- Joints: 6 (j1–j6)
- External axis: 4 (mặc định {0,0,0,0})

### SDK link chain
```
libfairino.so → libfairino.so.2 → libfairino.so.2.2.6
```
- Symlinks được tạo tự động trong `CMakeLists.txt`
- Copy vào `install/lib/` khi build

### Command Server
- Service name: `fairino_remote_command_service`
- Service type: `fairino_msgs/srv/RemoteCmdInterface`
- Language version: controlled by `CHN_VERSION` / `ENG_VERSION` macros
- Max reconnect retries: defined in code
- State feedback timer: `10ms` interval

## File types trong project

| Extension | Mục đích |
|-----------|----------|
| `.cpp` / `.hpp` | C++ source / header |
| `.py` | Python launch files, test scripts |
| `.xacro` | URDF/ros2_control XML macros |
| `.yaml` | ROS 2 parameter / config |
| `.xml` | pluginlib export, package manifest |
| `.srdf` | MoveIt semantic robot description |
| `.stl` / `.dae` | 3D mesh files |
| `.srv` / `.msg` | ROS 2 service/message definitions |
