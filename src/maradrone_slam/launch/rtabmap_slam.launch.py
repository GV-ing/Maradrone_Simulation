from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, PythonExpression
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
from launch_ros.substitutions import FindPackageShare

# Runs RGB-D SLAM (RTAB-Map) on the camera streams bridged by
# x500_depth_bridge.launch.py. Uses rtabmap_ros's own visual odometry
# (rgbd_odometry, started internally by rtabmap_launch via
# visual_odometry:=true) instead of any external/ground-truth odometry
# source, since maradrone_description's URDF does not define a camera frame
# tied to base_link. frame_id is therefore set to camera_link directly, so
# rgbd_odometry publishes a self-contained odom -> camera_link TF chain.
#
# rgbd_odometry requires the RGB and depth images to share the same aspect
# ratio (it only tolerates a resolution difference that is a pure scale
# factor). x500_depth's IMX214 RGB camera is 1920x1080 (16:9) while its
# depth camera is 640x480 (4:3) — different ratios, which makes
# rgbd_odometry abort immediately ("imageWidth/depthWidth ==
# imageHeight/depthHeight" assertion). We crop the RGB image centrally to
# 1440x1080 (4:3, matching the depth image's ratio) with image_proc's
# CropDecimateNode before handing it to RTAB-Map.
#
# Requires the ros-humble-rtabmap-ros and ros-humble-image-pipeline apt
# packages (both already in docker_scripts/Dockerfile).


def generate_launch_description():
    declare_rtabmap_viz = DeclareLaunchArgument(
        'rtabmap_viz', default_value='true',
        description='Launch the rtabmap_viz visualization window.')
    declare_delete_db = DeclareLaunchArgument(
        'delete_db_on_start', default_value='true',
        description='Delete the previous RTAB-Map database on startup (-d flag).')

    # Crops the 1920x1080 RGB image to a centered 1440x1080 (4:3) region so
    # its aspect ratio matches the 640x480 depth image.
    rgb_crop_container = ComposableNodeContainer(
        name='rgb_crop_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[
            ComposableNode(
                package='image_proc',
                plugin='image_proc::CropDecimateNode',
                name='rgb_crop',
                remappings=[
                    ('in/image_raw', '/camera/rgb/image_raw'),
                    ('in/camera_info', '/camera/rgb/camera_info'),
                    ('out/image_raw', '/camera/rgb/image_cropped'),
                    ('out/camera_info', '/camera/rgb/camera_info_cropped'),
                ],
                parameters=[{
                    'width': 1440,
                    'height': 1080,
                    'offset_x': 240,
                    'offset_y': 0,
                    'decimation_x': 1,
                    'decimation_y': 1,
                    'use_sim_time': True,
                }],
            ),
        ],
        output='screen',
    )

    rtabmap_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([FindPackageShare('rtabmap_launch'), 'launch', 'rtabmap.launch.py'])
        ),
        launch_arguments={
            'rgb_topic': '/camera/rgb/image_cropped',
            'depth_topic': '/camera/depth/image_raw',
            'camera_info_topic': '/camera/rgb/camera_info_cropped',
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
        rgb_crop_container,
        rtabmap_launch,
    ])
