# FAIRINO C++ SDK (fairino-cpp-sdk)

> **Official C++ SDK** provided by [FAIRINO](https://www.fairino.com/) for controlling FAIRINO collaborative robots.
> Licensed under **Apache 2.0**.

---

## Table of Contents

- [Overview](#overview)
- [Directory Structure](#directory-structure)
  - [Root Level](#root-level)
  - [libfairino/ — Source Code (Build from Source)](#libfairino--source-code-build-from-source)
  - [linux/ — Prebuilt Libraries for Linux](#linux--prebuilt-libraries-for-linux)
  - [windows/ — Prebuilt Libraries for Windows](#windows--prebuilt-libraries-for-windows)
- [Source Code Modules](#source-code-modules)
- [Header Files (Public API)](#header-files-public-api)
- [API Categories](#api-categories)
- [How to Build (Linux)](#how-to-build-linux)
  - [Build from Source (x86_64)](#build-from-source-x8664)
  - [Cross-compile for ARM Boards](#cross-compile-for-arm-boards)
- [How to Build (Windows)](#how-to-build-windows)
- [Quick Start Example](#quick-start-example)
- [Official Documentation](#official-documentation)

---

## Overview

This SDK allows you to control FAIRINO collaborative robots (e.g., FR3, FR5, FR10) from a C++ application via **XML-RPC over TCP/UDP**. The robot controller acts as the server, and your application (using this SDK) acts as the client.

**Key facts:**
- **Language**: C++17
- **Communication**: XML-RPC (TCP) + UDP for real-time data
- **Platforms**: Linux (x86_64, ARM aarch64) and Windows (x86, x86-64)
- **SDK Version**: 2.3.4 (as defined in `CMakeLists.txt`)
- **Number of API functions**: ~530+ methods in the `FRRobot` class

---

## Directory Structure

### Root Level

```
fairino-cpp-sdk-main/
├── LICENSE                  # Apache 2.0 license
├── README.md                # This file
├── libfairino/              # 📦 Source code — build the SDK yourself
├── linux/                   # 📁 Prebuilt SDK for Linux (headers + .so libs)
└── windows/                 # 📁 Prebuilt SDK for Windows (headers + .dll/.lib)
```

### `libfairino/` — Source Code (Build from Source)

This is the **most important directory** — it contains the full SDK source code.

```
libfairino/
├── CMakeLists.txt           # Main CMake build file
├── readme.md                # Original build instructions (Chinese)
├── cc_rk3399.cmake          # Cross-compile toolchain for RK3399 (ARM)
├── cc_rk3568.cmake          # Cross-compile toolchain for RK3568 (ARM)
├── cc_rk3588.cmake          # Cross-compile toolchain for RK3588 (ARM)
├── LinuxBuild/              # Build scripts
│   ├── buildGcc.sh          # Build with native GCC (x86_64)
│   ├── build3399.sh         # Build for RK3399
│   ├── build3568.sh         # Build for RK3568
│   ├── build3588.sh         # Build for RK3588
│   ├── buildAndroid17.sh    # Build for Android API 17
│   └── buildAndroid22.sh    # Build for Android API 22
└── src/
    ├── main.cpp             # Example / test program (~363KB, many examples)
    ├── include/             # Header files (see "Source Code Modules" below)
    │   ├── Base/            # Utility headers (base64, md5, etc.)
    │   ├── ComClient/       # TCP/UDP communication headers
    │   ├── Log/             # Logging headers (elog)
    │   ├── Robot-EN/        # 🤖 Robot API headers (English version)
    │   └── XmlRpc/          # XML-RPC protocol headers
    └── src/                 # Implementation source files
        ├── Base/            # Utility implementations
        ├── ComClient/       # TCP/UDP client implementations
        ├── Log/             # Logging implementations (C/C++)
        ├── Robot/           # Robot API implementations
        └── XmlRpc/          # XML-RPC protocol implementations
```

### `linux/` — Prebuilt Libraries for Linux

Use these if you **don't want to build from source**.

```
linux/libfairino/
├── include/                 # Public headers
│   ├── robot.h              # Main API header (214KB)
│   ├── robot_types.h        # Data type definitions (14KB)
│   └── robot_error.h        # Error code definitions (5KB)
├── lib/                     # Prebuilt shared libraries (.so)
│   ├── x86.zip              # Libraries for x86_64
│   ├── arm3399.zip          # Libraries for RK3399 (aarch64)
│   └── arm3568.zip          # Libraries for RK3568 (aarch64)
└── src/
    └── test.cpp             # Minimal usage example
```

### `windows/` — Prebuilt Libraries for Windows

```
windows/libfairino/
├── include/                 # Public headers (same as linux)
│   ├── robot.h
│   ├── robot_types.h
│   └── robot_error.h
└── lib/                     # Prebuilt libraries (.dll / .lib)
    ├── vs2017 x86/
    ├── vs2017 x86-64/
    ├── vs2019 x86/
    ├── vs2019 x86-64/
    ├── vs2022 x86/
    └── vs2022 x86-64/
```

---

## Source Code Modules

| Module | Directory | Description |
|--------|-----------|-------------|
| **Robot** | `src/src/Robot/` | Core robot control logic — motion, device, I/O (`robot.cpp`, `robotMotion.cpp`, `robotIO.cpp`, `robotDevice.cpp`) |
| **ComClient** | `src/src/ComClient/` | Network communication — TCP client (`FRTcpClient`), UDP client (`FRUdpClient`), frame handling |
| **XmlRpc** | `src/src/XmlRpc/` | XML-RPC protocol implementation — client, server, serialization, socket handling |
| **Log** | `src/src/Log/` | Logging framework (`elog`) — file-based logging, async logging |
| **Base** | `src/src/Base/` | Utilities — Base64 encoding, MD5 hashing |

---

## Header Files (Public API)

The 3 most important headers you need to use this SDK:

| Header | Purpose |
|--------|---------|
| **`robot.h`** | Main header — defines the `FRRobot` class with **all** API methods |
| **`robot_types.h`** | Data structures — `JointPos`, `DescPose`, `ExaxisPos`, `SpiralParam`, etc. |
| **`robot_error.h`** | Error codes — `ERR_SUCCESS (0)`, `ERR_XMLRPC_COM_FAILED (-3)`, etc. |

> **Note:** There are two versions of the Robot headers in the source tree:
> - `Robot-EN/` — English comments (**recommended**)
> - The CMakeLists references `Robot-CN/` for the build (Chinese comments)

---

## API Categories

The `FRRobot` class exposes ~530+ methods. Here is a high-level grouping:

| Category | Example Methods | Description |
|----------|----------------|-------------|
| **Connection** | `RPC()`, `CloseRPC()` | Connect/disconnect to robot controller |
| **Info** | `GetSDKVersion()`, `GetControllerIP()`, `GetFirmwareVersion()` | Query robot & SDK info |
| **Motion — Basic** | `MoveJ()`, `MoveL()`, `MoveC()`, `Circle()` | Joint, linear, circular motions |
| **Motion — Advanced** | `ServoJ()`, `ServoCart()`, `NewSpiral()`, `MoveCart()` | Servo control, spiral, Cartesian |
| **Motion — Spline** | `SplineStart()`, `SplinePTP()`, `NewSplineStart()`, `NewSplinePoint()` | Smooth spline trajectories |
| **Motion — Control** | `StopMotion()`, `PauseMotion()`, `ResumeMotion()`, `SetSpeed()` | Motion flow control |
| **JOG** | `StartJOG()`, `StopJOG()`, `ImmStopJOG()` | Manual jogging |
| **I/O — Digital** | `SetDO()`, `GetDI()`, `SetToolDO()`, `GetToolDI()` | Digital I/O control |
| **I/O — Analog** | `SetAO()`, `GetAI()`, `SetToolAO()`, `GetToolAI()` | Analog I/O control |
| **I/O — Auxiliary** | `SetAuxDO()`, `GetAuxDI()`, `SetAuxAO()`, `GetAuxAI()` | Auxiliary I/O |
| **Tool/Workpiece** | `SetToolCoord()`, `SetWObjCoord()`, `SetExToolCoord()` | Tool & workpiece coordinate systems |
| **Force Sensor** | `FT_SetConfig()`, `FT_Guard()`, `FT_ComplianceStart()`, `FT_FindSurface()` | Force/torque sensor operations |
| **Welding** | `WeldingSetProcessParam()`, `WeaveStart()`, `SegmentWeldStart()` | Welding process control |
| **Gripper** | `MoveGripper()`, `GetGripperCurPosition()`, `SetGripperConfig()` | Gripper control |
| **Drag Teaching** | `DragTeachSwitch()`, `IsInDragTeach()` | Drag teaching mode |
| **Program** | `ProgramLoad()`, `ProgramRun()`, `ProgramStop()`, `ProgramPause()` | Program execution |
| **Lua Scripts** | `LuaUpload()`, `LuaDownLoad()`, `LuaDelete()` | Lua script management |
| **Trajectory** | `LoadTPD()`, `MoveTPD()`, `LoadTrajectoryJ()`, `MoveTrajectoryJ()` | Trajectory recording & playback |
| **Laser Tracking** | `LaserTrackingTrackOn()`, `LaserSensorRecord()` | Laser seam tracking |
| **Impedance** | `ImpedanceControlStartStop()`, `ForceAndJointImpedanceStartStop()` | Impedance control |
| **Kinematics** | `GetForwardKin()`, `GetInverseKin()`, `GetInverseKinRef()` | Forward/inverse kinematics |
| **Safety** | `SetAnticollision()`, `SetCollisionStrategy()`, `GetRobotEmergencyStopState()` | Collision detection & safety |
| **External Axis** | `SetExAxisRobotPlan()`, `GetCurExAxisCoord()` | External axis control |
| **FieldBus** | `FieldBusSlaveReadDI()`, `FieldBusSlaveWriteDO()` | FieldBus slave I/O |
| **System** | `RobotEnable()`, `Mode()`, `ResetAllError()`, `ShutDownRobotOS()` | System-level operations |

---

## How to Build (Linux)

### Prerequisites

- CMake >= 3.10
- GCC with C++17 support (GCC 7+)
- pthread library

### Build from Source (x86_64)

```bash
cd libfairino
mkdir -p build && cd build
cmake ..
make
```

**Output:**
- Shared library: `LinuxBuild/bin/libfairino.so.2.3.4`
- Test executable: `LinuxBuild/bin/test_fairino`

Or use the provided script:

```bash
cd libfairino/LinuxBuild
bash buildGcc.sh
```

This will build the library and copy the output to `LinuxBuild/lib/` and `LinuxBuild/include/`.

### Cross-compile for ARM Boards

For embedded ARM boards (e.g., RK3399, RK3568, RK3588):

```bash
cd libfairino/LinuxBuild

# For RK3399
bash build3399.sh

# For RK3568
bash build3568.sh

# For RK3588
bash build3588.sh
```

> **Note:** You need the corresponding cross-compiler toolchain installed at `/usr/local/toolchain/`.

---

## How to Build (Windows)

Using Visual Studio with CMake:

```cmd
cd libfairino
mkdir build && cd build

:: For x86
cmake .. -A win32
cmake --build . --config Release

:: For x64
cmake .. -A x64
cmake --build . --config Release
```

Or use the prebuilt libraries from `windows/libfairino/lib/` for your Visual Studio version.

---

## Quick Start Example

```cpp
#include "robot.h"
#include "robot_error.h"
#include <iostream>

int main() {
    FRRobot robot;

    // Connect to the robot controller (replace with your robot's IP)
    errno_t err = robot.RPC("192.168.57.2");
    if (err != ERR_SUCCESS) {
        std::cerr << "RPC connection failed: " << err << "\n";
        return 1;
    }

    // Get SDK version
    char version[64] = {0};
    robot.GetSDKVersion(version);
    std::cout << "SDK version: " << version << "\n";

    // Close connection
    robot.CloseRPC();
    return 0;
}
```

> **Tip:** See `libfairino/src/main.cpp` for many more usage examples including motion commands, welding, I/O, force sensor, gripper, and more.

---

## Official Documentation

📖 **C++ SDK Docs**: [https://fair-documentation.readthedocs.io/en/latest/SDKManual/cpp_intro.html](https://fair-documentation.readthedocs.io/en/latest/SDKManual/cpp_intro.html)

🌐 **FAIRINO Website**: [https://www.fairino.com/](https://www.fairino.com/)
