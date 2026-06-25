#!/bin/bash
# Convierte PNG a RGB565 (full-screen 480x272)
# Uso: ./script_tools/convert_png_to_rgb.sh <input.png> [output.rgb565]
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/.."
python3 script_tools/convert_png_to_rgb.py "$@"

