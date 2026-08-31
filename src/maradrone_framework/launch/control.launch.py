from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    px4_target = LaunchConfiguration('px4_target')

    declare_px4_target = DeclareLaunchArgument(
        'px4_target',
        default_value='none',
        description='PX4 SITL target. Use none to start PX4 without launching Gazebo.'
    )

    # Connects to an already-running Gazebo instance (started separately via
    # maradrone_gazebo.launch.py, which sets PX4_GZ_MODEL_NAME/PX4_GZ_WORLD and
    # spawns the model) rather than launching its own gz_<model> target.
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

    # offboard_takeoff is self-contained (it publishes /fmu/in/... directly via
    # px4_msgs) and lives in maradrone_framework, not in the external px4_ros_com
    # package, so no extra bridge launch is required here.
    offboard_node = Node(
        package='maradrone_framework',
        executable='offboard_takeoff',
        name='offboard_node',
        output='screen'
    )

    return LaunchDescription([
        declare_px4_target,
        px4_sitl,
        offboard_node
    ])