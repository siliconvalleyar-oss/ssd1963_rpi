#!/bin/bash
# Convierte PNG a RGB565 (full-screen 480x272)
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/.."
python3 script_tools_ssd1963_rpi/convert_png_to_rgb.py

