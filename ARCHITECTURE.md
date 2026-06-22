# Architecture

## Overview

This project implements a C++ driver for the Solomon Systech SSD1963 TFT LCD controller, interfaced via a 16-bit parallel 8080 bus using GPIO bit-banging on a Raspberry Pi. The driver uses `libbcm2835` for memory-mapped GPIO access to achieve the parallel protocol timing.

## System Diagram

```
+------------------+
|    main.cpp      |  Entry point: demo application
|  color blocks +  |
|  photo display   |
+--------+---------+
         |
         | creates and uses
+--------v---------+
|   SSD1963 class  |  Driver (ssd1963.cpp / ssd1963.hpp)
|  - init()        |
|  - draw_block()  |
|  - draw_pixel()  |
|  - draw_image()  |
|  - clear_screen()|
+--------+---------+
         |
         | depends on
+--------+---------+
|  libbcm2835      |  GPIO memory-mapped I/O library
|  bcm2835.h       |
+------------------+
```

## Layer Stack

```
+------------------------------------------------------------+
|  Application Layer  (main.cpp)                             |
|  Color blocks demo + full-screen photo display             |
+------------------------------------------------------------+
|  Driver Layer  (ssd1963.cpp / ssd1963.hpp)                 |
|  High-level drawing API, LCD initialization sequence       |
|  8080 parallel bus protocol (command/data write cycles)    |
+------------------------------------------------------------+
|  Hardware Abstraction  (libbcm2835)                        |
|  GPIO pin configuration, digital write, timing delays      |
+------------------------------------------------------------+
|  Physical Layer  (Raspberry Pi GPIO header -> SSD1963)     |
|  16 data lines + 5 control lines = 21 GPIOs                |
+------------------------------------------------------------+
```

## Key Design Decisions

### GPIO Bit-Banging Instead of SPI/I2C

- The SSD1963 supports an 8080-style parallel interface that requires 21 GPIO pins (16 data + 5 control).
- `libbcm2835` provides memory-mapped GPIO access, enabling fast bit-banging without kernel overhead.
- This approach is significantly faster than SPI for 16-bit parallel data transfers.

### PLL Configuration

```
REFclk (10 MHz) -> PLLclk (500 MHz) -> SYSclk (100 MHz)
    multiplier: 50x         divider: /5
```

The SSD1963's internal PLL is configured via:
- `SET_PLL_MN` (0xE2): sets multiplier (M=49 → x50) and divider (N=4 → /5)
- `SET_PLL` (0xE0): enables PLL with two-step activation (0x01 → delay → 0x03)

### Initialization Sequence

1. Hardware reset (GPIO RESET pin toggled LOW → HIGH)
2. Soft reset via command 0x01
3. PLL configuration and activation
4. LCD mode set (DE mode, RGB565, 480x272)
5. Pixel data interface (16-bit RGB565)
6. LSHIFT frequency (pixel clock: 2.0 MHz)
7. Horizontal timing registers
8. Vertical timing registers
9. Display ON command
10. Backlight ON (GPIO)

### 8080 Parallel Bus Protocol

**Command write** (RS = LOW):
```
CS_LOW → RS_LOW → set data bus → WR_LOW → WR_HIGH → CS_HIGH
```

**Data write** (RS = HIGH):
```
CS_LOW → RS_HIGH → set data bus → WR_LOW → WR_HIGH → CS_HIGH
```

## GPIO Pin Allocation

| Pins       | Count | Function             |
|------------|-------|----------------------|
| GPIO 12-27 | 16    | Data bus D0-D15      |
| GPIO 3     | 1     | Write strobe (WR)    |
| GPIO 4     | 1     | Register Select (RS) |
| GPIO 5     | 1     | Chip Select (CS)     |
| GPIO 6     | 1     | Hardware Reset       |
| GPIO 0     | 1     | Backlight control    |

## File Dependency Graph

```
main.cpp
  ├── ssd1963.hpp
  │   ├── bcm2835.h
  │   └── cstdint
  ├── color.hpp
  └── bcm2835.h

ssd1963.cpp
  ├── config_hw.hpp
  ├── ssd1963_cmd.hpp
  ├── ssd1963.hpp
  └── color.hpp
```

## Data Flow: Image Display

```
PNG file
    ↓  (external, Python PIL script)
RGB565 binary file (big-endian, 480x272×2 = 261,120 bytes)
    ↓  (fread in draw_image_rgb565())
uint16_t pixels (byte-swapped to little-endian for GPIO)
    ↓  (write_data_bus())
GPIO pins 12-27 (parallel 16-bit write)
    ↓  (WR strobe)
SSD1963 GRAM (write at current address)
    ↓
TFT Panel (pixel rendered)
```

## Timing Configuration

| Parameter              | Value  | Register               |
|------------------------|--------|------------------------|
| Horizontal pulse width | 41     | TFT_HSYNC_PULSE        |
| H-back porch           | 2      | TFT_HSYNC_BACK_PORCH   |
| H-front porch          | 2      | TFT_HSYNC_FRONT_PORCH  |
| H-total period         | 525    | TFT_HSYNC_PERIOD       |
| Vertical pulse width   | 10     | TFT_VSYNC_PULSE        |
| V-back porch           | 2      | TFT_VSYNC_BACK_PORCH   |
| V-front porch          | 2      | TFT_VSYNC_FRONT_PORCH  |
| V-total period         | 286    | TFT_VSYNC_PERIOD       |
| Pixel clock (LSHIFT)   | 2.0 MHz| LCD_FPR = 0x01E848    |
