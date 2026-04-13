# Memory — Bộ nhớ từ lịch sử làm việc

> Ghi nhận các bài học, vấn đề đã giải quyết, và kiến thức tích lũy qua các phiên làm việc.

---

## 🔧 Vấn đề đã giải quyết

### 1. Sửa lỗi compilation hardware interface (2026-03-30)
- **Vấn đề**: Missing includes, undeclared identifiers cho ROS 2 hardware interface
- **Nguyên nhân**: Thiếu `#include` cho `system_interface.hpp`, `hardware_interface_return_values.hpp`, `rclcpp_lifecycle`
- **Giải pháp**: Thêm đúng headers + cập nhật dependencies trong `CMakeLists.txt` và `package.xml`
- **Bài học**: Luôn kiểm tra `ament_target_dependencies()` khi thêm header mới

### 2. Socket create failed khi kết nối robot (2026-03-30)
- **Vấn đề**: `socket create failed` khi chạy `fairino_hardware` node
- **Nguyên nhân**: Không ping được robot controller `192.168.57.2`
- **Debug steps**:
  1. `ping 192.168.57.2`
  2. `telnet 192.168.57.2 20004`
  3. Kiểm tra `ip addr show` — workstation phải cùng subnet 192.168.57.x
  4. Tắt firewall nếu cần
- **Bài học**: Luôn kiểm tra network trước khi debug code

### 3. Test robot points trong RViz (2026-03-31 → 2026-04-01)
- **Mục tiêu**: Hiển thị các điểm Cartesian target trong RViz
- **Giải pháp**: Tạo `test_rviz.py` — node publish `MarkerArray` lên topic `visualization_marker_array`
- **Lưu ý**:
  - Chuyển đổi mm → m khi gửi vào RViz
  - `frame_id = "base_link"`
  - Marker type SPHERE, scale 0.05m

### 4. Gửi lệnh Cartesian qua Python service (2026-03-31 → 2026-04-01)
- **Mục tiêu**: Tạo Python node gửi lệnh MoveL đến robot thông qua command service
- **Giải pháp**: Tạo `move_cartesian.py` sử dụng `RemoteCmdInterface` service
- **Quy trình**:
  1. `CARTPoint(idx,x,y,z,rx,ry,rz)` — lưu điểm vào ô nhớ
  2. `MoveL(CART[idx],speed,tool,user)` — di chuyển đến điểm
- **⚠️ QUAN TRỌNG**: Tham số KHÔNG được có dấu cách (space) — SDK sẽ reject

### 5. Push code lên GitHub (2026-04-03)
- **Repo**: https://github.com/nongchithien/FR5.git
- **Đã force push** để xóa remote cũ và thay thế hoàn toàn
- **Lưu ý**: Cẩn thận với `--force`, chỉ dùng khi thật sự cần

### 6. Dịch comment Chinese → English (2026-04-05)
- **Vấn đề**: Codebase gốc Fairino SDK có rất nhiều comment tiếng Trung
- **Giải pháp**: Script Python scan file `.cpp`/`.py`, detect Chinese chars, translate ra English
- **Trạng thái**: Đang chuyển đổi dần, chưa hoàn thành 100%
- **Bài học**: Codebase gốc từ Fairino nên giữ bản gốc backup

### 7. Đồng bộ robot 6 trục + external axis (2026-04-06)
- **Vấn đề**: Joint commands cho 6 trục robot và external axis phải gửi đồng thời
- **Giải pháp**: Tích hợp `ExaxisPos` vào struct truyền kèm lệnh `ServoJ` — 1 lệnh duy nhất
- **TUYỆT ĐỐI KHÔNG** gửi riêng 2 lệnh (robot + external axis) — sẽ gây xung đột motion

### 8. Tạo cấu trúc .claude/ (2026-04-13)
- **Mục tiêu**: Tạo bộ nhớ + hướng dẫn cho AI assistant theo chuẩn `.claude/` directory
- **Kết quả**: 15 files được tạo trong `.claude/`:
  - `CLAUDE.md` — Project brain chính
  - `CLAUDE.local.md` — Config private local
  - `settings.json` / `settings.local.json` — Permissions & hooks
  - `memory.md` — File này
  - `rules/` — workflow.md, design.md, tech-defaults.md
  - `agents/` — researcher.md, reviewer.md, fixer.md
  - `skills/` — build-workspace.md, debug-connection.md, switch-sim-real.md, send-cartesian-cmd.md, add-sdk-function.md

### 9. Fix VSCode IntelliSense errors (2026-04-13)
- **Vấn đề**: IDE báo lỗi `'mtc_test/mtc_node.hpp' file not found` và cascade 17 errors
- **Nguyên nhân**: `settings.json` chỉ trỏ `compileCommands` đến `fairino_hardware`, không cover `mtc_test`
- **Giải pháp**:
  1. Tạo merged `build/compile_commands.json` từ tất cả packages
  2. Cập nhật `c_cpp_properties.json` — thêm đầy đủ includePaths + browse.path
  3. `settings.json` trỏ đến merged compile_commands
