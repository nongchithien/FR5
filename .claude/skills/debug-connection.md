# Skill: Debug Connection

> Chẩn đoán và sửa lỗi kết nối giữa workstation và Fairino robot controller.

## Bước 1: Kiểm tra network cơ bản

```bash
# Ping robot controller
ping -c 3 192.168.57.2

# Kiểm tra interface network
ip addr show

# Workstation PHẢI ở subnet 192.168.57.x
# Nếu không → cấu hình static IP cho interface kết nối robot
```

## Bước 2: Kiểm tra ports

```bash
# XML-RPC port (SDK communication)
nc -zv 192.168.57.2 20004

# State feedback port
nc -zv 192.168.57.2 20005

# Nếu timeout → robot chưa bật hoặc firewall chặn
```

## Bước 3: Kiểm tra firewall

```bash
# Tắt firewall tạm thời (để test)
sudo ufw disable

# Hoặc mở port cụ thể
sudo ufw allow 20004
sudo ufw allow 20005
```

## Bước 4: Chạy và đọc log

```bash
ros2 run fairino_hardware ros2_cmd_server 2>&1 | tee /tmp/robot_log.txt

# Tìm lỗi cụ thể
grep -i "error\|failed\|socket" /tmp/robot_log.txt
```

## Lỗi thường gặp

### "socket create failed"
- **Nguyên nhân**: Không tạo được TCP socket
- **Giải pháp**: Kiểm tra network interface, ping robot, restart robot controller

### "Robot connect failed"
- **Nguyên nhân**: XML-RPC connection refused
- **Giải pháp**: 
  1. Kiểm tra IP robot có đúng `192.168.57.2`
  2. Robot controller có đang bật?
  3. Có process khác đang chiếm port?

### "读取初始关节角度错误" / "Failed to read initial joint position"
- **Nguyên nhân**: SDK connected nhưng không đọc được state
- **Giải pháp**: Kiểm tra robot có ở trạng thái sẵn sàng (enabled, no error)

### "反馈状态数据帧长度" / "State feedback frame size mismatch"
- **Nguyên nhân**: Version mismatch giữa SDK và robot firmware
- **Giải pháp**: Cập nhật SDK hoặc robot firmware cho khớp version

## Checklist nhanh

- [ ] Robot controller đã bật?
- [ ] Cable Ethernet đã cắm?
- [ ] Workstation IP đúng subnet (192.168.57.x)?
- [ ] Ping 192.168.57.2 thành công?
- [ ] Không có process khác đang dùng SDK?
- [ ] Robot không ở trạng thái error?
