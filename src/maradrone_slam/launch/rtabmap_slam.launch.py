from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, PythonExpression
from launch_ros.substitutions import FindPackageShare

# Runs RGB-D SLAM (RTAB-Map) on the camera streams bridged by
# x500_depth_bridge.launch.py. Uses rtabmap_ros's own visual odometry
# (rgbd_odometry, started internally by rtabmap_launch via
# visual_odometry:=true) instead of any external/ground-truth odometry
# source, since maradrone_description's URDF does not define a camera frame
# tied to base_link. frame_id is therefore set to camera_link directly, so
# rgbd_odometry publishes a self-contained odom -> camera_link TF chain.
#
# Requires the ros-humble-rtabmap-ros apt package (added to docker_scripts/Dockerfile).


def generate_launch_description():
    declare_rtabmap_viz = DeclareLaunchArgument(
        'rtabmap_viz', default_value='true',
        description='Launch the rtabmap_viz visualization window.')
    declare_delete_db = DeclareLaunchArgument(
        'delete_db_on_start', default_value='true',
        description='Delete the previous RTAB-Map database on startup (-d flag).')

    rtabmap_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([FindPackageShare('rtabmap_launch'), 'launch', 'rtabmap.launch.py'])
        ),
        launch_arguments={
            'rgb_topic': '/camera/rgb/image_raw',
            'depth_topic': '/camera/depth/image_raw',
            'camera_info_topic': '/camera/rgb/camera_info',
            'frame_id': 'camera_link',
            'approx_sync': 'true',
            'qos': '2',
            'use_sim_time': 'true',
            'visual_odometry': 'true',
            'rtabmap_viz': LaunchConfiguration('rtabmap_viz'),
            'args': PythonExpression([
                "'-d' if '", LaunchConfiguration('delete_db_on_start'), "' == 'true' else ''"
            ]),
        }.items(),
    )

    return LaunchDescription([
        declare_rtabmap_viz,
        declare_delete_db,
        rtabmap_launch,
    ])
