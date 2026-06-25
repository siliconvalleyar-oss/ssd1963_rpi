#!/bin/bash
# Compilación remota en Raspberry Pi
# Uso: ./ejecutar_compilacion_remota.sh <user> <password> <ip>
#
# Ejemplo:
#   ./ejecutar_compilacion_remota.sh pi zero 192.168.1.37
#
# Pushea a GitHub, luego SSH al Pi, git pull y make.
set -e

if [ $# -ne 3 ]; then
    echo "Uso: $0 <user> <password> <ip>"
    echo ""
    echo "Ejemplo:"
    echo "  $0 pi zero 192.168.1.37"
    echo "  $0 pi raspberry raspberrypi.local"
    exit 1
fi

PI_USER="$1"
PI_PASS="$2"
PI_HOST="$3"
PI_DIR="/home/${PI_USER}/src/ssd1963_photo_png_2026_sucess_rgb"
BRANCH="feature/qr-menu-docs"

echo "=========================================="
echo " 1. Git push a GitHub (rama ${BRANCH})"
echo "=========================================="
git push origin "${BRANCH}" --follow-tags --force

echo ""
echo "=========================================="
echo " 2. SSH a ${PI_USER}@${PI_HOST}"
echo "    git pull + make"
echo "=========================================="

ssh-keyscan -H "${PI_HOST}" >> ~/.ssh/known_hosts 2>/dev/null

sshpass -p "${PI_PASS}" ssh -o StrictHostKeyChecking=no "${PI_USER}@${PI_HOST}" "
    cd ${PI_DIR} && \
    git pull origin ${BRANCH} && \
    make clean && \
    make -j4
"

echo ""
echo "=========================================="
echo " 3. Compilación exitosa"
echo "    Ejecutar en el Pi:"
echo "    sudo ${PI_DIR}/bin/ssd1963_app"
echo "=========================================="
