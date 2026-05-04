from moveit_configs_utils import MoveItConfigsBuilder
from moveit_configs_utils.launches import generate_move_group_launch


def generate_launch_description():
    moveit_config = (
        MoveItConfigsBuilder("fairino5_v6_robot", package_name="fr5_arm_hand_config")
        .sensors_3d("config/sensors_kinect_pointcloud.yaml")
        .to_moveit_configs()
    )
    return generate_move_group_launch(moveit_config)
