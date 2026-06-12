# SSD1963 Display Driver for Raspberry Pi

Drive an SSD1963 TFT LCD (480x272, 16-bit parallel 8080 interface) from a Raspberry Pi using `libbcm2835` GPIO bit-banging. Features interactive menu, UI widgets, QR code generation, and bitmap font rendering.

## Features

- **Optimized driver** — 2 GPIO calls per pixel (vs 32), batch writes, 16× faster screen fills
- **Interactive menu** — 4-button navigation (UP/DOWN/SELECT/BACK) with debounce
- **QR code generation** — Encode text/WiFi/URL as QR codes using Nayuki library
- **Bitmap font** — 8×8 font with 112 glyphs including arrows, symbols, icons
- **UI widgets** — ProgressBar (animated), CheckBox, TextBox, SelectiveMenu
- **Image display** — RGB565 raw binary files full-screen
- **Icons** — 8 built-in 16×16 icons (settings, photo, info, shapes, palette, etc.)

## Quick Start

```bash
# Build and run on Pi
make run

# Or remote build via SSH
make remote-run PI_HOST=pi@192.168.1.100
```

## Documentation

| File                        | Content                                 |
|-----------------------------|-----------------------------------------|
| `docs/API.md`               | Full driver API reference               |
| `docs/HARDWARE.md`          | Wiring diagram, pinout, timings         |
| `docs/MENU.md`              | Menu system and UI component guide      |
| `docs/OPTIMIZATION.md`      | Performance optimization details        |
| `docs/PROGRAMMING.md`       | Programming guide with examples         |

## Hardware

| Component | Details |
|-----------|---------|
| Display   | SSD1963, 480×272, RGB565, 16-bit parallel |
| Interface | 8080 bus (16 data + 5 control = 21 GPIOs) |
| GPIO lib  | [`libbcm2835`](http://www.airspayce.com/mikem/bcm2835/) |
| Buttons   | 4× GPIO with pull-up (UP=7, DOWN=8, SELECT=9, BACK=10) |
| Tested on | Raspberry Pi Zero 2W, Pi 4 |

## Project Structure

```
├── include/
│   ├── ssd1963.hpp         # Driver class, macros, bitmasks
│   ├── ssd1963_cmd.hpp     # Command constants
│   ├── config_hw.hpp       # Pins, dimensions, timings
│   ├── color.hpp           # RGB565 colors
│   ├── font.hpp            # 8×8 bitmap font + text rendering
│   ├── icons.hpp           # 16×16 icon bitmaps
│   ├── ui.hpp              # ProgressBar, CheckBox, TextBox, SelectiveMenu
│   ├── menu.hpp            # Menu class + callback declarations
│   ├── qrcodegen.hpp       # QR Code generator (Nayuki)
│   └── qr_display.hpp      # QR display helpers
├── src/
│   ├── main.cpp            # Entry point with demo menu
│   ├── menu.cpp            # Menu + 10 built-in actions
│   ├── ssd1963.cpp         # Driver implementation
│   └── qrcodegen.cpp       # QR Code generator
├── assets/
│   └── capibaras.rgb565    # Sample full-screen image
├── docs/
│   ├── API.md              # Driver API reference
│   ├── HARDWARE.md         # Wiring guide
│   ├── MENU.md             # UI component guide
│   ├── OPTIMIZATION.md     # Performance tuning
│   └── PROGRAMMING.md      # Developer guide
├── script_tools_ssd1963_rpi/  # PNG→RGB565 converter, deps installer
├── Makefile
└── README.md
```

## Menu Demo Items

| # | Item              | Description                              |
|---|-------------------|------------------------------------------|
| 1 | Color Test        | 7 color swatches with labels             |
| 2 | Show Photo        | Full-screen RGB565 image                 |
| 3 | Screen Info       | Driver/display information               |
| 4 | Draw Shapes       | Geometric primitives demo                |
| 5 | QR: GitHub URL    | QR code for opencode repository          |
| 6 | QR: WiFi Config   | QR code with WiFi credentials            |
| 7 | QR: Custom Text   | QR code with free text                   |
| 8 | About             | Version and credits                      |
| 9 | Exit              | Return to shell                          |

## Dependencies

```bash
# Build tools
sudo apt install build-essential python3-pil

# libbcm2835 (not in distro repos)
bash script_tools_ssd1963_rpi/install_deps.sh
```

## Image Conversion

```bash
python3 script_tools_ssd1963_rpi/convert_png_to_rgb.py input.png output.rgb565
```

## Troubleshooting

- `bcm2835.h: No such file` — Install libbcm2835
- `bcm2835_init() failed` — Run with `sudo`
- Blank screen — Check backlight (GPIO 0) and RESET (GPIO 6)
- Corrupted colors — Verify D0–D15 wiring
- Blurry image — Adjust `LCD_FPR` in `config_hw.hpp`
