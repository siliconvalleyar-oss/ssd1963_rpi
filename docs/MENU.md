# Menu System & UI Components

## Architecture

The menu system is built around 4 physical buttons (UP, DOWN, SELECT, BACK) connected to GPIO pins. The `Menu` class manages a list of labeled callbacks, renders them on screen, and dispatches user input.

```
┌─────────────────────────────────┐
│  SSD1963 DEMO v1.0              │  ← title
├─────────────────────────────────┤
│ → 1. Color Test                 │  ← selected (highlighted)
│   2. Show Photo                 │
│   3. Screen Info                │
│   4. Draw Shapes                │
│   5. QR: GitHub URL             │
│   6. QR: WiFi Config            │
│   7. QR: Custom Text            │
│   8. About                      │
│   9. Exit                       │
├─────────────────────────────────┤
│  ▲▼=Sel  OK=Enter  ◄=Back       │  ← footer
└─────────────────────────────────┘
```

## Button Mapping

| Button    | GPIO | Action                    |
|-----------|------|---------------------------|
| UP (▲)   | 7    | Select previous item      |
| DOWN (▼) | 8    | Select next item          |
| OK       | 9    | Execute selected item     |
| BACK (◄) | 10   | Return to menu / Exit app |

Debounce: 50ms (configurable via `DEBOUNCE_US`)

## Menu Class

```cpp
#include <menu.hpp>

Menu menu(lcd);
menu.set_title("My App");
menu.add_item("Option 1", callback_function);
menu.add_item("Option 2", [](SSD1963& lcd) {
    // inline action
});
menu.run();
```

### Callbacks

Each menu item has a `MenuCallback`:
```cpp
typedef void (*MenuCallback)(SSD1963&);
```

Built-in callbacks:
- `menu_color_test` — displays color swatches
- `menu_show_photo` — shows RGB565 image from file
- `menu_screen_info` — displays driver/display info
- `menu_draw_shapes` — draws geometric primitives
- `menu_qr_url` / `menu_qr_wifi` / `menu_qr_text` — QR code demos
- `menu_about` — version/about screen

### Navigation

- The menu runs in a blocking loop (`menu.run()`)
- BACK button exits the loop and returns control to `main()`

## UI Components (ui.hpp)

### ProgressBar

```cpp
draw_progress_bar(lcd, x, y, w, h, progress, fg, bg);
draw_progress_bar_animated(lcd, x, y, w, h, progress, fg_start, fg_end, bg);
```

- `w` x `h`: dimensions in pixels
- `progress`: 0–100
- Animated version interpolates color from `fg_start` (0%) to `fg_end` (100%)
- Border drawn around the bar

### CheckBox

```cpp
draw_checkbox(lcd, x, y, checked, "label");
```

- 10×10 px box
- Filled with checkmark when `checked = true`
- Optional text label to the right

### TextBox

```cpp
draw_textbox(lcd, x, y, w, h, "text", fg, bg);
```

- Bordered box with centered text
- Useful for dialogs, notifications, status messages

### SelectiveMenu

```cpp
const char* items[] = {"Option A", "Option B", "Option C"};
draw_selective_menu(lcd, x, y, w, h, items, 3, selected_index);
```

- Renders a vertical list with highlight on selected item
- Subtle highlight: light gray background (`0xDEFB`) instead of full inversion
- Shows footer with position indicator (e.g., `1/3`)

---

## QR Code Display

3 QR demos accessible from the menu:

| Menu Item            | Content                                        |
|----------------------|------------------------------------------------|
| QR: GitHub URL       | `https://github.com/opencode-ai/opencode`     |
| QR: WiFi Config      | `WIFI:S:MyNetwork;T:WPA;P:MyPassword123;;`    |
| QR: Custom Text      | "Hello from SSD1963!"                          |

Each renders a QR code centered on screen using the Nayuki QR Code Generator library (ECC MEDIUM, version auto-selected).

---

## Adding a New Menu Item

```cpp
// 1. Declare callback (in menu.hpp or your file)
void my_custom_action(SSD1963& lcd);

// 2. Implement it
void my_custom_action(SSD1963& lcd) {
    lcd.clear_screen(BLACK);
    draw_text_centered(lcd, LCD_HEIGHT/2, "Hello!", WHITE, BLACK);
    usleep(2000000);
}

// 3. Register in main()
menu.add_item("My Action", my_custom_action);
```

**Important:** The callback must not block indefinitely. It should return after a few seconds or when user input is detected. The menu re-draws automatically after the callback returns.
