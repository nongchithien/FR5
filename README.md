# frcobot_ros2
This is the ROS2 API project of Fairino robot(sofeware version must greater than V3.7.1), a serial of functions which based on Fair SDK API but were simplified are created, user can call them through service message.
API_description.md list all API functions.
Tutorial of installing and uasage of ROS2 API, please refer to the Fair document platform:https://fair-documentation.readthedocs.io/en/latest/ROSGuide/index.html#frcobot-ros2.

Version histroy:
2023.7.18 V1.0

## Quá trình Tích hợp Gripper vào MoveIt2 và Hardware Interface

Dưới đây là tóm tắt các lỗi gặp phải trong quá trình cấu hình tay kẹp (gripper) chung với cánh tay trên cùng một hardware interface và cách khắc phục:

### 1. Lỗi trùng lặp tên Hardware (Hardware name Fr5Hardware is duplicated)
* **Lỗi:** `resource_manager` của ROS 2 Humble không cho phép 2 khối `<ros2_control>` trong URDF có trùng tên `name="Fr5Hardware"`.
* **Sửa:** Gộp các khai báo `<joint>` của tay kẹp (`finger_joint1` và `finger_joint2`) trực tiếp vào khối `ros2_control` của cánh tay trong file `fairino5_v6_robot.ros2_control.xacro`. Loại bỏ macro gọi `ros2_control` riêng biệt của tay kẹp.

### 2. Lỗi số lượng interface (Joint 'j1' has 2 state interface. 3 expected)
* **Lỗi:** Code phần cứng C++ `FairinoHardwareInterface` chỉ hỗ trợ và kiểm tra đúng 1 `state_interface` (`position`) cho cánh tay, nhưng file xacro cũ khai báo dư `<state_interface name="velocity"/>`.
* **Sửa:** Xóa toàn bộ thẻ `<state_interface name="velocity"/>` của các khớp `j1` đến `j6` trong `fairino5_v6_robot.ros2_control.xacro`.

### 3. Thiếu velocity cho Gripper (State interface with key 'finger_joint1/velocity' does not exist)
* **Lỗi:** Trái ngược với cánh tay, bộ điều khiển `GripperActionController` của ROS 2 bắt buộc tay kẹp phải cung cấp cả 2 state interface: `position` và `velocity`.
* **Sửa:** 
  * Bổ sung lại thẻ `<state_interface name="velocity"/>` cho `finger_joint1` trong file xacro.
  * Cập nhật hàm `on_init` trong C++ (`fairino_hardware_interface.cpp`) để bỏ qua kiểm tra "chỉ có 1 state interface" đối với tay kẹp.
  * Khai báo một biến dummy `_gripper_velocity_state` và export ra trong `export_state_interfaces()`.

### 4. Lỗi sai tên khớp trên MoveIt (Missing finger_joint1)
* **Lỗi:** Hệ thống khai báo tên khớp là `fr5_finger_joint1` nhưng trong file `fairino5_v6.urdf` gốc lại định nghĩa khớp tên là `finger_joint1`.
* **Sửa:** Đồng bộ tên bằng cách đổi lại toàn bộ `fr5_finger_joint1` thành `finger_joint1` trong các file config yaml (ros2_controllers, moveit_controllers), tệp xacro và code logic mapping C++.

### 5. Lỗi quỹ đạo bị hủy ngang (Controller handle reports status ABORTED)
* **Lỗi:** Lệnh kẹp chạy bình thường ở thực tế nhưng RViz luôn báo Failed (Aborted). Nguyên nhân là do `GripperActionController` có cơ chế phát hiện kẹt (Stall Detection) với giới hạn 1.0 giây. Việc code C++ dừng vị trí chờ tay kẹp 3 giây khiến controller tưởng tay kẹp đã bị kẹt và hủy lệnh.
* **Sửa:** 
  * Triển khai code điều khiển tay kẹp qua DO (status=1 kích chạy, tự động tắt status=0 sau 3 giây) hoàn toàn **Non-blocking** trong hàm `write()` để không làm giật lag hàm `ServoJ`.
  * Áp dụng thuật toán **Nội suy (Interpolation)** để làm biến `_gripper_position_state` tăng dần đều đặn trong suốt 3 giây, giúp vượt qua hệ thống chống kẹt của ROS 2 một cách hoàn hảo, trả về trạng thái Success cho RViz.
