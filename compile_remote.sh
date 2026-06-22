#!/bin/bash
# Compilación remota en Raspberry Pi via SSH
set -e

PI_USER="pi"
PI_HOST="raspi.local"
PI_PASS="zero"
PI_DIR="/home/pi/src/ssd1963_photo_png_2026_sucess_rgb"

echo "=== Compilando en ${PI_USER}@${PI_HOST}:${PI_DIR} ==="
sshpass -p "${PI_PASS}" ssh "${PI_USER}@${PI_HOST}" "
    cd ${PI_DIR} && \
    git pull origin feature/qr-menu-docs && \
    make clean && \
    make -j4
"
echo "=== Listo ==="
