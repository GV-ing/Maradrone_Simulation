# Maradrone Simulation 🚁

[![ROS 2](https://img.shields.io/badge/ROS_2-Humble-3498db.svg)](https://docs.ros.org/en/humble/)
[![Gazebo](https://img.shields.io/badge/Gazebo-Harmonic-orange.svg)](https://gazebosim.org/home)
[![PX4](https://img.shields.io/badge/PX4-Autopilot-blue.svg)](https://px4.io/)
[![Docker](https://img.shields.io/badge/Docker-Enabled-2496ed.svg)](https://www.docker.com/)

A fully Dockerized simulation environment for **Maradrone**, based on the PX4 Autopilot ecosystem, ROS 2 Humble, and Gazebo Harmonic. This repository contains the setup for simulating an `x500_depth` drone inside a custom environment (`leonardo_race_field`), with full support for MAVLink telemetry, QGroundControl (QGC) autonomous missions (like Survey), and real-time video streaming via `ros_gz_bridge` and GStreamer.

---

## 📑 Table of Contents
- [Features](#-features)
- [Prerequisites](#-prerequisites)
- [Installation (Docker)](#-installation-docker)
- [Usage](#-usage)
  - [1. Start the Simulation](#1-start-the-simulation)
  - [2. View the Camera (ROS 2 / RQT)](#2-view-the-camera-ros-2--rqt)
  - [3. QGroundControl & GStreamer](#3-qgroundcontrol--gstreamer)
- [Repository Structure](#-repository-structure)

---

## ✨ Features
* **Custom Environment**: Includes the `leonardo_race_field` custom world and 3D models seamlessly injected into the PX4 Gazebo directories.
* **Camera Streaming (ROS 2)**: Bridges the Gazebo camera topic to a standard `sensor_msgs/Image` topic using a source-compiled `ros_gz_bridge` (Harmonic compatible).
* **QGroundControl Integration**: Ready-to-use GStreamer plugins to stream the video feed directly to QGC and execute autonomous waypoint/survey missions.
* **Fully Dockerized**: No need to pollute your local machine with ROS 2, Gazebo, or PX4 dependencies. Everything runs inside an isolated, reproducible container.

---

## 🛠 Prerequisites
You only need to have the following installed on your host machine:
* [Docker](https://docs.docker.com/engine/install/) (Ensure your user is added to the `docker` group)
* [QGroundControl](https://docs.qgroundcontrol.com/master/en/getting_started/download_and_install.html) (AppImage or Flatpak)

---

## 📥 Installation (Docker)

Clone the repository to your local machine:

```bash
git clone [https://github.com/GV-ing/Maradrone_Simulation.git](https://github.com/GV-ing/Maradrone_Simulation.git)
cd Maradrone_Simulation