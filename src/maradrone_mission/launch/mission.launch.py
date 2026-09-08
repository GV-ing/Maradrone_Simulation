from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    default_waypoints_file = PathJoinSubstitution(
        [FindPackageShare('maradrone_mission'), 'config', 'waypoints.yaml']
    )

    declare_waypoints_file = DeclareLaunchArgument(
        'waypoints_file',
        default_value=default_waypoints_file,
        description='Path to a YAML file with the waypoint_mission parameters (see config/waypoints.yaml).'
    )

    # Assumes PX4 SITL + Gazebo are already running (e.g. via
    # maradrone_gazebo.launch.py + control.launch.py, or the native
    # `make px4_sitl gz_x500` / `gz_x500_depth` workflow).
    waypoint_mission_node = Node(
        package='maradrone_mission',
        executable='waypoint_mission',
        name='waypoint_mission',
        output='screen',
        parameters=[LaunchConfiguration('waypoints_file')],
    )

    return LaunchDescription([
        declare_waypoints_file,
        waypoint_mission_node,
    ])
