from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare

# Convenience launch: bridges the x500_depth camera sensors and starts
# RTAB-Map RGB-D SLAM on top of them.
#
# Prerequisite (not started by this launch file): PX4 SITL + Gazebo running
# with the x500_depth model, e.g.
#   cd /root/PX4-Autopilot && PX4_GZ_WORLD=leonardo_race_field make px4_sitl gz_x500_depth


def generate_launch_description():
    pkg_share = FindPackageShare('maradrone_slam')

    bridge_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([pkg_share, 'launch', 'x500_depth_bridge.launch.py'])
        )
    )

    rtabmap_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([pkg_share, 'launch', 'rtabmap_slam.launch.py'])
        )
    )

    return LaunchDescription([
        bridge_launch,
        rtabmap_launch,
    ])
