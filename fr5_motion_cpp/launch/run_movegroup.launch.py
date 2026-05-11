"""
Headless MoveIt2 stack + fr5_movegroup_node (no RViz).
Includes: robot_state_publisher + ros2_control (mock) + move_group + controllers.

Usage:
  ros2 launch fr5_motion_cpp run_movegroup.launch.py
"""

import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, TimerAction, SetEnvironmentVariable
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from moveit_configs_utils import MoveItConfigsBuilder


def generate_launch_description():
    fastdds_profile = os.path.join(
        os.path.expanduser("~/frcobot_ros2-main"), "fastdds.xml"
    )
    cfg_pkg = get_package_share_directory("fairino5_v6_moveit2_config")

    set_fastdds = SetEnvironmentVariable(
        name="FASTRTPS_DEFAULT_PROFILES_FILE",
        value=fastdds_profile,
    )

    moveit_config = (
        MoveItConfigsBuilder("fairino5_v6_robot", package_name="fairino5_v6_moveit2_config")
        .robot_description(file_path="config/fairino5_v6_robot.urdf.xacro")
        .to_moveit_configs()
    )

    def include(launch_file):
        return IncludeLaunchDescription(
            PythonLaunchDescriptionSource(os.path.join(cfg_pkg, "launch", launch_file))
        )

    rsp        = include("rsp.launch.py")
    move_group = include("move_group.launch.py")
    spawn_ctrl = include("spawn_controllers.launch.py")

    # controller_manager (mock hardware) — required for trajectory execution
    ros2_control_node = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[
            moveit_config.robot_description,
            os.path.join(cfg_pkg, "config", "ros2_controllers.yaml"),
        ],
        output="screen",
    )

    motion_node = TimerAction(
        period=4.0,
        actions=[
            Node(
                package="fr5_motion_cpp",
                executable="fr5_movegroup_node",
                output="screen",
            )
        ],
    )

    return LaunchDescription([set_fastdds, rsp, ros2_control_node, move_group, spawn_ctrl, motion_node])
