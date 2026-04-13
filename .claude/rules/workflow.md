# Workflow Rules — Quy trình làm việc

## Build

```bash
# Build toàn bộ workspace
cd /home/thien123/frcobot_ros2-main && colcon build --symlink-install

# Build 1 package
colcon build --packages-select fairino_hardware

# Build với debug symbols
colcon build --cmake-args -DCMAKE_BUILD_TYPE=Debug

# Source sau khi build
source install/setup.bash
```

## Thứ tự build đúng (nếu build lần đầu)

1. `fairino_msgs` (messages + services trước)
2. `fairino_description` (URDF + meshes)
3. `fairino_hardware` (hardware plugin + command server)
4. `fairino5_v6_moveit2_config` (MoveIt config)
5. `mtc_test` (task constructor)

```bash
colcon build --packages-select fairino_msgs
colcon build --packages-select fairino_description
colcon build --packages-select fairino_hardware
colcon build --packages-select fairino5_v6_moveit2_config
colcon build --packages-select mtc_test
```

## Test trên simulation

```bash
# Chạy MoveIt demo (simulation — GenericSystem)
ros2 launch fairino5_v6_moveit2_config demo.launch.py
```

## Test trên robot thật

1. Đảm bảo đã chuyển xacro sang real robot mode (xem `skills/switch-sim-real.md`)
2. Rebuild: `colcon build --packages-select fairino5_v6_moveit2_config`
3. Kiểm tra kết nối: `ping 192.168.57.2`
4. Launch:
   ```bash
   ros2 launch fairino5_v6_moveit2_config demo.launch.py
   ```

## Chạy command server riêng

```bash
ros2 run fairino_hardware ros2_cmd_server
```

## Test gửi lệnh qua service

```bash
ros2 service call /fairino_remote_command_service \
  fairino_msgs/srv/RemoteCmdInterface \
  "{cmd_str: 'GetActualJointPosDegree()'}"
```

## Git workflow

```bash
git add -A
git commit -m "mô tả thay đổi"
git push origin main
```

> ⚠️ Không dùng `--force` trừ khi thật sự cần thiết
