# Development Guide

## Prerequisites

- Raspberry Pi (Zero 2W or Pi 4 recommended) or a Linux machine with `libbcm2835`
- ARM cross-compiler (optional, for local development without a Pi)
- `git` for version control

## Environment Setup

### 1. Install libbcm2835

```bash
# On Raspberry Pi (or use the install script)
sudo apt install build-essential wget
bash script_tools_ssd1963_rpi/install_deps.sh
```

The script downloads `libbcm2835-1.75` from source, compiles, and installs it.

### 2. Install Python image tools (optional, for image conversion)

```bash
sudo apt install python3-pil
```

### 3. Clone the repository

```bash
git clone https://github.com/siliconvalleyar-oss/ssd1963_rpi.git
cd ssd1963_rpi
```

## Build

### Local build (on Pi or machine with libbcm2835)

```bash
make              # build bin/ssd1963_app
make clean        # remove object files and binary
```

### Override compiler or flags

```bash
make CXX=clang++ CXXFLAGS="-Iinclude -O2"
```

## Run

```bash
make run          # build + run with sudo
```

Or manually:

```bash
sudo ./bin/ssd1963_app
```

GPIO access requires root on Raspbian, hence `sudo`.

## Remote Development Workflow

If developing on a desktop and deploying to a Pi:

### 1. Set the Pi host

```bash
export PI_HOST=pi@192.168.1.100
```

### 2. Sync and build remotely

```bash
make remote-build PI_HOST=pi@raspberry.local
```

### 3. Build and run remotely

```bash
make remote-run PI_HOST=pi@raspberry.local
```

### 4. Manual sync with rsync (alternative)

```bash
rsync -avz --exclude '.git' --exclude 'obj' --exclude 'bin' ./ pi@raspberry.local:/home/pi/src/ssd1963_photo_png_2026_sucess_rgb/
ssh pi@raspberry.local "cd /home/pi/src/ssd1963_photo_png_2026_sucess_rgb && make && sudo make run"
```

## Image Pipeline

### Converting a PNG to RGB565

```bash
python3 script_tools_ssd1963_rpi/convert_png_to_rgb.py input.png output.rgb565
```

The script:
1. Opens the PNG with PIL
2. Resizes to 480x272 (maintaining aspect ratio — use pre-cropped images for best results)
3. Converts each pixel to RGB565 (big-endian byte order)
4. Writes raw binary file (480 × 272 × 2 = 261,120 bytes)

### Shell wrapper

```bash
bash script_tools_ssd1963_rpi/convert_png_to_rgb.sh input.png
```

The output file can be placed in `assets/` and loaded by `draw_image_rgb565()`.

## Adding New Features

### Adding a Drawing Function

1. Declare the method in `include/ssd1963.hpp`
2. Implement it in `src/ssd1963.cpp`
3. Use `set_area()` + `WRITE_MEMORY_START` pattern for pixel operations
4. Use `write_command()` / `write_data()` for register access

Example — drawing a horizontal line:

```cpp
void SSD1963::draw_hline(uint16_t x, uint16_t y, uint16_t length, uint16_t color) {
    set_area(x, y, x + length - 1, y);
    write_command(SSD1963_WRITE_MEMORY_START);
    for (uint16_t i = 0; i < length; i++) {
        write_data(color);
    }
}
```

### Adding a New SSD1963 Command

1. Add the command opcode constant to `include/ssd1963_cmd.hpp`
2. Use `write_command(NEW_CMD)` followed by parameter `write_data()` calls

## Code Style

- Indentation: 4 spaces
- Naming: `snake_case` for functions and variables, `UPPER_CASE` for constants and macros
- Header guards: `#pragma once`
- Comments: Spanish (project convention) or English
- Includes: project headers first, then library headers

## Testing

There is currently no automated test suite. Testing is manual:

1. Build and run on the Pi
2. Verify color blocks display correctly
3. Verify photo displays full-screen
4. Check for visual artifacts (flickering, wrong colors, partial updates)

## Performance Considerations

- GPIO bit-banging is CPU-intensive. The pixel clock is set to 2.0 MHz.
- Full-screen fill (480×272 = 130,560 pixels) takes ~65ms at 2.0 MHz pixel clock.
- For better performance: consider using DMA or hardware SPI (if available on your display).
- The `write_data_bus()` function clears all 16 GPIO pins before setting them — this causes glitchy intermediate states but is simpler than read-modify-write.

## Troubleshooting Code Issues

| Problem                  | Likely Cause                          | Solution                             |
|--------------------------|---------------------------------------|--------------------------------------|
| `bcm2835.h` not found    | libbcm2835 not installed              | Run `install_deps.sh`                |
| Link error: `undefined reference to bcm2835_*` | Missing `-lbcm2835` | Verify LDFLAGS in Makefile           |
| Permission denied        | Running without root                  | Use `sudo` or set GPIO permissions   |
| Image not displaying     | RGB565 file not found                 | Place file in `assets/`              |
| Wrong colors             | Byte order mismatch                   | Check big-endian assumption          |
| Screen flickers          | PLL not locked or timing wrong        | Adjust `LCD_FPR` in `config_hw.hpp`  |

## Git Workflow

This project uses:
- `main` branch for stable releases
- Feature branches for development

Commit style: descriptive English or Spanish messages.

Tags follow semantic versioning (`v1.0.x`).

## Project References

- [SSD1963 Datasheet](helps/SSD1963.PDF) — official controller documentation
- [libbcm2835](http://www.airspayce.com/mikem/bcm2835/) — GPIO library
- [Raspberry Pi Pinout](https://pinout.xyz/) — GPIO reference
