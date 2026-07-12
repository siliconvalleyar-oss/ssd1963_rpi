#!/bin/bash
# Compilación remota en Raspberry Pi via SSH
set -e

PI_USER="$1"
PI_HOST="raspberry.local"
PI_PASS="$2"
PI_DIR="/home/pi/src/ssd1963_photo_png_2026_sucess_rgb"

echo "=== Compilando en ${PI_USER}@${PI_HOST}:${PI_DIR} ==="
# Agregar host key si no existe
ssh-keyscan -H "${PI_HOST}" >> ~/.ssh/known_hosts 2>/dev/null

sshpass -p "${PI_PASS}" ssh -o StrictHostKeyChecking=no "${PI_USER}@${PI_HOST}" "
    cd ${PI_DIR} && \
    git pull origin feature/qr-menu-docs && \
    make clean && \
    make -j4
"
echo "=== Listo ==="
