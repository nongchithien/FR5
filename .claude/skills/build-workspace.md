# Skill: Build Workspace

> Build và test ROS 2 workspace cho Fairino FR5.

## Quick build (toàn bộ)

```bash
cd /home/thien123/frcobot_ros2-main
colcon build --symlink-install
source install/setup.bash
```

## Build từng package

```bash
# Messages (build trước tiên nếu thay đổi .msg/.srv)
colcon build --packages-select fairino_msgs

# Hardware interface
colcon build --packages-select fairino_hardware

# MoveIt config
colcon build --packages-select fairino5_v6_moveit2_config

# Tất cả với debug
colcon build --cmake-args -DCMAKE_BUILD_TYPE=Debug
```

## Xóa build cache (khi gặp lỗi lạ)

```bash
rm -rf build/ install/ log/
colcon build --symlink-install
```

## Test nhanh

```bash
# Kiểm tra package có build được không
colcon build --packages-select fairino_hardware 2>&1 | tail -5

# Chạy simulation
ros2 launch fairino5_v6_moveit2_config demo.launch.py

# Liệt kê topics/services đang active
ros2 topic list
ros2 service list
```

## Lỗi thường gặp

| Lỗi | Nguyên nhân | Giải pháp |
|-----|-------------|-----------|
| `undefined reference` | Thiếu link library | Kiểm tra `target_link_libraries` trong CMakeLists |
| `No such file or directory` (header) | Thiếu include path | Kiểm tra `target_include_directories` |
| `Could not find package` | Thiếu dependency | Thêm vào `package.xml` + `find_package()` |
| `libfairino.so: cannot open` | SDK lib chưa install | Rebuild, kiểm tra symlink chain |
