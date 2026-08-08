# Maradrone Simulation 🚁

[![ROS 2](https://img.shields.io/badge/ROS_2-Humble-3498db.svg)](https://docs.ros.org/en/humble/)
[![Gazebo](https://img.shields.io/badge/Gazebo-Harmonic-orange.svg)](https://gazebosim.org/home)
[![PX4](https://img.shields.io/badge/PX4-Autopilot-blue.svg)](https://px4.io/)
[![Docker](https://img.shields.io/badge/Docker-Enabled-2496ed.svg)](https://www.docker.com/)

A fully Dockerized simulation environment for **Maradrone**, based on the PX4 Autopilot ecosystem, ROS 2 Humble, and Gazebo Harmonic.

This repository contains the setup for simulating an `x500_depth` drone inside a custom environment (`leonardo_race_field`), with full support for:

* MAVLink telemetry
* QGroundControl (QGC)
* Autonomous missions such as Survey
* Real-time video streaming
* ROS 2 image processing
* Gazebo-to-ROS communication through `ros_gz_bridge`
* GStreamer video streaming

---

## 📑 Table of Contents

* [Features](#-features)
* [Prerequisites](#-prerequisites)
* [Installation](#-installation-docker)
* [Usage](#-usage)

  * [1. Start the Simulation](#1-start-the-simulation)
  * [2. View the Camera](#2-view-the-camera-ros-2--rqt)
  * [3. QGroundControl and GStreamer](#3-qgroundcontrol--gstreamer)
* [Repository Structure](#-repository-structure)

---

## ✨ Features

### 🌍 Custom Environment

Includes the `leonardo_race_field` custom Gazebo world and its 3D models, automatically integrated into the PX4 Gazebo environment.

### 📷 Camera Streaming

The `x500_depth` drone is equipped with an IMX214 camera.

The Gazebo camera topic can be bridged to ROS 2 using a source-compiled `ros_gz_bridge`, providing a standard:

```text
sensor_msgs/msg/Image
```

topic for use with ROS 2, OpenCV, computer vision nodes, and `rqt_image_view`.

### 🎮 QGroundControl Integration

The simulation supports QGroundControl for:

* MAVLink telemetry
* Drone control
* Waypoint missions
* Survey missions
* Live video streaming

### 🎥 GStreamer Video Streaming

The camera stream can be forwarded to QGroundControl using GStreamer over UDP.

### 🐳 Fully Dockerized

The complete simulation environment runs inside Docker.

There is no need to install ROS 2, Gazebo, PX4, or their dependencies directly on the host machine.

---

## 🛠 Prerequisites

Only the following software is required on the host machine:

* [Docker](https://docs.docker.com/engine/install/)
* [QGroundControl](https://docs.qgroundcontrol.com/master/en/getting_started/download_and_install.html)

Make sure your user has permission to run Docker without `sudo`.

For example:

```bash
sudo usermod -aG docker $USER
```

After adding the user to the Docker group, log out and log back in for the change to take effect.

---

## 📥 Installation (Docker)

### 1. Clone the repository

Clone the repository to your local machine:

```bash
git clone https://github.com/GV-ing/Maradrone_Simulation.git
cd Maradrone_Simulation
```

### 2. Build the Docker image

Run the provided Docker build script:

```bash
./docker_scripts/docker_build_image.sh
```

The build process will:

1. Pull the required PX4 environment.
2. Install ROS 2 Humble.
3. Install Gazebo Harmonic.
4. Compile `ros_gz_bridge` from source.
5. Copy the custom models and worlds.
6. Configure the environment for the Maradrone simulation.

> **Note:** The first build may take several minutes because `ros_gz_bridge` is compiled from source for Gazebo Harmonic compatibility.

---

# 🚀 Usage

The complete setup uses three terminals.

---

## 1. Start the Simulation

Open the **first terminal**.

Navigate to the repository:

```bash
cd ~/Maradrone_Simulation
```

Start the Docker container:

```bash
./docker_scripts/docker_run_container.sh
```

Once inside the container, navigate to PX4:

```bash
cd ~/PX4-Autopilot
```

Start the PX4 SITL simulation using the custom world and `x500_depth` drone:

```bash
PX4_GZ_WORLD=leonardo_race_field make px4_sitl gz_x500_depth
```

Gazebo should now start with the `leonardo_race_field` environment and the `x500_depth` drone.

---

## 2. View the Camera (ROS 2 / RQT)

Open a **second terminal**.

Attach to the running Docker container:

```bash
docker exec -it maradrone_container bash
```

Start the Gazebo-to-ROS 2 bridge for the IMX214 camera:

```bash
ros2 run ros_gz_bridge parameter_bridge /world/leonardo_race_field/model/x500_depth_0/link/camera_link/sensor/IMX214/image@sensor_msgs/msg/Image[gz.msgs.Image
```

The Gazebo camera stream is now available through ROS 2.

### Open `rqt_image_view`

Open a **third terminal**:

```bash
docker exec -it maradrone_container bash
```

Then run:

```bash
rqt_image_view
```

In `rqt_image_view`, select the following topic:

```text
/world/leonardo_race_field/model/x500_depth_0/link/camera_link/sensor/IMX214/image
```

You should now see the live camera feed from the simulated drone.

---

## 3. QGroundControl & GStreamer

Launch **QGroundControl** on the host machine.

The PX4 simulation should automatically establish a MAVLink connection through UDP port:

```text
14550
```

### Configure the video stream

In QGroundControl:

1. Open **Application Settings**.
2. Select **Video**.
3. Set **Video Source** to:

   * `UDP h.264 Video Stream`
   * or `GStreamer`, depending on the QGC version.
4. Set the UDP port to:

```text
5600
```

5. Return to the **Fly** view.

The live camera stream should now be displayed in QGroundControl.

### Autonomous Survey Mission

Once the drone is connected:

1. Open the **Plan** view.
2. Create a **Survey** mission.
3. Define the desired survey area.
4. Upload the mission to the simulated drone.
5. Switch to the **Fly** view.
6. Start the mission.

The simulated drone can then execute the autonomous survey while providing live camera telemetry.

---

# 📁 Repository Structure

```text
Maradrone_Simulation/
├── docker_scripts/
│   ├── Dockerfile
│   │   └── Complete environment setup
│   │
│   ├── docker_build_image.sh
│   │   └── Script used to build the Docker image
│   │
│   └── docker_run_container.sh
│       └── Script used to launch the container
│
├── src/
│   └── maradrone_description/
│       ├── models/
│       │   └── leo_race_field/
│       │       └── 3D meshes, SDF and configuration files
│       │
│       └── worlds/
│           └── leonardo_race_field.sdf
│               └── Gazebo Harmonic world definition
│
└── README.md
```

---

## 🔧 Main Components

| Component         | Version / Technology |
| ----------------- | -------------------- |
| ROS 2             | Humble               |
| Gazebo            | Harmonic             |
| PX4               | Autopilot / SITL     |
| Drone             | `x500_depth`         |
| Camera            | IMX214               |
| Communication     | MAVLink              |
| ROS-Gazebo Bridge | `ros_gz_bridge`      |
| Video             | GStreamer            |
| Ground Control    | QGroundControl       |
| Containerization  | Docker               |

---

## 🧩 Communication Overview

The main data flow of the simulation is:

```text
                    ┌──────────────────────┐
                    │     PX4 SITL        │
                    │     x500_depth      │
                    └──────────┬───────────┘
                               │
                               ▼
                    ┌──────────────────────┐
                    │       Gazebo         │
                    │ leonardo_race_field  │
                    └──────────┬───────────┘
                               │
                 ┌─────────────┴─────────────┐
                 │                           │
                 ▼                           ▼
        ┌─────────────────┐         ┌─────────────────┐
        │ ros_gz_bridge   │         │     MAVLink     │
        └────────┬────────┘         └────────┬────────┘
                 │                           │
                 ▼                           ▼
        ┌─────────────────┐         ┌─────────────────┐
        │     ROS 2       │         │ QGroundControl  │
        │ sensor_msgs     │         │      QGC        │
        └────────┬────────┘         └─────────────────┘
                 │
                 ▼
        ┌─────────────────┐
        │ rqt_image_view  │
        │ / OpenCV / CV   │
        └─────────────────┘
```

---

## 📡 Important Ports

|    Port | Purpose                  |
| ------: | ------------------------ |
| `14550` | MAVLink / QGroundControl |
|  `5600` | UDP H.264 video stream   |

---

## 📝 Notes

* The simulation is designed to run inside Docker.
* ROS 2 Humble and Gazebo Harmonic are configured inside the container.
* `ros_gz_bridge` is compiled from source to ensure compatibility with Gazebo Harmonic.
* QGroundControl runs on the host machine.
* The camera stream can be consumed both by ROS 2 applications and QGroundControl.
* The custom environment is loaded through the PX4 `PX4_GZ_WORLD` variable.

---

