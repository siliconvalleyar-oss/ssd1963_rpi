# API Reference

## SSD1963 Class

Defined in `include/ssd1963.hpp`, implemented in `src/ssd1963.cpp`.

### Constructor

```cpp
SSD1963::SSD1963();
```

Creates a new SSD1963 driver instance. Does not initialize hardware.

---

### Initialization

```cpp
void SSD1963::setup_gpio();
```

Configures all 21 GPIO pins (16 data + 5 control) as outputs with default inactive states (CS=HIGH, WR=HIGH, RS=HIGH, RESET=HIGH, BACKLIGHT=OFF). Must be called after `bcm2835_init()`.

```cpp
void SSD1963::init();
```

Performs the full SSD1963 initialization sequence:
1. Hardware reset (RESET pin toggle)
2. Soft reset (command 0x01)
3. PLL configuration (10 MHz REF → 500 MHz PLL → 100 MHz SYS)
4. LCD mode setup (DE mode, RGB565, 480x272)
5. Pixel data interface (16-bit 565)
6. LSHIFT / pixel clock frequency (2.0 MHz)
7. Horizontal timing registers
8. Vertical timing registers
9. Display ON
10. Backlight ON

---

### Drawing Functions

```cpp
void SSD1963::clear_screen(uint16_t color);
```

Fills the entire 480x272 framebuffer with a solid `color` in RGB565 format.

```cpp
void SSD1963::draw_block(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
```

Draws a filled rectangle starting at `(x, y)` with the given `width` and `height` in `color`.

```cpp
void SSD1963::draw_pixel(uint16_t x, uint16_t y, uint16_t color);
```

Sets a single pixel at `(x, y)` to `color`.

```cpp
void SSD1963::draw_image_rgb565(const char* filepath);
```

Reads a raw RGB565 binary file (big-endian byte order, 480x272 = 261,120 bytes) and writes it full-screen. The file must contain exactly `LCD_WIDTH * LCD_HEIGHT * 2` bytes.

---

### Utility

```cpp
void SSD1963::delay_ms(uint32_t ms);
```

Blocks for `ms` milliseconds using `usleep()`.

---

## GPIO Control Macros

Defined in `include/ssd1963.hpp`.

| Macro            | Description             |
|------------------|-------------------------|
| `RS_LOW()`       | Select command register |
| `RS_HIGH()`      | Select data register    |
| `WR_LOW()`       | Start write strobe      |
| `WR_HIGH()`      | End write strobe        |
| `CS_LOW()`       | Select chip             |
| `CS_HIGH()`      | Deselect chip           |
| `RESET_LOW()`    | Assert hardware reset   |
| `RESET_HIGH()`   | Release hardware reset  |
| `BACKLIGHT_ON()` | Turn backlight on       |
| `BACKLIGHT_OFF()`| Turn backlight off      |

---

## Private Helpers

```cpp
void SSD1963::write_data_bus(uint16_t data);
```

Bit-bangs a 16-bit value onto GPIO pins 12-27. Clears all data pins first, then sets the appropriate bits HIGH.

```cpp
void SSD1963::write_command(uint8_t cmd);
```

Writes an 8-bit command byte using the 8080 protocol (RS=LOW).

```cpp
void SSD1963::write_data(uint16_t data);
```

Writes a 16-bit data value using the 8080 protocol (RS=HIGH).

```cpp
void SSD1963::set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
```

Sets the active window/area for subsequent pixel writes. Sends column and page address commands with the specified coordinates.

---

## Color Constants

Defined in `include/color.hpp`.

### Macro

```cpp
RGB565CONVERT(red, green, blue)
```

Converts 8-bit R, G, B values to packed RGB565 format:
- Red: bits 15-11 (5 bits)
- Green: bits 10-5 (6 bits)
- Blue: bits 4-0 (5 bits)

### Basic Colors

