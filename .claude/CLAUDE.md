# CLAUDE.md — Fairino FR5 ROS 2 Project Brain

> Bộ não chính của dự án. File này được commit lên Git cho cả team sử dụng.

## Dự án là gì?

ROS 2 driver + MoveIt 2 integration cho **robot công nghiệp Fairino FR5** (6 trục).
Cho phép điều khiển robot thật hoặc mô phỏng qua ros2_control và MoveIt 2.

- **Repo**: https://github.com/nongchithien/FR5.git
- **Robot**: Fairino FR5, 6-axis, firmware >= V3.7.1
- **SDK**: libfairino v2.2.6 (prebuilt C++ shared library)
- **ROS 2**: Humble / Iron trên Ubuntu Linux

## Cấu trúc workspace

```
frcobot_ros2-main/
├── fairino_hardware/            # ros2_control plugin + command server
│   ├── src/
│   │   ├── fairino_hardware_interface.cpp  # ServoJ realtime control loop
│   │   ├── command_server.cpp              # String command API service (3000+ LOC)
│   │   └── command_server_node.cpp
│   ├── include/fairino_hardware/
│   ├── libfairino/              # Fairino C++ SDK (.so binaries)
│   └── CMakeLists.txt
├── fairino5_v6_moveit2_config/  # MoveIt 2 Setup Assistant config
│   ├── config/                  # SRDF, controllers, kinematics, ros2_control xacro
│   └── launch/                  # Launch files + Python test scripts
├── fairino_description/         # URDF, meshes (STL/DAE)
├── fairino_msgs/                # Custom msg/srv (RemoteCmdInterface.srv)
├── mtc_test/                    # MoveIt Task Constructor tests
├── API_Instruction.pdf          # SDK API documentation
└── ErrorCode_lnstruction.pdf    # Error code reference
```

## Quy ước bắt buộc

| Quy ước | Giá trị |
|---------|---------|
| Đơn vị góc MoveIt/ROS | **radian** |
| Đơn vị góc Fairino SDK | **degree** |
| Đơn vị khoảng cách ROS | **meter** |
| Đơn vị khoảng cách SDK | **mm** |
| Controller IP | `192.168.57.2` |
| ServoJ cycle | `0.008s` (8ms) |
| Controller update rate | `100 Hz` |
| Build command | `colcon build --symlink-install` |
| C++ standard | C++17 |

## Quy tắc chi tiết

Xem thêm trong thư mục `rules/`:
- `rules/workflow.md` — Quy trình build, test, deploy
- `rules/design.md` — Kiến trúc và design patterns
- `rules/tech-defaults.md` — Công nghệ và cấu hình mặc định

## Sub-agents

- `agents/researcher.md` — Tra cứu SDK docs, ROS 2 patterns
- `agents/reviewer.md` — Review code an toàn cho robot thật
- `agents/fixer.md` — Sửa lỗi vặt: include, path, syntax, build errors

## Skills

- `skills/build-workspace.md` — Build & test ROS 2 workspace
- `skills/debug-connection.md` — Debug kết nối robot
- `skills/switch-sim-real.md` — Chuyển đổi simulation ↔ real robot
- `skills/send-cartesian-cmd.md` — Gửi lệnh Cartesian qua service
- `skills/add-sdk-function.md` — Thêm API function mới vào command server
