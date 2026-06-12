# SSD1963 Driver API Reference

## Class: `SSD1963`

Main driver class for the SSD1963 TFT LCD controller.

### Constructor

```cpp
SSD1963();
```

### Initialization

```cpp
void setup_gpio();
```
Configures all GPIO pins as outputs (data bus D0-D15, RS, WR, CS, RESET, BACKLIGHT). Sets initial idle states (CS high, WR high, RS high, backlight off).

```cpp
void init();
```
Full initialization sequence:
1. Hardware reset (RESET low 10ms, high 10ms)
2. Software reset command
3. PLL configuration (M=49, N=4 → ~122.5 MHz SYSCLK)
4. LCD mode setup (480x272, TFT)
5. Pixel data interface (16-bit RGB565)
6. Line shift frequency (FPR)
7. Horizontal/vertical timing (sync pulse, back porch, front porch)
8. Display ON, backlight ON

### Drawing Primitives

```cpp
void clear_screen(uint16_t color);
```
Fills entire screen with a single RGB565 color. Uses batch writes for performance.

```cpp
void draw_pixel(uint16_t x, uint16_t y, uint16_t color);
```
Sets a single pixel at (x, y).

```cpp
void draw_block(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
```
Fills a rectangular region with a solid color.

```cpp
void draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
```
Draws a rectangular outline (4 lines) in the given color.

```cpp
void draw_line_h(uint16_t x, uint16_t y, uint16_t len, uint16_t color);
```
Draws a horizontal line from (x, y) of length `len`.

```cpp
void draw_line_v(uint16_t x, uint16_t y, uint16_t len, uint16_t color);
```
Draws a vertical line from (x, y) of length `len`.

```cpp
void draw_image_rgb565(const char* filepath);
```
Loads a raw RGB565 binary file and displays it full-screen. File must contain exactly `LCD_WIDTH * LCD_HEIGHT * 2` bytes.

### Batch Operations (Optimized)

```cpp
void write_batch_start();
```
Keeps RS high and CS low for multiple consecutive writes. Call before a burst.

```cpp
void write_batch_data(uint16_t data);
```
Sets data bus and generates WR strobe. RS and CS remain in batch state.

```cpp
void write_batch_strobe();
```
Generates a single WR strobe pulse (WR low, WR high).

```cpp
void write_batch_end();
```
Returns CS to high, ending the batch.

### Timing

```cpp
void delay_ms(uint32_t ms);
```
Blocking delay in milliseconds (wraps `usleep`).

---

## Font Rendering (font.hpp)

```cpp
void draw_char(SSD1963& lcd, uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg);
```
Renders a single 8x8 character.

```cpp
void draw_text(SSD1963& lcd, uint16_t x, uint16_t y, const char* text, uint16_t fg, uint16_t bg);
```
Renders a string with newline/tab support. Auto-wraps at LCD_WIDTH.

```cpp
void draw_text_centered(SSD1963& lcd, uint16_t y, const char* text, uint16_t fg, uint16_t bg);
```
Renders centered text at given y position.

---

## QR Code Display (qr_display.hpp)

```cpp
void draw_qr(SSD1963& lcd, const char* text,
             uint16_t x_offset = 0, uint16_t y_offset = 0,
             uint16_t fg = BLACK, uint16_t bg = WHITE);
```
Encodes `text` as a QR code (ECC MEDIUM) and renders it centered in the available area. Automatically scales module size.

```cpp
void draw_qr_with_label(SSD1963& lcd, const char* text, const char* label,
                        uint16_t fg = BLACK, uint16_t bg = WHITE);
```
Clears screen, draws a label at top, renders QR below.

---

## Menu System (menu.hpp)

```cpp
Menu(SSD1963& display);
void set_title(const char* title);
void add_item(const char* label, MenuCallback action);
void draw();
void run();
```

- `MenuCallback` is `void (*)(SSD1963&)`
- Navigation via 4 GPIO buttons: UP (GPIO 7), DOWN (GPIO 8), SELECT (GPIO 9), BACK (GPIO 10)
- Debounce: 50ms
- Max 12 items

---

## UI Components (ui.hpp)

All components are inline functions rendering to the SSD1963 display.

### ProgressBar

```cpp
void draw_progress_bar(SSD1963& lcd, uint16_t x, uint16_t y,
                       uint16_t w, uint16_t h, uint8_t progress,
                       uint16_t fg = CYAN, uint16_t bg = 0x2104);
```
Renders a horizontal progress bar. `progress` is 0-100.

```cpp
void draw_progress_bar_animated(SSD1963& lcd, uint16_t x, uint16_t y,
                                uint16_t w, uint16_t h, uint8_t progress,
                                uint16_t fg_start = RED, uint16_t fg_end = GREEN,
                                uint16_t bg = 0x2104);
```
Same as above but with color gradient from `fg_start` (0%) to `fg_end` (100%).

### CheckBox

```cpp
void draw_checkbox(SSD1963& lcd, uint16_t x, uint16_t y, bool checked,
                   const char* label = nullptr);
```
Renders a 10x10 checkbox with optional label.

### TextBox

```cpp
void draw_textbox(SSD1963& lcd, uint16_t x, uint16_t y,
                  uint16_t w, uint16_t h, const char* text,
                  uint16_t fg = WHITE, uint16_t bg = 0x2104);
```
Renders a bordered box with text, centered.

### SelectiveMenu

```cpp
void draw_selective_menu(SSD1963& lcd, uint16_t x, uint16_t y,
                         uint16_t w, uint16_t h,
                         const char* items[], uint8_t item_count,
                         uint8_t selected, uint16_t fg = WHITE,
                         uint16_t bg = BLACK, uint16_t highlight = 0xDEFB);
```
Renders a scrollable menu list with highlight. Shows position indicator (e.g., `1/5`).

---

## Colors (color.hpp)

```cpp
#define RGB565CONVERT(red, green, blue)
```

Predefined constants: `BLACK`, `BLUE`, `RED`, `GREEN`, `CYAN`, `MAGENTA`, `YELLOW`, `WHITE`

---

## Hardware Configuration (config_hw.hpp)

```cpp
// Display
LCD_WIDTH  = 480
LCD_HEIGHT = 272

// Timing
TFT_HSYNC_PULSE       = 41
TFT_HSYNC_BACK_PORCH  = 2
TFT_HSYNC_FRONT_PORCH = 2
TFT_VSYNC_PULSE       = 10
TFT_VSYNC_BACK_PORCH  = 2
TFT_VSYNC_FRONT_PORCH = 2
LCD_FPR               = 0x01E848  // ~2 MHz pixel clock

// GPIO pins
SSD1963_LCD_D0..D15   = 12..27
SSD1963_LCD_WR        = 3
SSD1963_LCD_RS        = 4
SSD1963_LCD_CS        = 5
SSD1963_LCD_RESET     = 6
SSD1963_LCD_BACKLIGHT = 0

// Button pins
BTN_UP                = 7
BTN_DOWN              = 8
BTN_SELECT            = 9
BTN_BACK              = 10
```
