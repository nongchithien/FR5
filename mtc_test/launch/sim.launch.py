"""
All-in-one simulation launch: mock hardware + MoveGroup + RViz + MTC node.

Usage:
  ros2 launch mtc_test sim.launch.py
"""

import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import ExecuteProcess, TimerAction
from launch_ros.actions import Node
from moveit_configs_utils import MoveItConfigsBuilder


def generate_launch_description():
    moveit_config = (
        MoveItConfigsBuilder("fairino5_v6_robot", package_name="fairino5_v6_moveit2_config")
        .robot_description(file_path="config/fairino5_v6_robot.urdf.xacro")
        .to_moveit_configs()
    )

    fr5_config = os.path.join(
        get_package_share_directory("mtc_test"), "config", "fr5_config.yaml"
    )

    ros2_controllers_path = os.path.join(
        get_package_share_directory("fairino5_v6_moveit2_config"),
        "config",
        "ros2_controllers.yaml",
    )

    # ── robot_state_publisher ───────────────────────────────────────────────
    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        name="robot_state_publisher",
        output="both",
        parameters=[moveit_config.robot_description],
    )

    # ── controller_manager (mock hardware) ──────────────────────────────────
    ros2_control_node = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[moveit_config.robot_description, ros2_controllers_path],
        output="screen",
    )

    # ── move_group with MTC execute capability ──────────────────────────────
    move_group_node = Node(
        package="moveit_ros_move_group",
        executable="move_group",
        output="screen",
        parameters=[
            moveit_config.to_dict(),
            {"capabilities": "move_group/ExecuteTaskSolutionCapability"},
        ],
    )

    # ── RViz ────────────────────────────────────────────────────────────────
    rviz_config_file = os.path.join(
        get_package_share_directory("mtc_test"), "config", "mtc2.rviz"
    )
    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        output="log",
        arguments=["-d", rviz_config_file],
        parameters=[
            moveit_config.robot_description,
            moveit_config.robot_description_semantic,
            moveit_config.robot_description_kinematics,
            moveit_config.planning_pipelines,
        ],
    )

    # ── Static TF world → base_link ─────────────────────────────────────────
    static_tf = Node(
        package="tf2_ros",
        executable="static_transform_publisher",
        name="static_transform_publisher",
        output="log",
        arguments=["0.0", "0.0", "0.0", "0.0", "0.0", "0.0", "world", "base_link"],
    )

    # ── Controller spawners ─────────────────────────────────────────────────
    load_controllers = [
        ExecuteProcess(
            cmd=["ros2 run controller_manager spawner {}".format(c)],
            shell=True,
            output="screen",
        )
        for c in ["joint_state_broadcaster", "fr5_arm_controller", "hand_controller"]
    ]

    # ── MTC node (delayed so stack is fully up) ─────────────────────────────
    mtc_node = TimerAction(
        period=5.0,
        actions=[
            Node(
                package="mtc_test",
                executable="mtc_node",
                output="screen",
                parameters=[
                    moveit_config.robot_description,
                    moveit_config.robot_description_semantic,
                    moveit_config.robot_description_kinematics,
                    moveit_config.planning_pipelines,
                    fr5_config,
                ],
            )
        ],
    )

    return LaunchDescription(
        [
            static_tf,
            robot_state_publisher,
            ros2_control_node,
            move_group_node,
            rviz_node,
            mtc_node,
        ]
        + load_controllers
    )