- **Bài học**: Colcon tạo `compile_commands.json` riêng per-package, cần merge để IDE hoạt động

### 10. Fix mtc_test build errors (2026-04-13)
- **Vấn đề 1**: `moveit_task_constructor_core` not found
  - **Fix**: `sudo apt install ros-humble-moveit ros-humble-moveit-task-constructor-core`
- **Vấn đề 2**: `planning_scene.hpp: No such file` (sau khi cài MoveIt)
  - **Nguyên nhân**: ROS 2 Humble MoveIt dùng `.h` extension, code viết cho version mới dùng `.hpp`
  - **Fix**: Thêm `__has_include()` fallback trong `mtc_node.hpp`:
    ```cpp
    #if __has_include(<moveit/planning_scene/planning_scene.hpp>)
    #include <moveit/planning_scene/planning_scene.hpp>
    #else
    #include <moveit/planning_scene/planning_scene.h>
    #endif
    ```
- **Vấn đề 3**: CMakeLists.txt thiếu dependencies
  - **Fix**: Thêm `find_package` + `ament_target_dependencies` cho: `geometry_msgs`, `moveit_core`, `moveit_msgs`, `moveit_ros_planning_interface`, `shape_msgs`, `tf2_eigen`, `tf2_geometry_msgs`
- **Bài học**: ROS 2 Humble MoveIt headers dùng `.h`, không phải `.hpp`. Dùng `__has_include()` để tương thích nhiều version.

### 11. Phát triển MTC pick & place task (2026-04-13)
- **Mục tiêu**: Xây dựng MoveIt Task Constructor pipeline cho pick & place
- **Trạng thái**: Đang phát triển — đã thêm `stage_move_to_pick` (Connect stage) và `attach_object_stage`
- **Lưu ý**: Code demo hiện vẫn dùng `panda_arm` — cần đổi sang `fairino5` group names

### 12. Thêm package fr5_arm_hand_config (2026-04-13)
- **Mục tiêu**: MoveIt 2 config riêng cho FR5 + gripper/hand
- **Trạng thái**: Mới tạo, đang phát triển

---

## 📚 Kiến thức tích lũy

### Fairino SDK API patterns
```cpp
// Các API hay dùng nhất:
FRRobot::RPC(ip)                           // Kết nối
FRRobot::ServoJ(&cmd, &extcmd, ...)        // Servo realtime (trong write loop)
FRRobot::GetActualJointPosDegree(flag, &pos)// Đọc vị trí (trong read loop)
FRRobot::StopMotion()                       // Emergency stop
FRRobot::CloseRPC()                         // Ngắt kết nối
FRRobot::MoveJ/MoveL/MoveC(...)            // Motion commands
FRRobot::DragTeachSwitch(mode)              // Dạy kéo tay
FRRobot::RobotEnable(flag)                  // Enable/disable
FRRobot::GetRobotRealTimeState(&pkg)        // Trạng thái realtime
```

### Conversion formulas
```
degree → radian:  value / 180.0 * M_PI
radian → degree:  value / M_PI * 180.0
mm → meter:       value / 1000.0
meter → mm:       value * 1000.0
```

### Command Server string format
```
"FunctionName(param1,param2,...)"
```
- Không có space
- Params phân cách bằng dấu phẩy
- Number format: plain decimal (vd: `-178.118`)

### ros2_control plugin switching
- Simulation: `mock_components/GenericSystem`
- Real robot: `fairino_hardware/FairinoHardwareInterface`
- File: `fairino5_v6_moveit2_config/config/fairino5_v6_robot.ros2_control.xacro`

### ROS 2 Humble MoveIt header compatibility
- Humble MoveIt dùng `.h` (vd: `planning_scene.h`)
- MoveIt newer versions dùng `.hpp`
- Dùng `__has_include()` để viết code tương thích cả 2:
```cpp
#if __has_include(<moveit/xxx/xxx.hpp>)
#include <moveit/xxx/xxx.hpp>
#else
#include <moveit/xxx/xxx.h>
#endif
```

### VSCode + colcon IntelliSense
- Colcon tạo `compile_commands.json` riêng trong mỗi `build/<pkg>/`
- Cần merge vào `build/compile_commands.json` để IDE cover toàn bộ workspace
- Script merge:
```python
import json, glob
all_commands = []
for f in glob.glob('build/*/compile_commands.json'):
    with open(f) as fh:
        all_commands.extend(json.load(fh))
with open('build/compile_commands.json', 'w') as fh:
    json.dump(all_commands, fh, indent=2)
```

