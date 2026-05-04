import os

from moveit_configs_utils import MoveItConfigsBuilder
from moveit_configs_utils.launches import generate_demo_launch

from ament_index_python.packages import get_package_share_directory
from launch.actions import IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource


def generate_launch_description():
    moveit_config = (
        MoveItConfigsBuilder("fairino5_v6_robot", package_name="fr5_arm_hand_config")
        .sensors_3d("config/sensors_kinect_pointcloud.yaml")
        .to_moveit_configs()
    )
    ld = generate_demo_launch(moveit_config)

    orbbec_launch_path = get_package_share_directory("orbbec_camera")
    config_path = os.path.join(
        get_package_share_directory("fr5_arm_hand_config"),
        "config",
        "orbbec_low_cpu.yaml",
    )
    orbbec_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            [orbbec_launch_path, "/launch/gemini_330_series_low_cpu.launch.py"]
        ),
        launch_arguments={"config_file_path": config_path}.items(),
    )
    ld.add_action(TimerAction(period=5.0, actions=[orbbec_launch]))

    return ld
