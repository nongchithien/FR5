# Skill: Thêm SDK Function vào Command Server

> Quy trình thêm một Fairino SDK API function mới vào ROS 2 command server.

## Tổng quan

Command server (`command_server.cpp`) wrap các SDK function thành ROS 2 service calls.
Mỗi function nhận string parameters và trả về string result.

## Quy trình 3 bước

### Bước 1: Khai báo method trong header

**File**: `fairino_hardware/include/fairino_hardware/command_server.hpp`

```cpp
// Thêm khai báo method
std::string NewFunctionName(std::string para);
```

### Bước 2: Implement trong source

**File**: `fairino_hardware/src/command_server.cpp`

```cpp
/**
 * @brief Description of the function
 * @param[in] para - param1,param2,param3 separated by comma
 * @return "0" on success, error code string on failure
 */
std::string robot_command_thread::NewFunctionName(std::string para) {
    // Parse parameters từ string
    std::list<std::string> list;
    _splitString2List(para, list);
    
    // Lấy từng param
    int param1 = std::stoi(list.front()); list.pop_front();
    double param2 = std::stod(list.front()); list.pop_front();
    
    // Nếu cần DescPose (6 values: x,y,z,rx,ry,rz)
    // DescPose pose;
    // _fillDescPose(list, pose);
    
    // Nếu cần JointPos (6 values: j1-j6)
    // JointPos jpos;
    // _fillJointPose(list, jpos);
    
    // Gọi SDK function
    return std::to_string(_ptr_robot->SdkFunctionName(param1, param2));
}
```

### Bước 3: Register vào function map

**File**: `fairino_hardware/include/fairino_hardware/command_server.hpp`

Tìm `_fr_function_list` map và thêm entry:

```cpp
{"NewFunctionName", &robot_command_thread::NewFunctionName}
```

## Helper functions có sẵn

| Function | Mục đích |
|----------|---------|
| `_splitString2List(str, list)` | Split string by `,` → `std::list<std::string>` |
| `_splitString2Vec(str, vec)` | Split string by `,` → `std::vector<std::string>` |
| `_fillDescPose(list, pose)` | Pop 6 values → `DescPose` (x,y,z,rx,ry,rz) |
| `_fillDescTran(list, tran)` | Pop 3 values → `DescTran` (x,y,z) |
| `_fillJointPose(list, pos)` | Pop 6 values → `JointPos` (j1-j6) |

## Ví dụ thực tế: SetSpeed

```cpp
// Header: std::string SetSpeed(std::string para);
// Map: {"SetSpeed", &robot_command_thread::SetSpeed}

std::string robot_command_thread::SetSpeed(std::string para) {
    return std::to_string(_ptr_robot->SetSpeed(std::stoi(para)));
}
```

## Ví dụ phức tạp: SetToolCoord

```cpp
std::string robot_command_thread::SetToolCoord(std::string para) {
    // Lấy thêm params từ ROS parameter server
    std::string install = this->get_parameter("toolcoord_install").value_to_string();
    std::string type = this->get_parameter("toolcoord_type").value_to_string();
    para = para + "," + type + "," + install;

    std::list<std::string> datalist;
    _splitString2List(para, datalist);
    
    int id = std::stoi(datalist.front()); datalist.pop_front();
    DescPose trans;
    _fillDescPose(datalist, trans);
    int typei = std::stoi(datalist.front()); datalist.pop_front();
    int installi = std::stoi(datalist.front());
    
    return std::to_string(_ptr_robot->SetToolCoord(id, &trans, typei, installi, 0, 0.));
}
```

## Build & test

```bash
# Build
colcon build --packages-select fairino_hardware

# Test gọi function mới
ros2 service call /fairino_remote_command_service \
  fairino_msgs/srv/RemoteCmdInterface \
  "{cmd_str: 'NewFunctionName(param1,param2)'}"
```

## Checklist

- [ ] Method khai báo trong header
- [ ] Method implement trong source (có error handling)
- [ ] Registered trong `_fr_function_list`
- [ ] Build thành công
- [ ] Test gọi qua service
