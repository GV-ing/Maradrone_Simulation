from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

# Bridges the Gazebo sensors of the x500_depth model (spawned by PX4's native
# `PX4_GZ_WORLD=leonardo_race_field make px4_sitl gz_x500_depth`, see the
# README) into ROS 2, for consumption by rtabmap_slam.launch.py.
#
# The RGB image/camera_info topic names below match the ones already
# documented in the README for x500_depth's IMX214 camera. The depth
# image/camera_info topic names are a BEST-GUESS default based on the
# StereoOV7251 depth sensor defined in PX4-gazebo-models' OakD-Lite model
# (which declares an explicit <topic>depth_camera</topic>, overriding Gazebo's
# scoped default naming) — the exact name can vary with the PX4-Autopilot
# version actually cloned into the Docker image. If SLAM finds no depth data,
# run `gz topic -l` while the simulation is running (after `make px4_sitl
# gz_x500_depth`) and override depth_topic_gz / depth_camera_info_topic_gz
# accordingly, e.g.:
#   ros2 launch maradrone_slam x500_depth_bridge.launch.py depth_topic_gz:=/actual/topic


def generate_launch_description():
    world_name = LaunchConfiguration('world_name')
    model_name = LaunchConfiguration('model_name')
    rgb_topic_gz = LaunchConfiguration('rgb_topic_gz')
    rgb_camera_info_topic_gz = LaunchConfiguration('rgb_camera_info_topic_gz')
    depth_topic_gz = LaunchConfiguration('depth_topic_gz')
    depth_camera_info_topic_gz = LaunchConfiguration('depth_camera_info_topic_gz')

    declare_world_name = DeclareLaunchArgument('world_name', default_value='leonardo_race_field')
    declare_model_name = DeclareLaunchArgument('model_name', default_value='x500_depth_0')
    declare_rgb_topic_gz = DeclareLaunchArgument(
        'rgb_topic_gz',
        default_value=['/world/', world_name, '/model/', model_name,
                        '/link/camera_link/sensor/IMX214/image'])
    declare_rgb_camera_info_topic_gz = DeclareLaunchArgument(
        'rgb_camera_info_topic_gz',
        default_value=['/world/', world_name, '/model/', model_name,
                        '/link/camera_link/sensor/IMX214/camera_info'])
    declare_depth_topic_gz = DeclareLaunchArgument(
        'depth_topic_gz',
        default_value='/depth_camera',
        description='BEST GUESS default — verify with `gz topic -l` and override if needed.')
    declare_depth_camera_info_topic_gz = DeclareLaunchArgument(
        'depth_camera_info_topic_gz',
        default_value='/depth_camera/camera_info',
        description='BEST GUESS default — verify with `gz topic -l` and override if needed.')

    bridge_node = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        output='screen',
        arguments=[
            # Required so use_sim_time:=true works downstream (rgbd_odometry/rtabmap).
            '/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock',
            [rgb_topic_gz, '@sensor_msgs/msg/Image[gz.msgs.Image'],
            [rgb_camera_info_topic_gz, '@sensor_msgs/msg/CameraInfo[gz.msgs.CameraInfo'],
            [depth_topic_gz, '@sensor_msgs/msg/Image[gz.msgs.Image'],
            [depth_camera_info_topic_gz, '@sensor_msgs/msg/CameraInfo[gz.msgs.CameraInfo'],
        ],
        remappings=[
            (rgb_topic_gz, '/camera/rgb/image_raw'),
            (rgb_camera_info_topic_gz, '/camera/rgb/camera_info'),
            (depth_topic_gz, '/camera/depth/image_raw'),
            (depth_camera_info_topic_gz, '/camera/depth/camera_info'),
        ],
        parameters=[{'use_sim_time': True}],
    )

    return LaunchDescription([
        declare_world_name,
        declare_model_name,
        declare_rgb_topic_gz,
        declare_rgb_camera_info_topic_gz,
        declare_depth_topic_gz,
        declare_depth_camera_info_topic_gz,
        bridge_node,
    ])
