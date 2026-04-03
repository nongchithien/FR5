#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from fairino_msgs.srv import RemoteCmdInterface
import sys
import time

# =====================================================================
# KHU VỰC CẤU HÌNH DÀNH CHO USER (CHỈNH SỬA TỌA ĐỘ VÀ THÔNG SỐ Ở ĐÂY)
# =====================================================================

# Tọa độ XYZ (Đơn vị: mm) theo chuẩn struct DescPose
TARGET_X = -419.524
TARGET_Y = -13.000
TARGET_Z = 351.569

# Góc quay cổ tay XYZ_RPY (Đơn vị: Độ - Degree) theo chuẩn struct DescPose
TARGET_RX = -178.118
TARGET_RY = 0.314
TARGET_RZ = 3.833

# Cài đặt di chuyển
SPEED_PERCENT = 20  # Tốc độ di chuyển chặn an toàn ở 20%
TOOL_ID = 0         # ID của Tool (Tool Coordinate)
USER_ID = 0         # ID của Base/Workpiece (User Coordinate)

# =====================================================================

class MoveCartesianNode(Node):
    def __init__(self):
        super().__init__('move_cartesian_node')
        
        # Tạo Service Client gọi đến hệ thống của Fairino
        self.cli = self.create_client(RemoteCmdInterface, 'fairino_remote_command_service')
        
        # Chờ đợi Service sẵn sàng (đảm bảo command_server_node đang chạy)
        while not self.cli.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Đang chờ service fairino_remote_command_service khởi động...')
        self.req = RemoteCmdInterface.Request()

    def send_command(self, cmd_string):
        """Gửi chuỗi lệnh xuống tay máy và chờ kết quả"""
        self.get_logger().info(f'Đang gửi lệnh: "{cmd_string}"')
        
        self.req.cmd_str = cmd_string
        future = self.cli.call_async(self.req)
        rclpy.spin_until_future_complete(self, future)
        
        res = future.result()
        if res is not None:
            if res.cmd_res == "0":
                self.get_logger().info(f'[{cmd_string}] -> THÀNH CÔNG (0)')
                return True
            else:
                self.get_logger().error(f'[{cmd_string}] -> THẤT BẠI (-1) - Vui lòng kiểm tra lại log C++')
                return False
        else:
            self.get_logger().error(f'Service call failed: Không nhận được phản hồi')
            return False

def main(args=None):
    rclpy.init(args=args)
    action_node = MoveCartesianNode()

    # Bước 1: Ghép chuỗi tạo Điểm Cartesian Point (CARTPoint)
    # Cú pháp mẫu của SDK: CARTPoint(idx,x,y,z,rx,ry,rz)
    # LƯU Ý QUAN TRỌNG: C++ Fairino chặn không cho dấu cách (space) lọt vào tham số
    point_cmd = f"CARTPoint(1,{TARGET_X},{TARGET_Y},{TARGET_Z},{TARGET_RX},{TARGET_RY},{TARGET_RZ})"
    
    # Bước 2: Ghép chuỗi Lệnh di chuyển theo đường thẳng (MoveL)
    # Cú pháp mẫu của SDK: MoveL(CART[idx],speed,tool,user)
    move_cmd = f"MoveL(CART1,{SPEED_PERCENT},{TOOL_ID},{USER_ID})"

    # --- CHẠY QUY TRÌNH ---
    action_node.get_logger().info('--- BẮT ĐẦU CHƯƠNG TRÌNH GỬI TỌA ĐỘ ---')
    time.sleep(1) # Nghỉ 1s an toàn
    
    # Gửi toạ độ xuống tủ tay máy lưu vào ô nhớ 1 (CART1)
    status_point = action_node.send_command(point_cmd)
    
    if status_point:
        # Nếu lưu điểm thành công, tiến hành nội suy di chuyển (MoveL) đến ô nhớ số 1
        action_node.get_logger().info('Lưu vị trí thành công, chuẩn bị chuyển động MoveL...')
        action_node.send_command(move_cmd)
    
    action_node.get_logger().info('--- KẾT THÚC CHƯƠNG TRÌNH ---')
    action_node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
