from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, SetEnvironmentVariable
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, Command
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from ament_index_python.packages import get_package_share_directory
from launch_ros.substitutions import FindPackageShare
from launch.actions import ExecuteProcess
import os

def generate_launch_description():
    # Percorsi dei pacchetti
    pkg_maradrone_desc = get_package_share_directory('maradrone_description')
    
    # Parametri e file
    # Usiamo lo Xacro per ROS (TF) e l'SDF per Gazebo (Fisica PX4)
    set_gz_model_name = SetEnvironmentVariable(name='PX4_GZ_MODEL_NAME', value='maradrone')
    set_gz_world_name = SetEnvironmentVariable(name='PX4_GZ_WORLD', value='leonardo_race_field')
    urdf_path = os.path.join(pkg_maradrone_desc, "urdf", "maradrone.urdf.xacro")
    sdf_file = os.path.join(pkg_maradrone_desc, 'models', 'x500', 'model.sdf')
    default_world_path = os.path.join(pkg_maradrone_desc, "worlds", "leonardo_race_field.sdf") # Assicurati che esista

    # 1. Variabili d'ambiente (Standard Armando)
    gz_resource_path = SetEnvironmentVariable(
        name='GZ_SIM_RESOURCE_PATH',
        value=os.path.join(pkg_maradrone_desc, 'models') + ':' + 
              os.path.join(pkg_maradrone_desc, '..')
    )

    # 2. Argomenti di lancio
    declare_use_sim_time = DeclareLaunchArgument('use_sim_time', default_value='true')
    world_arg = DeclareLaunchArgument('world', default_value=default_world_path)

    # 3. Descrizione del Robot (Xacro -> robot_description)
    robot_description_content = ParameterValue(
        Command(['xacro ', urdf_path, ' package_path:=', pkg_maradrone_desc]),
        value_type=str
    )

    # Nodo: robot_state_publisher (Necessario per TF e RViz)
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[{'robot_description': robot_description_content, 'use_sim_time': True}],
    )

    # 4. Simulatore Gazebo
    gazebo_launch = ExecuteProcess(
        cmd=['gz', 'sim', '-r', LaunchConfiguration('world')],
        output='screen'
    )

    # 5. Nodo di Spawn (Carica il modello SDF nativo di PX4)
    spawn_robot_node = Node(
        package='ros_gz_sim',
        executable='create',
        arguments=[
            '-file', sdf_file,
            '-name', 'maradrone',
            '-z', '0.2',
            '-allow_renaming', 'true'
        ],
        output='screen',
    )

    # 6. Bridge ROS <-> Gazebo (Fondamentale per il clock e la telemetria)
    bridge_node = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        arguments=[
            '/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock',
            '/model/maradrone/pose@tf2_msgs/msg/TFMessage[gz.msgs.Pose_V',        
            '/world/leonardo_race_field/pose/info@tf2_msgs/msg/TFMessage[gz.msgs.Pose_V', # Cambia 'default' col nome del tuo mondo
        ],
        output='screen',
        parameters=[{'use_sim_time': True}],
    )

    return LaunchDescription([
        set_gz_model_name,
        set_gz_world_name,
        gz_resource_path,
        declare_use_sim_time,
        world_arg,
        robot_state_publisher_node,
        gazebo_launch,
        spawn_robot_node,
        bridge_node
    ])