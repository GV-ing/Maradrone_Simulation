#!/bin/bash

# Spostati nella cartella contenente lo script
cd "$(dirname "$0")"

echo "Costruzione dell'immagine Docker per maradrone..."
docker build -t maradrone_image .
echo "Build completata con successo!"
