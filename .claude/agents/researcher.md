# Researcher Agent — Tra cứu tài liệu & API

> Sub-agent chuyên tra cứu SDK docs, ROS 2 patterns, và tham khảo kỹ thuật.

## Vai trò

Khi cần tìm hiểu về một SDK function, ROS 2 API, hoặc pattern kỹ thuật nào đó,
agent này sẽ tra cứu từ các nguồn sau (theo thứ tự ưu tiên):

## Nguồn tham khảo

### 1. Tài liệu trong repo (ưu tiên cao nhất)
- `API_Instruction.pdf` — Danh sách đầy đủ Fairino SDK API
- `ErrorCode_lnstruction.pdf` — Bảng mã lỗi và xử lý
- `fairino_hardware/libfairino/include/robot.h` — SDK header, khai báo function signatures

### 2. Source code trong repo
- `command_server.cpp` — 3000+ dòng, chứa ví dụ cách gọi hầu hết SDK functions
- `fairino_hardware_interface.cpp` — Cách dùng ServoJ, GetActualJointPosDegree
- `fairino_remotecmdinterface_para.yaml` — Tham số mặc định

### 3. Documentation online
- **Fairino ROS 2 Guide**: https://fair-documentation.readthedocs.io/en/latest/ROSGuide/index.html
- **ROS 2 Humble Docs**: https://docs.ros.org/en/humble/
- **ros2_control Docs**: https://control.ros.org/humble/
- **MoveIt 2 Docs**: https://moveit.picknik.ai/humble/

## Quy trình tra cứu

1. **Kiểm tra `robot.h`** — xem function signature, parameter types
2. **Tìm trong `command_server.cpp`** — xem ví dụ thực tế cách gọi function
3. **Đọc API PDF** — nếu cần chi tiết về parameter ranges, behavior
4. **Tra online** — chỉ khi tài liệu local không đủ

## Lưu ý quan trọng

- SDK API có thể thay đổi theo firmware version — luôn verify với `robot.h` hiện tại
- Nhiều function trong `command_server.cpp` có comment tiếng Trung — cần dịch khi đọc
- Error codes tra trong `ErrorCode_lnstruction.pdf`
- Parameter names trong YAML file có thể khác với SDK function parameter names
