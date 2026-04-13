# Skill: Switch Simulation ↔ Real Robot

> Chuyển đổi giữa chế độ simulation (mô phỏng) và chạy robot thật.

## File cần sửa

```
fairino5_v6_moveit2_config/config/fairino5_v6_robot.ros2_control.xacro
```

## Chế độ Simulation (mặc định)

```xml
<hardware>
    <plugin>mock_components/GenericSystem</plugin>
    <!-- <plugin>fairino_hardware/FairinoHardwareInterface</plugin> -->
</hardware>
```

## Chế độ Real Robot

```xml
<hardware>
    <!-- <plugin>mock_components/GenericSystem</plugin> -->
    <plugin>fairino_hardware/FairinoHardwareInterface</plugin>
</hardware>
```

## Quy trình chuyển đổi

### Simulation → Real Robot

1. Mở file xacro
2. Comment `mock_components/GenericSystem`
3. Uncomment `fairino_hardware/FairinoHardwareInterface`
4. Rebuild:
   ```bash
   colcon build --packages-select fairino5_v6_moveit2_config
   source install/setup.bash
   ```
5. Kiểm tra kết nối: `ping 192.168.57.2`
6. Launch: `ros2 launch fairino5_v6_moveit2_config demo.launch.py`

### Real Robot → Simulation

1. Mở file xacro
2. Uncomment `mock_components/GenericSystem`
3. Comment `fairino_hardware/FairinoHardwareInterface`
4. Rebuild + source

## ⚠️ Cảnh báo quan trọng

- **LUÔN TEST trên simulation trước** khi chạy robot thật
- Khi chuyển sang real, đảm bảo:
  - Robot controller đang bật
  - Network kết nối tốt
  - Không ai đứng trong vùng hoạt động robot
  - Emergency stop button sẵn sàng
- **KHÔNG commit** file xacro ở chế độ real robot (để mặc định simulation cho an toàn)
