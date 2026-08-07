#!/bin/bash

# Spostati nella directory principale del progetto (un livello sopra lo script)
cd "$(dirname "$0")/.."

echo "Costruzione dell'immagine Docker per maradrone..."
# Usa il punto (.) per prendere tutta la cartella principale come contesto,
# e specifica dove trovare il Dockerfile con -f
docker build -t maradrone_image -f docker_scripts/Dockerfile .
echo "Build completata con successo!"