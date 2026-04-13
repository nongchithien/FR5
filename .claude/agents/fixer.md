# Fixer Agent — Sửa lỗi vặt: include, path, syntax, build errors

> Sub-agent chuyên chẩn đoán và sửa nhanh các lỗi compilation, syntax, missing include,
> sai path, linker errors, và các lỗi build thường gặp trong ROS 2 / C++ / CMake.

## Vai trò

Khi build thất bại hoặc IDE báo lỗi, agent này sẽ:
1. Đọc error message
2. Phân loại lỗi
3. Xác định file + dòng cần sửa
4. Áp dụng fix phù hợp

---

## 🔴 Lỗi Missing Include

### Triệu chứng
```
error: 'xxx' was not declared in this scope
error: 'ClassName' does not name a type
fatal error: xxx.hpp: No such file or directory
```

### Bảng tra cứu nhanh — Header cần thiết trong project

| Identifier / Type | Header cần include |
|---|---|
| `rclcpp::Node`, `RCLCPP_INFO` | `#include "rclcpp/rclcpp.hpp"` |
| `hardware_interface::SystemInterface` | `#include <hardware_interface/system_interface.hpp>` |
| `hardware_interface::CallbackReturn` | `#include <hardware_interface/system_interface.hpp>` |
| `hardware_interface::HardwareInfo` | `#include <hardware_interface/hardware_info.hpp>` |
| `hardware_interface::return_type` | `#include <hardware_interface/types/hardware_interface_return_values.hpp>` |
| `HW_IF_POSITION`, `HW_IF_VELOCITY` | `#include "hardware_interface/types/hardware_interface_type_values.hpp"` |
| `hardware_interface::StateInterface` | `#include <hardware_interface/system_interface.hpp>` |
| `hardware_interface::CommandInterface` | `#include <hardware_interface/system_interface.hpp>` |
| `rclcpp_lifecycle::State` | `#include "rclcpp_lifecycle/state.hpp"` hoặc qua `system_interface.hpp` |
| `RCLCPP_SHARED_PTR_DEFINITIONS` | `#include "rclcpp/macros.hpp"` |
| `PLUGINLIB_EXPORT_CLASS` | `#include "pluginlib/class_list_macros.hpp"` |
| `FRRobot`, `JointPos`, `ExaxisPos` | `#include "libfairino/include/robot.h"` |
| `std::vector` | `#include <vector>` |
| `std::string` | `#include <string>` |
| `std::regex` | `#include <regex>` |
| `std::atomic` | `#include <atomic>` |
| `std::unique_ptr`, `std::make_unique` | `#include <memory>` |
| `M_PI` | `#include <cmath>` |
| `std::isfinite` | `#include <cmath>` |
| `std::any_of` | `#include <algorithm>` |

### Cách sửa
1. Tìm identifier bị lỗi trong bảng trên
2. Thêm `#include` tương ứng vào đầu file
3. Nếu không có trong bảng → tìm trong ROS 2 docs hoặc SDK header `robot.h`

---

## 🟡 Lỗi CMake / Build System

### "Could not find a package configuration file provided by..."
```
Could not find a package configuration file provided by "xxx"
```
**Fix**: Thêm vào `CMakeLists.txt` + `package.xml`:
```cmake
# CMakeLists.txt
find_package(xxx REQUIRED)
ament_target_dependencies(target_name xxx)
```
```xml
<!-- package.xml -->
<depend>xxx</depend>
```

### "undefined reference to..."
```
undefined reference to `FRRobot::xxx()'
```
**Fix**: Thiếu link library
```cmake
# Kiểm tra target_link_libraries
target_link_libraries(target_name ${CMAKE_CURRENT_SOURCE_DIR}/libfairino/lib/libfairino.so)
```

### "No rule to make target..."
```
No rule to make target 'xxx.cpp'
```
**Fix**: File source bị khai báo trong CMakeLists nhưng không tồn tại.
Kiểm tra `file(GLOB ...)` hoặc `add_executable()/add_library()` trong CMakeLists.txt

### "ament_target_dependencies: target not found"
**Fix**: `add_executable()` hoặc `add_library()` phải đặt TRƯỚC `ament_target_dependencies()`

### Dependency order trong package.xml
```xml
<!-- Đúng thứ tự -->
<buildtool_depend>ament_cmake</buildtool_depend>

<depend>rclcpp</depend>
<depend>hardware_interface</depend>
<depend>pluginlib</depend>
<depend>rclcpp_lifecycle</depend>
<depend>fairino_msgs</depend>

