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
* [Architecture and PX4 Communication](#-architecture-and-px4-communication)
* [Repository Structure](#-repository-structure)
* [Final Notes](#-final-notes)

---

## 🔎 Overview

**Maradrone Simulation** provides a Dockerized environment for running a PX4-based drone simulation integrated with ROS 2 and Gazebo.

The project combines the PX4 flight stack with custom ROS 2 nodes, Gazebo sensors, QGroundControl, and a custom simulation environment.

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

* PX4 Autopilot SITL with the `x500_depth` drone model
* Custom `leonardo_race_field` simulation world
* PX4–ROS 2 communication through `px4_msgs`
* `ros_gz_bridge` built for Gazebo Harmonic
* IMX214 camera bridged to ROS 2
* GStreamer and UDP video streaming for QGroundControl
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

Inside the container, build the ROS 2 workspace:

```bash
cd /root/ros2_ws
colcon build --packages-select px4_msgs maradrone_framework offboard_rl force_land read_rpy
```

Then source the workspace:

```bash
source install/setup.bash
```

### Custom Nodes

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
             ▼                  │  IMX214 camera           │
┌─────────────────────────┐      └────────────┬────────────┘
│          ROS 2          │                   │
│                         │◄──────────────────┘
│ px4_msgs                │      ros_gz_bridge
│ Custom ROS 2 nodes      │
└─────────────────────────┘
```

### Communication Flow

1. **PX4-Autopilot** runs the SITL flight-control stack.
2. **Gazebo Harmonic** simulates the drone and the custom environment.
3. **`ros_gz_bridge`** exposes Gazebo sensor data to ROS 2.
4. **`px4_msgs`** provides ROS 2 message definitions corresponding to PX4 uORB messages.
5. Custom ROS 2 nodes publish commands to `/fmu/in/...`.
6. PX4 publishes vehicle state through `/fmu/out/...`.
7. **QGroundControl** communicates with PX4 through MAVLink on UDP port `14550`.
8. Camera data can be streamed through UDP port `5600` for QGroundControl.

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
* ROS 2 is used for high-level control, perception, and custom application logic.

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
* The `maradrone_framework`, `offboard_rl`, `force_land`, and `read_rpy` packages depend on `px4_msgs` and the PX4–ROS 2 communication layer.
* PX4 remains the central flight-control component, while ROS 2 provides the interface for custom control and application-level logic.
