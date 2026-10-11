#!/bin/bash
# Script para: git pull, compilar y ejecutar en la Raspberry Pi
# Uso: ./git_pull_compile_exec.sh [branch]

set -e

BRANCH="${1:-feature/qr-menu-docs}"
REPO_DIR="/home/pi/src/ssd1963_photo_png_2026_sucess_rgb"
BINARY="bin/ssd1963_app"

cd "$REPO_DIR"

echo "=== Git Pull ==="
git fetch origin
git checkout "$BRANCH"
git pull origin "$BRANCH"

echo "=== Compilando ==="
make clean
make -j4

echo "=== Ejecutando ==="
sudo ./"$BINARY"
