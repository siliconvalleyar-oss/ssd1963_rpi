# SSD1963 Photo Display for Raspberry Pi

Drive an SSD1963 TFT LCD (480x272, 16-bit parallel 8080 interface) from a Raspberry Pi using `libbcm2835` GPIO bit-banging. Displays color blocks and a full-screen PNG photo.

## Hardware

| Component | Details |
|-----------|---------|
| Display   | SSD1963 controller, 480x272, RGB565, 16-bit parallel |
| Interface | 8080 parallel bus (16 data + 5 control pins = 21 GPIOs) |
| GPIO lib  | [`libbcm2835`](http://www.airspayce.com/mikem/bcm2835/) (memory-mapped I/O) |
| Tested on | Raspberry Pi Zero 2W, Raspberry Pi 4 |

### Pinout (GPIO → SSD1963)

| Function | BCM GPIO |
|----------|----------|
| D0–D15   | 12–27    |
| WR       | 3        |
| RS (DC)  | 4        |
| CS       | 5        |
| RESET    | 6        |
| BACKLIGHT| 0        |

## Dependencies

```bash
sudo apt install build-essential
```

Install `libbcm2835` from source (not in distro repos):

```bash
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.75.tar.gz
tar xzf bcm2835-1.75.tar.gz
cd bcm2835-1.75
./configure && make && sudo make install
```

Or use the script:
```bash
bash script_tools_ssd1963_rpi/install_deps.sh
```

## Build & Run

### Local build (requires libbcm2835 installed on this machine)

```bash
make          # builds bin/ssd1963_app
make run      # runs with sudo (GPIO requires root)
sudo ./bin/ssd1963_app
```

### Remote build via SSH (recommended if compiling on a Pi)

```bash
make remote-build   # ssh pi@raspberry.local "cd <dir> && make -j4"
make remote-run     # build + run on the Pi
```

Override host/path:
```bash
make remote-build PI_HOST=pi@192.168.1.100 PI_DIR=/home/pi/my_project
```

The program:
1. Initializes the SSD1963 (PLL, timings, 480x272)
2. Clears screen to black
3. Draws 7 color blocks (R,G,B,Y,C,M,W)
4. Renders `assets/capibaras.rgb565` full-screen

## Image Pipeline

```
source PNG → Python PIL → raw .rgb565 (RGB565 big-endian) → display
```

### Converting your own images

```bash
python3 script_tools_ssd1963_rpi/convert_png_to_rgb.py input.png output.rgb565
```

Or use the shell wrapper (auto-names output):
```bash
bash script_tools_ssd1963_rpi/convert_png_to_rgb.sh input.png
```

Requirements for conversion: `python3-pil` (`sudo apt install python3-pil`)

## Project Structure

```
ssd1963_photo_png_2026_sucess_rgb/
├── include/
│   ├── ssd1963.hpp        # SSD1963 class (GPIO ops, init, drawing)
│   ├── ssd1963_cmd.hpp    # SSD1963 command constants (~100+)
│   ├── config_hw.hpp      # Pin definitions, LCD timings, dimensions
│   └── color.hpp          # RGB565 macro + basic color constants
├── src/
│   ├── main.cpp           # Test sequence: color blocks → photo
│   └── ssd1963.cpp        # Bus init, command/data, drawing routines
├── assets/
│   ├── capibaras.png      # Source image (480x272)
│   └── capibaras.rgb565   # Pre-converted RGB565 for direct display
├── script_tools_ssd1963_rpi/
│   ├── convert_png_to_rgb.py  # PNG → RGB565 converter
│   ├── convert_png_to_rgb.sh  # Shell wrapper
│   └── install_deps.sh        # Installs libbcm2835 + build tools
├── helps/                 # Datasheets, pinout docs, installation guides
├── Makefile
├── .gitignore
└── README.md
```

## Documentation

- [**Architecture**](ARCHITECTURE.md) — Software architecture, layers, data flow, timing configuration
- [**Changelog**](CHANGELOG.md) — Version history and release notes
- [**Hardware Guide**](docs/hardware.md) — Detailed pinout, wiring diagram, connection checklist
- [**API Reference**](docs/api.md) — Full SSD1963 class API, GPIO macros, commands, color constants
- [**Development Guide**](docs/development.md) — Setup, build, remote workflow, image pipeline, troubleshooting

## Troubleshooting

- **"bcm2835.h not found"** – `libbcm2835` is not installed. Run `bash script_tools_ssd1963_rpi/install_deps.sh`.
- **No display output** – Check wiring. Backlight on GPIO 0 must be high.
- **Blurry image** – Verify LCD_FPR in `config_hw.hpp` matches your display's pixel clock.
- **Permission denied** – Run with `sudo` (GPIO access requires root).
