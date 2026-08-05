#!/bin/bash

# Abilita la visualizzazione delle interfacce grafiche (Gazebo)
xhost +local:docker > /dev/null 2>&1

# Determina la root del progetto sull'Host
cd "$(dirname "$0")/.."
WORKSPACE_DIR=$(pwd)
CONTAINER_NAME="maradrone_container"
IMAGE_NAME="maradrone_image"

# Crea automaticamente il COLCON_IGNORE per proteggere la build di ROS 2
touch "$WORKSPACE_DIR/PX4-Autopilot/COLCON_IGNORE"

echo "Verifica dello stato del container '$CONTAINER_NAME'..."

# Controlla se il container esiste (indipendentemente dal fatto che sia acceso o spento)
if [ "$(docker ps -aq -f name=^/${CONTAINER_NAME}$)" ]; then
    # Se esiste, controlla se è attualmente in esecuzione
    if [ "$(docker ps -q -f name=^/${CONTAINER_NAME}$)" ]; then
        echo "--> Il container è già in esecuzione. Connessione in corso (exec)..."
        docker exec -it $CONTAINER_NAME bash
    else
        echo "--> Il container esiste ma è spento. Avvio in corso (start)..."
        docker start -i $CONTAINER_NAME
    fi
else
    # Se non esiste affatto, lo crea da zero (run)
    echo "--> Il container non esiste. Creazione e avvio in corso (run)..."
    docker run -it \
      --privileged \
      --network host \
      -e DISPLAY=$DISPLAY \
      -v /dev/bus/usb:/dev/bus/usb \
      -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
      -v "$WORKSPACE_DIR/src":/root/ros2_ws/src:rw \
      -v "$WORKSPACE_DIR/PX4-Autopilot":/root/ros2_ws/PX4-Autopilot:rw \
      --workdir="/root/ros2_ws" \
      --name $CONTAINER_NAME \
      $IMAGE_NAME bash
fi