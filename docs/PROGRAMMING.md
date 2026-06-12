# Programming Guide

## Project Structure

```
ssd1963_rpi/
├── include/
│   ├── ssd1963.hpp          # Driver class, macros, mask definitions
│   ├── ssd1963_cmd.hpp      # SSD1963 command constants
│   ├── config_hw.hpp        # Pin mappings, display dimensions, timings
│   ├── color.hpp            # RGB565 color definitions
│   ├── font.hpp             # 8×8 bitmap font + text rendering functions
│   ├── icons.hpp            # 16×16 icon bitmaps
│   ├── ui.hpp               # UI widgets (ProgressBar, CheckBox, TextBox, SelectiveMenu)
│   ├── menu.hpp             # Menu class + callback declarations
│   ├── qrcodegen.hpp        # QR Code generator (Nayuki library)
│   └── qr_display.hpp       # QR rendering helpers
├── src/
│   ├── ssd1963.cpp          # Driver implementation
│   ├── menu.cpp             # Menu + built-in actions
│   ├── qrcodegen.cpp        # QR Code generator implementation
│   └── main.cpp             # Entry point
├── assets/
│   ├── capibaras.png        # Source image
│   └── capibaras.rgb565     # Pre-converted RGB565 binary
├── script_tools_ssd1963_rpi/
│   ├── convert_png_to_rgb.py
│   ├── convert_png_to_rgb.sh
│   └── install_deps.sh
├── docs/                    # Documentation
├── Makefile
└── README.md
```

## Build System

### Prerequisites

- Raspberry Pi (Zero 2W, 3, 4, or 5)
- `libbcm2835` installed (see `script_tools_ssd1963_rpi/install_deps.sh`)
- g++ with C++17 support

### Building

```bash
make          # Build the binary (bin/ssd1963_app)
make run      # Build and run (requires sudo)
make clean    # Remove build artifacts
```

### Remote Development

```bash
PI_HOST=pi@192.168.1.100
PI_DIR=/home/pi/src/ssd1963_photo_png_2026_sucess_rgb

make remote-build   # SSH and compile on Pi
make remote-run     # SSH, compile, and run on Pi
```

## Quick Start

### Basic Setup

```cpp
#include <ssd1963.hpp>
#include <bcm2835.h>

int main() {
    SSD1963 lcd;

    if (!bcm2835_init()) return 1;

    lcd.setup_gpio();
    lcd.init();
    lcd.clear_screen(BLACK);

    // Your code here

    bcm2835_close();
    return 0;
}
```

### Drawing Text

```cpp
#include <font.hpp>

draw_text(lcd, 10, 10, "Hello World!", WHITE, BLACK);
draw_text_centered(lcd, 100, "Centered Text", CYAN, BLACK);
```

### Drawing Shapes

```cpp
lcd.draw_block(50, 50, 100, 80, BLUE);         // Filled rectangle
lcd.draw_rect(50, 50, 100, 80, RED);            // Outline rectangle
lcd.draw_line_h(10, 200, 200, GREEN);           // Horizontal line
lcd.draw_line_v(300, 50, 100, YELLOW);          // Vertical line
lcd.draw_pixel(150, 136, WHITE);                // Single pixel
```

### Displaying an Image

Convert PNG to RGB565:
```bash
./script_tools_ssd1963_rpi/convert_png_to_rgb.sh assets/myimage.png
```

Display it:
```cpp
lcd.draw_image_rgb565("assets/myimage.rgb565");
```

### Using the Menu

```cpp
#include <menu.hpp>

void my_action(SSD1963& lcd) {
    lcd.clear_screen(BLACK);
    draw_text_centered(lcd, LCD_HEIGHT/2, "Action!", WHITE, BLACK);
    usleep(2000000);
}

int main() {
    // ... init ...

    Menu menu(lcd);
    menu.set_title("My App");
    menu.add_item("Do Something", my_action);
    menu.add_item("Exit", [](SSD1963&){});
    menu.run();

    // ... cleanup ...
}
```

### Generating a QR Code

```cpp
#include <qr_display.hpp>

// Simple usage — centers QR on screen
draw_qr(lcd, "https://example.com");

// With label
draw_qr_with_label(lcd, "Hello QR!", "My Label");

// Custom position and colors
draw_qr(lcd, "data", 20, 40, BLUE, WHITE);
```

### UI Widgets

```cpp
#include <ui.hpp>

// Progress bar (0-100)
draw_progress_bar(lcd, 10, 10, 200, 20, 75, CYAN, 0x2104);

// Animated progress bar (color gradient)
draw_progress_bar_animated(lcd, 10, 40, 200, 20, 75, RED, GREEN, 0x2104);

// Checkbox
draw_checkbox(lcd, 10, 70, true, "Enable Feature");

// Text box
draw_textbox(lcd, 10, 100, 200, 40, "System Ready", WHITE, 0x2104);

// Selective menu
const char* items[] = {"WiFi", "Bluetooth", "Display", "Audio"};
draw_selective_menu(lcd, 10, 150, 200, 100, items, 4, 1);
```

## Image Conversion

To display images, convert them to raw RGB565 format:

```bash
# Using Python script
python3 script_tools_ssd1963_rpi/convert_png_to_rgb.py assets/image.png

# Or using shell script (if ImageMagick is installed)
./script_tools_ssd1963_rpi/convert_png_to_rgb.sh assets/image.png
```

The output is a `.rgb565` file containing `480 × 272 × 2 = 261,120` bytes of raw pixel data.

## Adding New Features

1. **New drawing primitive** — Add method to `SSD1963` class in `ssd1963.hpp`/`ssd1963.cpp`
2. **New UI widget** — Add inline function in `ui.hpp`
3. **New menu action** — Add callback function in `menu.cpp`, declare in `menu.hpp`, register in `main.cpp`
4. **New font** — Add bitmap data to `FONT8x8` array in `font.hpp`
5. **New icon** — Add bitmap data to icons namespace in `icons.hpp`

## Troubleshooting

### Build Errors

| Error                          | Fix                                  |
|--------------------------------|--------------------------------------|
| `bcm2835.h: No such file`      | Install libbcm2835 (see install_deps)|
| `undefined reference to bcm2835_*` | Add `-lbcm2835` to LDFLAGS      |
| `Permission denied` on run     | Run with `sudo`                      |

### Runtime Errors

| Error                          | Fix                                  |
|--------------------------------|--------------------------------------|
| `bcm2835_init() failed`        | Run with `sudo` or check GPIO access |
| Image file not found           | Run from project root directory      |
| Wrong colors                   | Check RGB565 byte order (big-endian) |
| Display flickers               | Adjust `LCD_FPR` in `config_hw.hpp`  |