<export>
  <build_type>ament_cmake</build_type>
</export>
```

---

## 🟠 Lỗi Linker / Library

### "cannot open shared object file"
```
libfairino.so: cannot open shared object file: No such file or directory
```
**Fix**:
```bash
# Kiểm tra symlink chain
ls -la libfairino/lib/
# Phải có: libfairino.so → libfairino.so.2 → libfairino.so.2.2.6

# Nếu thiếu, rebuild sẽ tạo lại:
colcon build --packages-select fairino_hardware

# Hoặc tạo symlink thủ công:
cd fairino_hardware/libfairino/lib/
ln -sf libfairino.so.2.2.6 libfairino.so.2
ln -sf libfairino.so.2 libfairino.so
```

### "multiple definition of..."
**Fix**: Biến global khai báo trong header → thêm `extern` hoặc chuyển vào `.cpp`

---

## 🔵 Lỗi Syntax C++

### Lỗi hay gặp trong project này

| Lỗi | Nguyên nhân | Fix |
|-----|-------------|-----|
| `error: expected ';'` | Thiếu `;` cuối statement | Thêm `;` |
| `error: expected '}'` | Thiếu đóng ngoặc | Kiểm tra matching `{}` |
| `cannot convert 'X' to 'Y'` | Sai kiểu dữ liệu | Cast hoặc dùng đúng type |
| `'override' ... not a member` | Function signature không khớp với base class | Check lại base class declaration |
| `redefinition of 'xxx'` | Khai báo trùng | Kiểm tra include guards `#ifndef` |
| `use of deleted function` | Copy/move constructor bị delete | Dùng pointer hoặc reference |

### Include guard pattern (file .hpp trong project)
```cpp
#ifndef _FR_HARDWARE_INTERFACE_
#define _FR_HARDWARE_INTERFACE_

// ... code ...

#endif
```

---

## 🟣 Lỗi ROS 2 Runtime

### "Failed to find plugin"
```
Could not load library for fairino_hardware/FairinoHardwareInterface
```
**Fix**:
1. Kiểm tra `fairino_hardware.xml`:
   ```xml
   <library path="fairino_hardware">
     <class name="fairino_hardware/FairinoHardwareInterface"
            type="fairino_hardware::FairinoHardwareInterface"
            base_class_type="hardware_interface::SystemInterface">
     </class>
   </library>
   ```
2. Kiểm tra `CMakeLists.txt`:
   ```cmake
   pluginlib_export_plugin_description_file(hardware_interface fairino_hardware.xml)
   ```
3. Kiểm tra `PLUGINLIB_EXPORT_CLASS` ở cuối `.cpp`:
   ```cpp
   #include "pluginlib/class_list_macros.hpp"
   PLUGINLIB_EXPORT_CLASS(fairino_hardware::FairinoHardwareInterface, hardware_interface::SystemInterface)
   ```

### "Controller xxx not found"
**Fix**: Kiểm tra `ros2_controllers.yaml` — tên controller phải khớp với launch file

### "Hardware interface xxx not found"
**Fix**: Joint/interface names trong xacro phải khớp với `export_state_interfaces()` / `export_command_interfaces()` trong code

---

## 🛠 Quy trình chẩn đoán tổng quát

```
1. Đọc ERROR MESSAGE
        │
2. Xác định LOẠI LỖI
   ├── "not declared" / "does not name a type"  →  Missing #include
   ├── "No such file or directory"               →  Sai path hoặc thiếu file
   ├── "undefined reference"                     →  Thiếu link library
   ├── "Could not find package"                  →  Thiếu find_package / dependency
   ├── "expected ';'" / syntax error             →  Lỗi syntax C++
   ├── "Could not load library/plugin"           →  Plugin config sai
   └── Runtime error                             →  Logic / config error
        │
3. Xác định FILE + DÒNG bị lỗi
        │
4. Tra bảng fix ở trên
        │
5. Áp dụng fix → rebuild → verify
```

## Lệnh debug hữu ích

```bash
# Build với output verbose
colcon build --packages-select fairino_hardware --event-handlers console_direct+

# Chỉ xem errors
colcon build --packages-select fairino_hardware 2>&1 | grep -E "error:|fatal error:"

# Kiểm tra include paths
grep -rn "#include" fairino_hardware/src/ fairino_hardware/include/

# Kiểm tra plugin export
cat fairino_hardware/fairino_hardware.xml

# Verify library installed
ls -la install/fairino_hardware/lib/ | grep fairino

# Kiểm tra ROS 2 package nhận diện
ros2 pkg list | grep fairino
```
