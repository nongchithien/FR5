# Skill: Gửi lệnh Cartesian qua Service

> Quy trình gửi lệnh di chuyển Cartesian (XYZ + RPY) đến robot Fairino qua ROS 2 service.

## Yêu cầu

- `ros2_cmd_server` đang chạy (hoặc sử dụng hardware interface)
- Service `fairino_remote_command_service` available

## Quy trình 2 bước

### Bước 1: Lưu điểm Cartesian vào ô nhớ

```
CARTPoint(idx, x, y, z, rx, ry, rz)
```

- `idx`: Số thứ tự ô nhớ (1, 2, 3, ...)
- `x, y, z`: Tọa độ vị trí (mm)
- `rx, ry, rz`: Góc quay (degree)
- **KHÔNG CÓ SPACE**

Ví dụ:
```bash
ros2 service call /fairino_remote_command_service \
  fairino_msgs/srv/RemoteCmdInterface \
  "{cmd_str: 'CARTPoint(1,-419.524,-13.000,351.569,-178.118,0.314,3.833)'}"
```

### Bước 2: Di chuyển đến điểm

```
MoveL(CART[idx], speed, tool_id, user_id)
```

- `CART[idx]`: Tham chiếu đến ô nhớ (vd: `CART1`)
- `speed`: Tốc độ % (0-100, khuyến nghị ≤ 20 khi test)
- `tool_id`: Tool coordinate ID (thường = 0)
- `user_id`: User/workpiece coordinate ID (thường = 0)

Ví dụ:
```bash
ros2 service call /fairino_remote_command_service \
  fairino_msgs/srv/RemoteCmdInterface \
  "{cmd_str: 'MoveL(CART1,20,0,0)'}"
```

## Python script mẫu

```python
#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from fairino_msgs.srv import RemoteCmdInterface

class MoveNode(Node):
    def __init__(self):
        super().__init__('move_node')
        self.cli = self.create_client(RemoteCmdInterface, 'fairino_remote_command_service')
        while not self.cli.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Waiting for service...')

    def send(self, cmd):
        req = RemoteCmdInterface.Request()
        req.cmd_str = cmd
        future = self.cli.call_async(req)
        rclpy.spin_until_future_complete(self, future)
        return future.result().cmd_res

def main():
    rclpy.init()
    node = MoveNode()
    
    # Lưu điểm
    node.send("CARTPoint(1,-419.524,-13.000,351.569,-178.118,0.314,3.833)")
    
    # Di chuyển
    result = node.send("MoveL(CART1,20,0,0)")
    print(f"Result: {result}")  # "0" = success
    
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
```

## Các lệnh di chuyển khác

| Lệnh | Mô tả |
|-------|-------|
| `MoveJ(JNT[idx],speed,tool,user)` | Di chuyển joint space |
| `MoveL(CART[idx],speed,tool,user)` | Di chuyển Cartesian thẳng |
| `MoveC(CART[idx1],CART[idx2],speed,tool,user)` | Di chuyển Cartesian cung tròn |

## ⚠️ Lưu ý

- Tốc độ ≤ 20% khi test lần đầu
- Kiểm tra tọa độ target không nằm ngoài workspace robot
- Đảm bảo đường đi không có vật cản
- Emergency stop button sẵn sàng
