FROM px4io/px4-dev-base-jammy:latest

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

# Installazione ROS Gazebo bridge e Nav2
RUN apt-get update && apt-get install -y \
    ros-humble-ros-gz ros-humble-ros-gz-bridge ros-humble-controller-manager \
    ros-humble-ros2-control ros-humble-ros2-controllers ros-humble-ign-ros2-control \
    ros-humble-ign-ros2-control-demos ros-humble-joint-state-publisher-gui \
    ros-humble-xacro ros-humble-usb-cam ros-humble-image-pipeline \
    ros-humble-tf-transformations ros-humble-navigation2 ros-humble-nav2-bringup \
    ros-humble-robot-localization \
 && rm -rf /var/lib/apt/lists/*

# Safety Git per PX4-Autopilot
RUN git config --global --add safe.directory '*'

