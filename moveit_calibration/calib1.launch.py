""" Static transform publisher acquired via MoveIt 2 hand-eye calibration """
""" EYE-IN-HAND: connector_link -> camera_color_optical_frame """
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description() -> LaunchDescription:
    nodes = [
        Node(
            package="tf2_ros",
            executable="static_transform_publisher",
            output="log",
            arguments=[
                "--frame-id",
                "connector_link",
                "--child-frame-id",
                "camera_color_optical_frame",
                "--x",
                "-0.282734",
                "--y",
                "-0.149793",
                "--z",
                "0.233673",
                "--qx",
                "0.116639",
                "--qy",
                "0.0936385",
                "--qz",
                "0.0149766",
                "--qw",
                "0.988637",
                # "--roll",
                # "0.23412",
                # "--pitch",
                # "0.18978",
                # "--yaw",
                # "0.00791115",
            ],
        ),
    ]
    return LaunchDescription(nodes)
