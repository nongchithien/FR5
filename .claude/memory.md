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
