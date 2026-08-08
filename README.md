# Maradrone Simulation 🚁

[\![ROS 2](https://img.shields.io/badge/ROS_2-Humble-3498db.svg)](https://docs.ros.org/en/humble/)
[\![Gazebo](https://img.shields.io/badge/Gazebo-Harmonic-orange.svg)](https://gazebosim.org/home)
[\![PX4](https://img.shields.io/badge/PX4-Autopilot-blue.svg)](https://px4.io/)
[\![Docker](https://img.shields.io/badge/Docker-Enabled-2496ed.svg)](https://www.docker.com/)

Una simulazione Dockerizzata di **Maradrone** che unisce:

* PX4 Autopilot SITL
* ROS 2 Humble
* Gazebo Harmonic
* QGroundControl
* una mappa custom `leonardo_race_field`

---

## 📌 Perché questa repo è diversa

Questa repository differisce strutturalmente rispetto le altre due definite per il framework didattico `Armando-Simulation` o `Fra2mo-Simulation`.

### Differenze chiave

* `Armando-Simulation` e `Fra2mo-Simulation` si basano interamente su ROS 2 e condividono un'architettura simile.
* Questa repository invece integra un'installazione completa di PX4 Autopilot in modalità SITL.
* Nel `Dockerfile`:
  * viene clonato `PX4-Autopilot` in `/root/PX4-Autopilot`
  * la cartella PX4 è esterna al workspace ROS 2
  * la mappa custom `leonardo_race_field` viene copiata direttamente nei sorgenti PX4
  * i modelli custom vengono copiati dentro `/root/PX4-Autopilot/Tools/simulation/gz/models/`
* La struttura architetturale e le metodologie di questa repo sono quindi diverse rispetto ai due progetti ROS 2 puri.

### Comportamento del container

* Il container non è avviato con `--rm`.
* Quando si esce dalla shell, il container viene fermato ma non cancellato.
* Questo permette di non perdere le modifiche fatte dentro il container, fondamentale se si modifica il codice PX4 o si salva una nuova configurazione.

---

## 📑 Table of Contents

* [Features](#-features)
* [Prerequisites](#-prerequisites)
* [Installation](#-installation)
* [Usage](#-usage)
  * [Build e avvio del Docker](#build-e-avvio-del-docker)
  * [Avvio di PX4 e caricamento della mappa custom](#avvio-di-px4-e-caricamento-della-mappa-custom)
  * [Download e avvio di QGroundControl da un terminale esterno](#download-e-avvio-di-qgroundcontrol-da-un-terminale-esterno)
  * [Avvio dei bridge tra Gazebo e ROS 2](#avvio-dei-bridge-tra-gazebo-e-ros-2)
  * [Avvio del microagent e funzione](#avvio-del-microagent-e-funzione)
  * [Avvio dei nodi custom e dipendenze](#avvio-dei-nodi-custom-e-dipendenze)
* [Structure & PX4 communication](#structure--px4-communication)
* [Repository Structure](#repository-structure)

---

## ✨ Features

* PX4 Autopilot SITL con drone `x500_depth`
* Mondo custom `leonardo_race_field`
* Interfaccia ROS 2 `px4_msgs`
* `ros_gz_bridge` compilato per Gazebo Harmonic
* Camera IMX214 bridged in ROS 2
* GStreamer e streaming UDP per QGroundControl
* Ambiente completamente Dockerizzato

---

## 🛠 Prerequisites

* Docker installato sull'host
* QGroundControl installato sull'host
* Permessi Docker senza `sudo` (consigliato)

```bash
sudo usermod -aG docker $USER
```

Dopo aver aggiunto l'utente al gruppo Docker, esci e rientra nella sessione.

---

## 📥 Installation

### Build e avvio del Docker

```bash
cd /path/to/Maradrone_Simulation
./docker_scripts/docker_build_image.sh
./docker_scripts/docker_run_container.sh
```

Il primo comando costruisce l'immagine Docker e prepara:

* ROS 2 Humble
* Gazebo Harmonic
* `ros_gz_bridge`
* Micro-XRCE-DDS-Agent
* la copia di `PX4-Autopilot`
* il mondo e i modelli custom dentro PX4

### Quando il container esiste già

Se il container esiste ma è spento si puo usare il comando di run che in questa configurazione permette anche di entrare in un container già in esecuzuzione:

```bash
./docker_scripts/docker_run_container.sh

```

---

## 🚀 Avvio di PX4 e caricamento della mappa custom

Una volta dentro il container:

```bash
cd /root/PX4-Autopilot
PX4_GZ_WORLD=leonardo_race_field make px4_sitl gz_x500_depth
```

Questo comando avvia:

* PX4 SITL
* Gazebo Harmonic
* il mondo custom `leonardo_race_field`
* il modello `x500_depth`

---

## 🛰 Download e avvio di QGroundControl da un terminale esterno

Scarica QGroundControl dal sito ufficiale:

https://docs.qgroundcontrol.com/master/en/getting_started/download_and_install.html

Se usi l'AppImage su Linux:

```bash
cd ~/Downloads
chmod +x QGroundControl.AppImage
./QGroundControl.AppImage
```

Collegati alla simulazione PX4 su porta MAVLink:

```text
14550
```

Per il video UDP in QGC usa la porta:

```text
5600
```

---

## 🔌 Avvio dei bridge tra Gazebo e ROS 2

Nel container:

```bash
docker exec -it maradrone_container bash
ros2 run ros_gz_bridge parameter_bridge   /world/leonardo_race_field/model/x500_depth_0/link/camera_link/sensor/IMX214/image@sensor_msgs/msg/Image[gz.msgs.Image
```

Poi avvia `rqt_image_view` e seleziona il topic:

```text
/world/leonardo_race_field/model/x500_depth_0/link/camera_link/sensor/IMX214/image
```

---

## ⚙️ Avvio del microagent e funzione

Durante la build del container vengono installati i componenti necessari per il bridge PX4/ROS 2:

* `Micro-XRCE-DDS-Agent`
* `ros_gz_bridge`

Il microagent è il componente che fa da companion tra PX4 e ROS 2:

* PX4 usa `uxrce_dds_client`
* il container esegue `Micro-XRCE-DDS-Agent`
* `px4_msgs` rappresenta i messaggi PX4 su ROS 2

In questo progetto, il microagent è già predisposto e non richiede un avvio manuale separato.

---

## 🧩 Avvio dei nodi custom e dipendenze

Nel container, costruisci il workspace ROS 2:

```bash
cd /root/ros2_ws
colcon build --packages-select px4_msgs maradrone_framework offboard_rl force_land read_rpy
source install/setup.bash
```

### Nodi custom inclusi

* `maradrone_framework`
  * eseguibile: `offboard_takeoff`
  * invia:
    * `/fmu/in/offboard_control_mode`
    * `/fmu/in/trajectory_setpoint`
    * `/fmu/in/vehicle_command`

* `offboard_rl`
  * eseguibile: `go_to_point`
  * legge:
    * `/fmu/out/vehicle_local_position`
    * `/fmu/out/vehicle_attitude`
  * invia:
    * `/fmu/in/offboard_control_mode`
    * `/fmu/in/trajectory_setpoint`
    * `/fmu/in/vehicle_command`

* `force_land`
  * eseguibile: `force_land`
  * invia il comando di atterraggio di emergenza tramite `/fmu/in/vehicle_command`

* `read_rpy`
  * eseguibile: `read_rpy`
  * legge l'assetto dal topic `/fmu/out/vehicle_attitude`

### Esempi di esecuzione

```bash
ros2 run maradrone_framework offboard_takeoff
ros2 run offboard_rl go_to_point
ros2 run force_land force_land
ros2 run read_rpy read_rpy
```

> Questi nodi richiedono che `px4_msgs` sia compilato e che il bridge PX4/ROS 2 sia attivo.

---

## 📘 Structure & PX4 communication

### Panoramica architetturale

1. `PX4-Autopilot` esegue la simulazione SITL.
2. Gazebo Harmonic visualizza il drone e il mondo custom.
3. `ros_gz_bridge` porta i sensori Gazebo su ROS 2.
4. `px4_msgs` converte i messaggi PX4 uORB in tipi ROS 2.
5. I nodi ROS 2 pubblicano comandi su `/fmu/in/...`.
6. PX4 restituisce lo stato su `/fmu/out/...`.
7. QGroundControl comunica con PX4 via MAVLink su `14550`.

### Messaggi chiave

* `/fmu/in/offboard_control_mode`
* `/fmu/in/trajectory_setpoint`
* `/fmu/in/vehicle_command`
* `/fmu/out/vehicle_local_position`
* `/fmu/out/vehicle_attitude`

### Differenza rispetto ad un sistema ROS 2 puro

Questa repository non è una semplice simulazione ROS 2: è un sistema ibrido PX4 + ROS 2.

* `px4_msgs` è l'interfaccia che rende i messaggi PX4 disponibili su ROS 2.
* I nodi custom inviano comandi direttamente a PX4 tramite topic `/fmu/in/...`.
* Il firmware PX4 rimane il componente di controllo centrale.

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

## 💡 Note finali

* Questa repo utilizza metodologie funzionali diverse rispetto a `Armando-Simulation` e `Fra2mo-Simulation`.
* Il Dockerfile prepara il codice PX4 e copia la mappa custom all'interno di `/root/PX4-Autopilot`.
* Il container conserva lo stato tra le uscite, così le modifiche non vanno perse.
* I pacchetti `maradrone_framework`, `offboard_rl`, `force_land` e `read_rpy` dipendono da `px4_msgs` e dal bridge PX4/ROS 2.
