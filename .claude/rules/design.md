# Design Rules — Kiến trúc và mẫu thiết kế

## Kiến trúc tổng thể

```
┌─────────────────────────────────────────────────────┐
│                    MoveIt 2                          │
│         (motion planning, collision check)            │
└──────────────────────┬──────────────────────────────┘
                       │ JointTrajectoryController
                       │ (100 Hz)
┌──────────────────────▼──────────────────────────────┐
│            ros2_control framework                    │
│  ┌─────────────────────────────────────────────┐    │
│  │  FairinoHardwareInterface (SystemInterface)  │    │
│  │   read()  ← GetActualJointPosDegree()        │    │
│  │   write() → ServoJ() (8ms cycle)             │    │
│  └─────────────────────────────────────────────┘    │
└──────────────────────┬──────────────────────────────┘
                       │ libfairino SDK
┌──────────────────────▼──────────────────────────────┐
│           Fairino Robot Controller                   │
│              (192.168.57.2)                          │
└─────────────────────────────────────────────────────┘

Song song:
┌─────────────────────────────────────────────────────┐
│  command_server (ros2_cmd_server)                     │
│  - ROS 2 Service: fairino_remote_command_service     │
│  - String command API: "MoveJ(...)", "MoveL(...)"    │
│  - Dùng cho lệnh one-shot, không qua MoveIt         │
└─────────────────────────────────────────────────────┘
```

## Hardware Interface — Quy tắc khi sửa

### Hàm `read()` và `write()`
- Chạy ở **100Hz** — code PHẢI nhẹ, KHÔNG blocking
- Không `RCLCPP_INFO` trong vòng lặp chính (tốn thời gian I/O) — chỉ log khi có lỗi
- Luôn check `returncode` từ SDK calls
- Convert đơn vị đúng: `degree ↔ radian`

### ServoJ call
```cpp
_ptr_robot->ServoJ(&cmd, &extcmd, 0, 0, 0.008, 0, 0);
//                  ↑      ↑              ↑
//              6-axis   ext axis     cycle time
```
- `ExaxisPos extcmd` **phải luôn được truyền**, kể cả khi = {0,0,0,0}
- **KHÔNG BAO GIỜ** gửi ServoJ cho robot và external axis riêng biệt

### Thêm state/command interface mới
1. Khai báo biến trong `.hpp` (vd: `double _new_var[6]`)
2. Export trong `export_state_interfaces()` hoặc `export_command_interfaces()`
3. Validate trong `on_init()` (check interface count/name)
4. Cập nhật `on_activate()` (khởi tạo giá trị)

## Command Server — Quy tắc khi sửa

### Thêm function mới
1. Khai báo method trong `.hpp`
2. Implement trong `.cpp` — sử dụng `_splitString2List()` để parse params
3. Register vào `_fr_function_list` map
4. Bọc SDK call trong `std::to_string()` để trả về result

### String format bắt buộc
```
"FunctionName(param1,param2,...)"
```
- Không space
- Params phân cách bằng `,`
- Regex validation: `([A-Za-z_]+)[(](.*)[)]`

## MoveIt Config — Quy tắc khi sửa

- **Không sửa SRDF thủ công** — dùng MoveIt Setup Assistant
- Joint names phải khớp: `j1, j2, j3, j4, j5, j6`
- Controller name: `fairino5_controller`
- Kinematics solver: mặc định KDL

## Code style

- **C++17** standard
- Comments bằng **tiếng Anh** (ưu tiên)
- Giữ code gọn trong `read()/write()` — tối ưu performance
- Log messages: dùng `RCLCPP_INFO/WARN/ERROR/FATAL`
- Compile flags: `-Wall -Wextra -Wpedantic`
