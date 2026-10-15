# SSD1963 Photo Display for Raspberry Pi

Drive an SSD1963 TFT LCD (480x272, 16-bit parallel 8080 interface) from a Raspberry Pi using `libbcm2835` GPIO bit-banging. Displays color blocks and a full-screen PNG photo, and includes a **Pac-Man arcade game** (`bin/pacman_game`).

## Hardware

| Component | Details |
|-----------|---------|
| Display   | SSD1963 controller, 480x272, RGB565, 16-bit parallel |
| Interface | 8080 parallel bus (16 data + 5 control pins = 21 GPIOs) |
| GPIO lib  | [`libbcm2835`](http://www.airspayce.com/mikem/bcm2835/) (memory-mapped I/O) |
| Tested on | Raspberry Pi Zero 2W, Raspberry Pi 4 |

### Botones (juego Pac-Man, activos en bajo con pull-up interno)

| Función | BCM GPIO |
|---------|----------|
| Arriba    | 8  |
| Abajo     | 9  |
| Izquierda | 10 |
| Derecha   | 11 |
| Inicio / Pausa | 7 |

Cada botón conecta su GPIO a GND (el pull-up lo activa el propio programa).

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
make          # builds bin/pacman_game (juego Pac-Man)
make run      # runs with sudo (GPIO requires root)
sudo ./bin/pacman_game
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

## Pac-Man Arcade Game

Juego arcade completo en `src/pacman.cpp` + `include/pacman.hpp` sobre el mismo
driver SSD1963:

- Laberinto clásico 28x31 (casillas de 8 px) + HUD con puntaje, récord, nivel y vidas
- 4 fantasmas con IA clásica (scatter/chase, modo asustado con pastillas,
  ojos que vuelven a la casa, salida escalonada de la casa, túnel lateral)
- Estados: título, READY, juego, pausa, muerte, parpadeo de muros al
  completar el nivel, game over
- Render a 60 Hz con back-buffer en RAM; solo se suben al display los
  rectángulos que cambian (el bus 8080 bit-bangueado no da para más)
- `write_data_bus()` optimizado: los 16 pines GPIO12-27 se escriben con una
  sola escritura de registro (GPSET0/GPCLR0) vía `bcm2835_gpio_write_mask`

Puntuación: punto 10, pastilla 50, fantasmas 200/400/800/1600, vida extra a 10000.

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
│   ├── ssd1963.hpp        # SSD1963 class (GPIO ops, init, drawing, draw_buffer)
│   ├── ssd1963_cmd.hpp    # SSD1963 command constants (~100+)
│   ├── config_hw.hpp      # Pin definitions, LCD timings, dimensions
│   ├── color.hpp          # RGB565 macro + basic color constants
│   └── pacman.hpp         # Clase del juego Pac-Man
├── src/
│   ├── main.cpp           # Test sequence: color blocks → photo
│   ├── main_pacman.cpp    # Entry point del juego Pac-Man (bucle 60 Hz)
│   ├── pacman.cpp         # Lógica y render del juego Pac-Man
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

## Troubleshooting

- **"bcm2835.h not found"** – `libbcm2835` is not installed. Run `bash script_tools_ssd1963_rpi/install_deps.sh`.
- **No display output** – Check wiring. Backlight on GPIO 0 must be high.
- **Blurry image** – Verify LCD_FPR in `config_hw.hpp` matches your display's pixel clock.
- **Permission denied** – Run with `sudo` (GPIO access requires root).
