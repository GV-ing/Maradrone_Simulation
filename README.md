# MaraDrone Simulation Framework

## Panoramica del Progetto

**MaraDrone Simulation** è un framework di simulazione per robotica aerea basato su:

- ROS 2 Humble
- Gazebo Harmonic
- PX4 Autopilot (SITL)

Il framework adotta un'architettura containerizzata tramite Docker per garantire la riproducibilità dell'ambiente di sviluppo e ridurre il carico cognitivo estraneo, in linea con i principi pedagogici del framework **Robotics Lab 2026**.

La struttura del repository integra i nodi di controllo e comunicazione custom (come `read_rpy`, `offboard_rl` e `force_land`) all'interno del workspace ROS 2, interfacciandosi con il firmware **PX4-Autopilot** e la libreria di messaggi `px4_msgs`.

---

# Struttura del Repository

```text
.
├── docker_scripts/
│   ├── Dockerfile
│   ├── docker_build.sh
│   ├── docker_run_container.sh
│   └── ...
│
├── PX4-Autopilot/
│   └── Firmware PX4 utilizzato per la simulazione SITL
│
└── ros2_ws/
    ├── src/
    │   ├── read_rpy/
    │   ├── offboard_rl/
    │   ├── force_land/
    │   └── px4_msgs/
    ├── build/
    ├── install/
    └── log/
```

## Pacchetti ROS 2

### `read_rpy`

Nodo per la lettura e la conversione dell'assetto del veicolo in angoli di Eulero mediante la libreria geometrica **Eigen**.

### `offboard_rl`

Nodo responsabile della gestione della modalità **Offboard**, dell'armamento del drone e dell'invio delle traiettorie di volo.

### `force_land`

Nodo di sicurezza che impone un atterraggio forzato quando vengono superate soglie altimetriche critiche.

### `px4_msgs`

Pacchetto contenente le interfacce di messaggistica utilizzate per la comunicazione tra ROS 2 e PX4.

---

# Guida Operativa

## 1. Avvio dell'ambiente Docker

Posizionarsi nella cartella contenente gli script Docker ed eseguire:

```bash
cd docker_scripts
./docker_run_container.sh
```

### Nota

Lo script verifica automaticamente lo stato del container:

- se il container è spento viene eseguito `docker start`;
- se non esiste viene creato automaticamente tramite `docker run`;
- se è già in esecuzione viene aperta una nuova sessione mediante `docker exec`.

---

## 2. Avvio di PX4 Autopilot e Gazebo

Una volta entrati nel container Docker (directory iniziale `/root/ros2_ws`), spostarsi nella directory di PX4 ed avviare la simulazione SITL con il modello **gz_x500**:

```bash
cd /root/ros2_ws/PX4-Autopilot
make px4_sitl gz_x500
```

---

## 3. Avvio di QGroundControl

Per motivi di compatibilità con le librerie grafiche e crittografiche (GLIBC e SSL), **QGroundControl** deve essere eseguito direttamente sul sistema Host (fuori dal container Docker).

Nel caso di installazione tramite Flatpak:

```bash
flatpak run org.mavlink.qgroundcontrol
```

Grazie alla configurazione della rete Docker in modalità **host** (`--network host`), QGroundControl si connetterà automaticamente al flusso MAVLink prodotto dalla simulazione PX4.

---

# Sviluppo ROS 2

Per compilare i pacchetti personalizzati del workspace:

```bash
cd /root/ros2_ws

colcon build --packages-select \
    read_rpy \
    offboard_rl \
    force_land

source install/setup.bash
```

---

# Tecnologie Utilizzate

- ROS 2 Humble
- Gazebo Harmonic
- PX4 Autopilot (SITL)
- Docker
- QGroundControl
- Eigen
- MAVLink
- `px4_msgs`

---

# Obiettivo del Framework

Il framework **MaraDrone Simulation** fornisce un ambiente di sviluppo riproducibile per applicazioni di robotica aerea, consentendo lo sviluppo, il test e la validazione di algoritmi di controllo, navigazione e apprendimento in simulazione prima del deployment su piattaforme reali.