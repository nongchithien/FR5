import rclpy
from rclpy.node import Node
from visualization_msgs.msg import Marker, MarkerArray

class PointVisualizer(Node):
    def __init__(self):
        super().__init__('point_visualizer')
        self.publisher_ = self.create_publisher(MarkerArray, 'visualization_marker_array', 10)
        self.timer = self.create_timer(1.0, self.timer_callback)
        
        # Nhập các điểm DescPose trong C++ SDK của bạn (Đơn vị: mm)
        self.points_mm = [
            (-419.524, -13.000, 351.569),    # desc_pos1
            (-321.222, 185.189, 335.520),    # desc_pos2
            (-487.434, 154.362, 308.576),    # desc_pos3
            (-443.165, 147.881, 480.951)     # desc_pos4
        ]

    def timer_callback(self):
        marker_array = MarkerArray()
        
        for i, (x, y, z) in enumerate(self.points_mm):
            marker = Marker()
            marker.header.frame_id = "base_link" # Lấy gốc tọa độ của robot làm chuẩn
            marker.header.stamp = self.get_clock().now().to_msg()
            marker.ns = "test_points"
            marker.id = i
            marker.type = Marker.SPHERE
            marker.action = Marker.ADD
            
            # Lưu ý đổi sang mét cho RViz
            marker.pose.position.x = x / 1000.0
            marker.pose.position.y = y / 1000.0
            marker.pose.position.z = z / 1000.0
            marker.pose.orientation.w = 1.0 # Bỏ qua góc quay cổ tay RPY để tối giản hình hiển thị
            
            # Kích thước khối biểu tượng (ví dụ 5cm)
            marker.scale.x = 0.05 
            marker.scale.y = 0.05
            marker.scale.z = 0.05
            
            # Đặt màu xanh lá chói
            marker.color.r = 0.0
            marker.color.g = 1.0
            marker.color.b = 0.0
            marker.color.a = 0.8
            
            marker_array.markers.append(marker)
            
        self.publisher_.publish(marker_array)

def main(args=None):
    rclpy.init(args=args)
    node = PointVisualizer()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
