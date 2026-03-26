#include "robot.h"
#include "robot_error.h"
#include <iostream>

int main() {
    FRRobot robot;

    errno_t err = robot.RPC("192.168.57.2");
    if (err != ERR_SUCCESS) {
        std::cerr << "RPC failed: " << err << "\n";
        return 1;
    }

    char version[64] = {0};
    robot.GetSDKVersion(version);
    std::cout << "SDK version: " << version << "\n";

    robot.CloseRPC();
    return 0;
}