| Constant  | RGB565 Value | Description          |
|-----------|-------------|----------------------|
| `BLACK`   | `0x0000`    | Black                |
| `BLUE`    | `0x001F`    | Blue                 |
| `RED`     | `0xF800`    | Red                  |
| `GREEN`   | `0x07E0`    | Green                |
| `CYAN`    | `0x07FF`    | Cyan                 |
| `MAGENTA` | `0xF81F`    | Magenta              |
| `YELLOW`  | `0xFFE0`    | Yellow               |
| `WHITE`   | `0xFFFF`    | White                |

---

## Key SSD1963 Commands

Defined in `include/ssd1963_cmd.hpp`.

### Initialization Commands

| Command Macro             | Opcode | Purpose                   |
|---------------------------|--------|---------------------------|
| `SSD1963_SOFT_RESET`      | 0x01   | Software reset            |
| `SSD1963_SET_PLL_MN`      | 0xE2   | PLL multiplier/divider    |
| `SSD1963_SET_PLL`         | 0xE0   | PLL enable/control        |
| `SSD1963_SET_LCD_MODE`    | 0xB0   | LCD mode configuration    |
| `SSD1963_SET_PIXEL_DATA_INTERFACE` | 0xF0 | Pixel data interface mode |
| `SSD1963_SET_LSHIFT_FREQ` | 0xE6   | Pixel clock frequency     |
| `SSD1963_SET_HOR_PERIOD`  | 0xB4   | Horizontal timing         |
| `SSD1963_SET_VER_PERIOD`  | 0xB6   | Vertical timing           |

### Drawing Commands

| Command Macro             | Opcode | Purpose                   |
|---------------------------|--------|---------------------------|
| `SSD1963_SET_COLUMN_ADDRESS` | 0x2A | Set column start/end    |
| `SSD1963_SET_PAGE_ADDRESS`   | 0x2B | Set page start/end     |
| `SSD1963_WRITE_MEMORY_START` | 0x2C | Begin writing to GRAM  |
| `SSD1963_ON_DISPLAY`      | 0x29   | Turn display on           |
| `SSD1963_BLANK_DISPLAY`   | 0x28   | Blank display             |

### Pixel Data Interface Modes

| Macro                     | Value | Description               |
|---------------------------|-------|---------------------------|
| `SSD1963_PDI_8BIT`        | 0x00  | 8-bit interface           |
| `SSD1963_PDI_16BIT565`    | 0x03  | 16-bit RGB565 (used)      |
| `SSD1963_PDI_16BIT555`    | 0x01  | 16-bit RGB555             |
| `SSD1963_PDI_18BIT666`    | 0x04  | 18-bit RGB666             |
| `SSD1963_PDI_24BIT888`    | 0x06  | 24-bit RGB888             |

---

## Configuration Parameters

Defined in `include/config_hw.hpp`.

| Parameter                | Value      | Description                |
|--------------------------|------------|----------------------------|
| `SSD1963_LCD_D0..D15`   | 12..27     | GPIO data pin mapping      |
| `SSD1963_LCD_WR`         | 3          | Write strobe pin           |
| `SSD1963_LCD_RS`         | 4          | Register select pin        |
| `SSD1963_LCD_CS`         | 5          | Chip select pin            |
| `SSD1963_LCD_RESET`      | 6          | Hardware reset pin         |
| `SSD1963_LCD_BACKLIGHT`  | 0          | Backlight control pin      |
| `LCD_WIDTH`              | 480        | Display width in pixels    |
| `LCD_HEIGHT`             | 272        | Display height in pixels   |
| `LCD_FPR`                | 0x13332   | Pixel clock divider (~9 MHz)|
| `TFT_HSYNC_PULSE`        | 41         | H-sync pulse width         |
| `TFT_HSYNC_BACK_PORCH`   | 2          | H-back porch               |
| `TFT_HSYNC_FRONT_PORCH`  | 2          | H-front porch              |
| `TFT_VSYNC_PULSE`        | 10         | V-sync pulse width         |
| `TFT_VSYNC_BACK_PORCH`   | 2          | V-back porch               |
| `TFT_VSYNC_FRONT_PORCH`  | 2          | V-front porch              |
