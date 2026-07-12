#!/usr/bin/env bash
set -e

echo "=== Installing build dependencies ==="
sudo apt update
sudo apt install -y build-essential wget

echo "=== Installing libbcm2835 ==="
if [ -f /usr/local/include/bcm2835.h ]; then
    echo "libbcm2835 already installed, skipping."
else
    cd /tmp
    wget -q http://www.airspayce.com/mikem/bcm2835/bcm2835-1.75.tar.gz
    tar xzf bcm2835-1.75.tar.gz
    cd bcm2835-1.75
    ./configure
    make
    sudo make install
    cd /
    rm -rf /tmp/bcm2835-1.75 /tmp/bcm2835-1.75.tar.gz
    echo "libbcm2835 installed successfully."
fi

echo "=== Installing Python image tools (optional) ==="
sudo apt install -y python3-pil || true

echo "=== Done. Run 'make' to build. ==="
