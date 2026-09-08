# Maradrone Simulation 🚁

[![ROS 2](https://img.shields.io/badge/ROS_2-Humble-3498db.svg)](https://docs.ros.org/en/humble/)
[![Gazebo](https://img.shields.io/badge/Gazebo-Harmonic-orange.svg)](https://gazebosim.org/home)
[![PX4](https://img.shields.io/badge/PX4-Autopilot-blue.svg)](https://px4.io/)
[![Docker](https://img.shields.io/badge/Docker-Enabled-2496ed.svg)](https://www.docker.com/)

A fully Dockerized simulation environment for **Maradrone**, integrating:

* **PX4 Autopilot SITL**
* **ROS 2 Humble**
* **Gazebo Harmonic**
* **QGroundControl**
* A custom `leonardo_race_field` simulation world
* **RTAB-Map RGB-D SLAM**
* A configurable multi-waypoint mission executor

---

## 📑 Table of Contents

* [Overview](#-overview)
* [Why Is This Repository Different?](#-why-is-this-repository-different)
* [Features](#-features)
* [Prerequisites](#-prerequisites)
* [Installation](#-installation)
* [Usage](#-usage)

  * [Building and Starting the Docker Container](#building-and-starting-the-docker-container)
  * [Starting PX4 and Loading the Custom World](#starting-px4-and-loading-the-custom-world)
  * [Downloading and Starting QGroundControl](#downloading-and-starting-qgroundcontrol)
  * [Starting the Gazebo–ROS 2 Bridge](#starting-the-gazebo-ros-2-bridge)
  * [Micro-XRCE-DDS-Agent](#micro-xrce-dds-agent)
  * [Building and Running Custom ROS 2 Nodes](#building-and-running-custom-ros-2-nodes)
  * [Node Parameters](#node-parameters)
  * [Waypoint Mission](#waypoint-mission)
  * [SLAM (RTAB-Map RGB-D)](#slam-rtab-map-rgb-d)
  * [Visualizing the Robot in RViz](#visualizing-the-robot-in-rviz)
* [Architecture and PX4 Communication](#-architecture-and-px4-communication)
* [Repository Structure](#-repository-structure)
* [Final Notes](#-final-notes)

---

## 🔎 Overview

**Maradrone Simulation** provides a Dockerized environment for running a PX4-based drone simulation integrated with ROS 2 and Gazebo.

The project combines the PX4 flight stack with custom ROS 2 nodes, Gazebo sensors, QGroundControl, RGB-D SLAM, and a custom simulation environment.

Unlike the other repositories developed for the `Armando-Simulation` and `Fra2mo-Simulation` educational framework, this project uses **PX4 Autopilot SITL as the central flight-control component**.

---

## 📌 Why Is This Repository Different?

This repository has a different architecture compared to the other two repositories defined for the `Armando-Simulation` and `Fra2mo-Simulation` educational framework.

### Key Differences

* `Armando-Simulation` and `Fra2mo-Simulation` are fully based on ROS 2 and share a similar architecture.
* This repository integrates a complete **PX4 Autopilot SITL installation**.
* PX4 is maintained outside the ROS 2 workspace.
* During the Docker image build:

  * `PX4-Autopilot` is cloned into `/root/PX4-Autopilot`.
  * The PX4 source tree remains external to the ROS 2 workspace.
  * The custom `leonardo_race_field` world is copied directly into the PX4 sources.
  * Custom models are copied into `/root/PX4-Autopilot/Tools/simulation/gz/models/`.
* The overall architecture and development workflow are therefore different from the two pure ROS 2 projects.

### Container Persistence

The container is **not** started with the `--rm` option.

When the shell is exited, the container is stopped but **not removed**. This makes it possible to preserve changes made inside the container, which is particularly useful when modifying PX4 source code or creating and testing new configurations.

---

## ✨ Features

* PX4 Autopilot SITL with the `x500` and `x500_depth` drone models
* Custom `leonardo_race_field` simulation world
* PX4–ROS 2 communication through `px4_msgs`
* `ros_gz_bridge` built for Gazebo Harmonic
* IMX214 camera bridged to ROS 2
* GStreamer and UDP video streaming for QGroundControl
* `maradrone_utils`: shared attitude-conversion and quintic-trajectory utilities, reused across all custom nodes
* Configurable ROS 2 parameters for takeoff altitude, forced-landing altitude threshold, and trajectory publish rate
* `maradrone_mission`: configurable multi-waypoint mission executor
* `maradrone_slam`: RGB-D SLAM with RTAB-Map, built on the `x500_depth` depth camera
* Fully Dockerized development and simulation environment

---

## 🛠 Prerequisites

The following components are required on the host machine:

* Docker
* QGroundControl
* Docker permissions without `sudo` (recommended)

To add the current user to the Docker group:

```bash
sudo usermod -aG docker $USER
```

After adding the user to the Docker group, log out and log back in for the changes to take effect.

---

## 📥 Installation

### Building and Starting the Docker Container

Clone the repository and enter its root directory:

```bash
cd /path/to/Maradrone_Simulation
```

Build the Docker image:

```bash
./docker_scripts/docker_build_image.sh
```

Then start the container:

```bash
./docker_scripts/docker_run_container.sh
```

The first command builds the Docker image and prepares:

* ROS 2 Humble
* Gazebo Harmonic
* `ros_gz_bridge`
* Micro-XRCE-DDS-Agent
* The `PX4-Autopilot` source tree
* The custom world and models inside PX4
* `rtabmap_ros` (RGB-D SLAM), `navigation2`, and `robot_localization`

> **Note:** If you built the image before this SLAM feature was added, rebuild it (`./docker_scripts/docker_build_image.sh`) to pick up `ros-humble-rtabmap-ros`.

### Starting an Existing Container

If the container already exists but is stopped, the same run script can be used to start it again and enter the container:

```bash
./docker_scripts/docker_run_container.sh
```

Because the container is not automatically removed, changes made inside it are preserved between runs.

---

## 🚀 Usage

### Starting PX4 and Loading the Custom World

Once inside the container, navigate to the PX4 directory:

```bash
cd /root/PX4-Autopilot
```

Start PX4 SITL with Gazebo Harmonic:

```bash
PX4_GZ_WORLD=leonardo_race_field make px4_sitl gz_x500_depth
```

This command starts:

* PX4 SITL
* Gazebo Harmonic
* The custom `leonardo_race_field` world
* The `x500_depth` drone model

---

### 🛰 Downloading and Starting QGroundControl

Download QGroundControl from the official website:

[QGroundControl Download & Install](https://docs.qgroundcontrol.com/master/en/getting_started/download_and_install.html)

On Linux, if using the AppImage:

```bash
cd ~/Downloads
chmod +x QGroundControl.AppImage
./QGroundControl.AppImage
```

QGroundControl should connect to the PX4 simulation through MAVLink on:

```text
14550
```

For the UDP video stream, configure QGroundControl to use:

```text
5600
```

---

### 🔌 Starting the Gazebo–ROS 2 Bridge

From inside the container, run:

```bash
docker exec -it maradrone_container bash
```

Then start the Gazebo–ROS 2 bridge:

```bash
ros2 run ros_gz_bridge parameter_bridge \
/world/leonardo_race_field/model/x500_depth_0/link/camera_link/sensor/IMX214/image@sensor_msgs/msg/Image[gz.msgs.Image
```

The IMX214 camera image is then available in ROS 2 on:

```text
/world/leonardo_race_field/model/x500_depth_0/link/camera_link/sensor/IMX214/image
```

You can visualize the image using `rqt_image_view`:

```bash
rqt_image_view
```

Select the camera topic above from the topic list.

> For RGB-D **SLAM** (RGB + depth + camera_info, remapped to clean topic names, plus `/clock`), use the `maradrone_slam` package instead of this manual bridge command — see [SLAM (RTAB-Map RGB-D)](#slam-rtab-map-rgb-d) below.

---

### ⚙️ Micro-XRCE-DDS-Agent

The Docker image automatically installs the components required for PX4–ROS 2 communication:

* `Micro-XRCE-DDS-Agent`
* `ros_gz_bridge`

The **Micro-XRCE-DDS-Agent** acts as the communication bridge between PX4 and ROS 2:

```text
PX4
 │
 │ uXRCE-DDS
 ▼
Micro-XRCE-DDS-Agent
 │
 │ DDS
 ▼
ROS 2
```

More specifically:

* PX4 runs the `uxrce_dds_client`.
* The container provides the `Micro-XRCE-DDS-Agent`.
* `px4_msgs` exposes PX4 message definitions to ROS 2.

In this project, the Micro-XRCE-DDS-Agent is already configured as part of the environment and does **not require a separate manual startup**.

---

### 🧩 Building and Running Custom ROS 2 Nodes

Inside the container, build the ROS 2 workspace. `maradrone_utils` is a header-only dependency of several other packages, so it (or `--packages-up-to`) should be built first:

```bash
cd /root/ros2_ws
colcon build --packages-select px4_msgs maradrone_utils maradrone_framework offboard_rl force_land read_rpy maradrone_mission maradrone_slam
```

Then source the workspace:

```bash
source install/setup.bash
```

### Custom Packages and Nodes

#### `maradrone_utils`

Header-only library, not an executable. Provides:

* `maradrone_utils/attitude_utils.h`: quaternion → roll/pitch/yaw conversion (`quatToRpy`) and shortest-path angle error (`angleError`).
* `maradrone_utils/quintic_trajectory.h`: the `QuinticTrajectory` class used by `go_to_point` and `waypoint_mission` to generate a smooth position/velocity/acceleration profile (zero velocity/acceleration at both endpoints) between two 4D (x, y, z, yaw) setpoints.

#### `maradrone_framework`

Executable:

```text
offboard_takeoff
```

Publishes:

```text
/fmu/in/offboard_control_mode
/fmu/in/trajectory_setpoint
/fmu/in/vehicle_command
```

Arms the vehicle and climbs to a configurable takeoff altitude (see [Node Parameters](#node-parameters)).

---

#### `offboard_rl`

Executable:

```text
go_to_point
```

Subscribes to:

```text
/fmu/out/vehicle_local_position
/fmu/out/vehicle_attitude
```

Publishes:

```text
/fmu/in/offboard_control_mode
/fmu/in/trajectory_setpoint
/fmu/in/vehicle_command
```

Reads a single `x y z yaw T` setpoint from the terminal (meters, meters, meters, radians, seconds) and flies to it along a quintic trajectory generated by `maradrone_utils::QuinticTrajectory`.

---

#### `force_land`

Executable:

```text
force_land
```

Publishes an emergency landing command through:

```text
/fmu/in/vehicle_command
```

Forces `VEHICLE_CMD_NAV_LAND` when the vehicle exceeds a configurable altitude threshold (see [Node Parameters](#node-parameters)).

---

#### `read_rpy`

Executable:

```text
read_rpy
```

Reads the vehicle attitude from:

```text
/fmu/out/vehicle_attitude
```

Publishes roll/pitch/yaw (`geometry_msgs/Vector3`) on `/out/rpy_info`.

---

#### `maradrone_mission`

Executable:

```text
waypoint_mission
```

Flies a configurable sequence of waypoints in PX4 offboard mode, one quintic trajectory leg at a time. See [Waypoint Mission](#waypoint-mission).

---

#### `maradrone_slam`

Launch-only package (no executables): bridges the `x500_depth` RGB-D camera into ROS 2 and runs RTAB-Map RGB-D SLAM. See [SLAM (RTAB-Map RGB-D)](#slam-rtab-map-rgb-d).

### Running the Nodes

The custom nodes can be started with:

```bash
ros2 run maradrone_framework offboard_takeoff
ros2 run offboard_rl go_to_point
ros2 run force_land force_land
ros2 run read_rpy read_rpy
```

> **Note:** These nodes require `px4_msgs` to be built and the PX4–ROS 2 communication layer to be active.

---

### Node Parameters

The nodes below expose ROS 2 parameters (with the same defaults as before) instead of hardcoded values. Any parameter can be overridden from the command line with `--ros-args -p <name>:=<value>`, or from a launch file's `parameters=[...]`.

| Package | Node | Parameter | Default | Meaning |
|---|---|---|---|---|
| `maradrone_framework` | `offboard_takeoff` | `takeoff_altitude` | `5.0` | Altitude (m, positive up) to climb to after arming. |
| `force_land` | `force_land` | `max_altitude` | `20.0` | Altitude (m) above which a forced `NAV_LAND` is triggered. |
| `offboard_rl` | `go_to_point` | `trajectory_rate_hz` | `50.0` | Rate (Hz) at which trajectory setpoints are sampled and published. |
| `maradrone_mission` | `waypoint_mission` | `trajectory_rate_hz` | `50.0` | Same as above, for the mission executor. |

Examples:

```bash
ros2 run maradrone_framework offboard_takeoff --ros-args -p takeoff_altitude:=8.0
ros2 run force_land force_land --ros-args -p max_altitude:=25.0
ros2 run offboard_rl go_to_point --ros-args -p trajectory_rate_hz:=100.0
```

---

### Waypoint Mission

`maradrone_mission` flies a fixed sequence of waypoints in PX4 offboard mode, reusing the same quintic-trajectory generator as `go_to_point`. Each waypoint has a target `x, y, z, yaw` (z in the PX4 **NED** frame, i.e. negative = above ground), a `duration` (seconds to fly the trajectory leg into it), and a `hold_time` (seconds to hover there before continuing).

Waypoints are declared as ROS 2 parameters in a YAML file, `src/maradrone_mission/config/waypoints.yaml`:

```yaml
waypoint_mission:
  ros__parameters:
    trajectory_rate_hz: 50.0
    waypoints:
      x:         [0.0, 5.0, 5.0, 0.0, 0.0]
      y:         [0.0, 0.0, 5.0, 5.0, 0.0]
      z:         [-5.0, -5.0, -5.0, -5.0, -5.0]
      yaw:       [0.0, 1.5708, 3.1416, -1.5708, 0.0]
      duration:  [8.0, 6.0, 6.0, 6.0, 6.0]
      hold_time: [2.0, 2.0, 2.0, 2.0, 2.0]
```

All six `waypoints.*` arrays must have the same length. Edit this file (or copy it and pass your own path) to define a different mission.

With PX4 SITL + Gazebo already running (see [Starting PX4 and Loading the Custom World](#starting-px4-and-loading-the-custom-world), or the [ROS 2 Gazebo/PX4 launch workflow](#ros-2-gazebopx4-launch-workflow)), build and run the mission:

```bash
colcon build --packages-select maradrone_utils maradrone_mission
source install/setup.bash
ros2 launch maradrone_mission mission.launch.py
```

To use a different waypoint file:

```bash
ros2 launch maradrone_mission mission.launch.py waypoints_file:=/path/to/my_waypoints.yaml
```

---

### SLAM (RTAB-Map RGB-D)

`maradrone_slam` runs RGB-D SLAM with [RTAB-Map](http://introlab.github.io/rtabmap/) on the `x500_depth` model's IMX214 (RGB) and depth cameras, using RTAB-Map's own visual odometry (`rgbd_odometry`) — no lidar or external odometry source is required.

**Prerequisites:**

* The Docker image must include `ros-humble-rtabmap-ros` (rebuild the image if it predates this feature — see [Installation](#-installation)).
* PX4 SITL + Gazebo must be running with the **`x500_depth`** model (which carries the depth camera; the plain `x500` model used elsewhere in this repo has no camera).

1. Start PX4 SITL with the depth-camera model:

   ```bash
   cd /root/PX4-Autopilot
   PX4_GZ_WORLD=leonardo_race_field make px4_sitl gz_x500_depth
   ```

2. In another terminal inside the container, build and source the workspace:

   ```bash
   cd /root/ros2_ws
   colcon build --packages-select maradrone_slam
   source install/setup.bash
   ```

3. Launch the camera bridge and RTAB-Map together:

   ```bash
   ros2 launch maradrone_slam slam.launch.py
   ```

   This launches, in order:
   * `x500_depth_bridge.launch.py` — bridges `/clock`, the RGB image/camera_info, and the depth image/camera_info from Gazebo into ROS 2 (remapped to `/camera/rgb/...` and `/camera/depth/...`).
   * `rtabmap_slam.launch.py` — first crops the 1920x1080 RGB image to a centered 1440x1080 (4:3) region with `image_proc`'s `CropDecimateNode`, to match the depth camera's 640x480 (4:3) aspect ratio (`rgbd_odometry` requires the two to share an aspect ratio, and IMX214/depth don't out of the box). Then includes `rtabmap_launch`'s `rtabmap.launch.py` with `visual_odometry:=true`, `use_sim_time:=true`, `frame_id:=camera_link`, building a live occupancy/point-cloud map and localizing the camera within it. `rtabmap_viz` opens by default for visualization.

4. **Verify the depth camera topic.** The default depth image/camera_info topic names (`/depth_camera`, `/camera_info` — note the camera_info topic is unscoped, not `/depth_camera/camera_info`) were verified against real `gz topic -l` output, but can still vary with the exact `PX4-Autopilot` version cloned into the image. If RTAB-Map reports no depth data, list the actual Gazebo topics while the simulation is running:

   ```bash
   gz topic -l | grep -i camera
   ```

   and override the bridge with the real topic name, e.g.:

   ```bash
   ros2 launch maradrone_slam slam.launch.py depth_topic_gz:=/actual/depth/topic depth_camera_info_topic_gz:=/actual/depth/camera_info/topic
   ```

   You can run just the bridge on its own the same way (`ros2 launch maradrone_slam x500_depth_bridge.launch.py ...`) to inspect the remapped ROS topics (`ros2 topic hz /camera/depth/image_raw`) before starting RTAB-Map.

5. To fly the drone while mapping, run `go_to_point` or `waypoint_mission` (see above) in another terminal — SLAM does not arm or control the vehicle itself.

**Troubleshooting:** if `rtabmap`/`rgbd_odometry`/`rtabmap_viz` executables are not found after installing `ros-humble-rtabmap-ros`, check `apt list --installed | grep rtabmap` and try `apt-get update && apt-get upgrade` inside the container — some historical Ubuntu Jammy/Humble APT snapshots shipped incomplete `rtabmap_ros` binaries.

---

### Visualizing the Robot in RViz

To visualize the robot description (URDF) and TF tree in RViz:

```bash
ros2 launch maradrone_description maradrone_rviz.launch.py
```

---

## 📘 Architecture and PX4 Communication

### System Overview

The overall communication architecture can be summarized as follows:

```text
                    ┌──────────────────┐
                    │  QGroundControl  │
                    └────────┬─────────┘
                             │ MAVLink
                           UDP 14550
                             │
                             ▼
┌───────────────────────────────────────────────────────────┐
│                         PX4 SITL                          │
│                                                           │
│  PX4-Autopilot                                            │
│  ├── uORB                                                 │
│  └── uxrce_dds_client                                     │
└───────────────┬───────────────────────────┬───────────────┘
                │                           │
                │ uXRCE-DDS                 │ Gazebo
                ▼                           ▼
┌─────────────────────────┐      ┌─────────────────────────┐
│ Micro-XRCE-DDS-Agent    │      │    Gazebo Harmonic      │
└────────────┬────────────┘      │  leonardo_race_field    │
             │ DDS              │  x500_depth              │
             ▼                  │  IMX214 + depth camera   │
┌─────────────────────────┐      └────────────┬────────────┘
│          ROS 2          │                   │
│                         │◄──────────────────┘
│ px4_msgs                │      ros_gz_bridge
│ Custom ROS 2 nodes      │
│ maradrone_utils         │
│ maradrone_mission       │
│ maradrone_slam          │
│  └── RTAB-Map RGB-D SLAM│
└─────────────────────────┘
```

### Communication Flow

1. **PX4-Autopilot** runs the SITL flight-control stack.
2. **Gazebo Harmonic** simulates the drone and the custom environment.
3. **`ros_gz_bridge`** exposes Gazebo sensor data to ROS 2.
4. **`px4_msgs`** provides ROS 2 message definitions corresponding to PX4 uORB messages.
5. Custom ROS 2 nodes (`offboard_takeoff`, `go_to_point`, `waypoint_mission`) publish commands to `/fmu/in/...`.
6. PX4 publishes vehicle state through `/fmu/out/...`.
7. **QGroundControl** communicates with PX4 through MAVLink on UDP port `14550`.
8. Camera data can be streamed through UDP port `5600` for QGroundControl, or bridged into ROS 2 for **RTAB-Map RGB-D SLAM** (`maradrone_slam`), which builds its own visual odometry from the RGB-D stream.

### Key PX4 Topics

#### Commands sent to PX4

```text
/fmu/in/offboard_control_mode
/fmu/in/trajectory_setpoint
/fmu/in/vehicle_command
```

#### State received from PX4

```text
/fmu/out/vehicle_local_position
/fmu/out/vehicle_attitude
```

---

## 🔄 PX4 + ROS 2 vs. Pure ROS 2

This repository is **not a pure ROS 2 simulation**. It is a hybrid PX4 + ROS 2 system.

The main distinction is that PX4 remains the central flight-control component:

* `px4_msgs` provides the interface between PX4 messages and ROS 2.
* Custom ROS 2 nodes send commands directly to PX4 through `/fmu/in/...`.
* PX4 remains responsible for the vehicle's core flight-control logic.
* Gazebo provides the simulated environment and sensor data.
* ROS 2 is used for high-level control, perception (SLAM), and custom application logic.

This architecture is therefore fundamentally different from the ROS 2-only approach used by `Armando-Simulation` and `Fra2mo-Simulation`.

---

## 📁 Repository Structure

```text
Maradrone_Simulation/
├── docker_scripts/
│   ├── Dockerfile
│   ├── docker_build_image.sh
│   └── docker_run_container.sh
├── src/
│   ├── force_land/
│   ├── maradrone_description/
│   │   ├── models/
│   │   └── worlds/
│   ├── maradrone_framework/
│   ├── maradrone_mission/
│   │   └── config/
│   ├── maradrone_slam/
│   │   └── launch/
│   ├── maradrone_utils/
│   │   └── include/
│   ├── offboard_rl/
│   ├── px4_msgs/
│   └── read_rpy/
└── README.md
```

---

## 💡 Final Notes

* This repository follows a different development methodology and architecture compared to `Armando-Simulation` and `Fra2mo-Simulation`.
* The Dockerfile prepares the PX4 source tree and copies the custom simulation world into `/root/PX4-Autopilot`.
* Custom Gazebo models are installed under `/root/PX4-Autopilot/Tools/simulation/gz/models/`.
* The Docker container is persistent and is not removed when stopped, allowing modifications to be preserved.
* The `maradrone_framework`, `offboard_rl`, `force_land`, `read_rpy`, and `maradrone_mission` packages depend on `px4_msgs` and the PX4–ROS 2 communication layer; `offboard_rl`, `read_rpy`, and `maradrone_mission` also depend on `maradrone_utils`.
* `maradrone_slam` depends on `rtabmap_ros` (installed via the Dockerfile) and on the `x500_depth` model's cameras, which are only present in PX4-Autopilot's own `x500_depth` Gazebo model (not vendored in this repository).
* PX4 remains the central flight-control component, while ROS 2 provides the interface for custom control, mission execution, perception, and application-level logic.
