from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess, IncludeLaunchDescription
from launch.launch_description_sources import YamlLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    px4_target = LaunchConfiguration('px4_target')

    declare_px4_target = DeclareLaunchArgument(
        'px4_target',
        default_value='none',
        description='PX4 SITL target. Use none to start PX4 without launching Gazebo.'
    )

    px4_sitl = ExecuteProcess(
        cmd=[
            'bash',
            '-lc',
            [
                'source /opt/ros/humble/setup.bash && source /root/ros2_ws/install/setup.bash && ',
                'cd /root/PX4-Autopilot && make px4_sitl_default ',
                px4_target,
            ]
        ],
        output='screen'
    )

    px4_bridge = IncludeLaunchDescription(
        YamlLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('px4_ros_com'),
                'launch',
                'offboard_control_launch.yaml'
            )
        )
    )

    offboard_node = Node(
        package='maradrone_control',
        executable='offboard_takeoff',
        name='offboard_node',
        output='screen'
    )

    return LaunchDescription([
        declare_px4_target,
        px4_sitl,
        px4_bridge,
        offboard_node
    ])