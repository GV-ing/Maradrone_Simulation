FROM docker.io/px4io/px4-dev-base-jammy:latest

ENV DEBIAN_FRONTEND=noninteractive \
    LANG=en_US.UTF-8 \
    LC_ALL=en_US.UTF-8

USER root

# Installazione Base Tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    git bash-completion gedit sudo locales software-properties-common curl nano wget \
 && locale-gen en_US en_US.UTF-8 \
 && update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8 \
 && add-apt-repository universe \
 && rm -rf /var/lib/apt/lists/*

# Installazione Python
RUN apt-get update && apt-get install -y python3-pip \
 && rm -rf /var/lib/apt/lists/*

# Installazione ROS 2 Humble
RUN curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
      -o /usr/share/keyrings/ros-archive-keyring.gpg \
 && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] \
      http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" \
      > /etc/apt/sources.list.d/ros2.list \
 && apt-get update \
 && apt-get install -y ros-humble-desktop ros-dev-tools \
 && echo "source /opt/ros/humble/setup.bash" >> /root/.bashrc \
 && python3 -m pip install --user -U empy==3.3.4 pyros-genmsg setuptools \
 && rm -rf /var/lib/apt/lists/*

# Installazione Micro-XRCE-DDS-Agent
RUN git clone https://github.com/eProsima/Micro-XRCE-DDS-Agent.git /Micro-XRCE-DDS-Agent \
 && cd /Micro-XRCE-DDS-Agent \
 && mkdir build && cd build \
 && cmake .. \
 && make -j"$(nproc)" \
 && make install \
 && ldconfig /usr/local/lib/

# Installazione PlotJuggler e Gazebo Harmonic
RUN apt-get update && apt-get install -y ros-humble-plotjuggler-ros lsb-release gnupg \
 && curl -sSL https://packages.osrfoundation.org/gazebo.gpg \
      -o /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg \
 && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] https://packages.osrfoundation.org/gazebo/ubuntu-stable $(lsb_release -cs) main" \
      > /etc/apt/sources.list.d/gazebo-stable.list \
 && apt-get update \
 && apt-get install -y \
      gz-harmonic libgz-sim8-dev libgz-transport13-dev libgz-sensors8-dev \
      libgz-plugin2-dev libgz-utils2-dev libgz-msgs10-dev protobuf-compiler \
      libprotobuf-dev libopencv-dev \
 && rm -rf /var/lib/apt/lists/*

# Installazione Nav2, strumenti ROS, RQT e GStreamer per QGC
RUN apt-get update && apt-get install -y \
    ros-humble-controller-manager \
    ros-humble-ros2-control ros-humble-ros2-controllers ros-humble-ign-ros2-control \
    ros-humble-ign-ros2-control-demos ros-humble-joint-state-publisher-gui \
    ros-humble-xacro ros-humble-usb-cam ros-humble-image-pipeline \
    ros-humble-tf-transformations ros-humble-navigation2 ros-humble-nav2-bringup \
    ros-humble-robot-localization ros-humble-rqt-image-view \
    ros-humble-rtabmap-ros \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev gstreamer1.0-plugins-bad gstreamer1.0-plugins-good \
 && rm -rf /var/lib/apt/lists/*
# ==========================================
# Compilazione ROS-GZ Bridge per Harmonic
# ==========================================
WORKDIR /root/ros2_ws/src
RUN git clone https://github.com/gazebosim/ros_gz.git -b humble

WORKDIR /root/ros2_ws
# Eseguiamo la compilazione in un ambiente bash che fa prima il source di ROS 2
# Eseguiamo la compilazione in un ambiente bash (sequenziale per risparmiare RAM)
RUN /bin/bash -c "source /opt/ros/humble/setup.bash && \
    export GZ_VERSION=harmonic && \
    colcon build --packages-up-to ros_gz_bridge --cmake-args -DGZ_VERSION=harmonic --executor sequential"
    
# Aggiungiamo il source del workspace compilato al bashrc
RUN echo "source /root/ros2_ws/install/setup.bash" >> /root/.bashrc

RUN git config --global --add safe.directory '*'

# Clone di PX4 isolato dal workspace ROS 2.
# Pinnato allo stesso commit da cui è stato generato il sottomodulo
# src/px4_msgs (vedi il commit "Update to PX4 <sha>" di PX4BuildBot in quel
# repo), cosi' i nomi dei topic uXRCE-DDS versionati (es. vehicle_local_position_v1,
# vehicle_status_v4) restano stabili invece di cambiare ad ogni rebuild
# dell'immagine seguendo l'evolversi di PX4-Autopilot main.
ARG PX4_COMMIT=3f9c6ec2c3f93d233b1a54a8d59ef055c7675b4c
WORKDIR /root
RUN git clone https://github.com/PX4/PX4-Autopilot.git /root/PX4-Autopilot \
 && git -C /root/PX4-Autopilot checkout ${PX4_COMMIT} \
 && git -C /root/PX4-Autopilot submodule update --init --recursive

# --- INIZIO COPIA MAPPA E MODELLI ---
COPY src/maradrone_description/worlds/leonardo_race_field.sdf /root/PX4-Autopilot/Tools/simulation/gz/worlds/
COPY src/maradrone_description/models/leo_race_field /root/PX4-Autopilot/Tools/simulation/gz/models/leo_race_field
# --- FINE COPIA ---

# Impostazione della workdir predefinita per l'ingresso nel container
WORKDIR /root/ros2_ws