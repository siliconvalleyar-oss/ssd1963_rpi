#include <menu.hpp>
#include <color.hpp>
#include <config_hw.hpp>
#include <qr_display.hpp>
#include <bcm2835.h>
#include <unistd.h>
#include <cstdio>

#define BTN_UP      7
#define BTN_DOWN    8
#define BTN_SELECT  9
#define BTN_BACK    10

#define DEBOUNCE_US 50000

static int read_button(uint8_t gpio) {
    return bcm2835_gpio_lev(gpio) == LOW;
}

Menu::Menu(SSD1963& display)
    : lcd(display), item_count(0), selected(0), title(nullptr), exit_flag(false) {}

void Menu::set_title(const char* t) {
    title = t;
}

void Menu::add_item(const char* label, MenuCallback action) {
    if (item_count < MAX_MENU_ITEMS) {
        items[item_count].label = label;
        items[item_count].action = action;
        item_count++;
    }
}

void Menu::draw_item(uint8_t index, bool highlighted) {
    uint16_t y = (title ? FONT_H + 12 : 6) + index * MENU_ITEM_H;
    uint16_t fg = highlighted ? BLACK : WHITE;
    uint16_t bg = highlighted ? WHITE : BLACK;

    lcd.draw_block(0, y, LCD_WIDTH, MENU_ITEM_H - 2, bg);
    draw_text(lcd, 12, y + 2, items[index].label, fg, bg);
}

void Menu::draw() {
    lcd.clear_screen(BLACK);

    if (title) {
        draw_text_centered(lcd, 4, title, WHITE, BLACK);
        lcd.draw_block(0, FONT_H + 8, LCD_WIDTH, 1, WHITE);
    }

    for (uint8_t i = 0; i < item_count; i++) {
        draw_item(i, i == selected);
    }

    draw_text(lcd, 2, LCD_HEIGHT - FONT_H - 2, "\x18\x19=Sel  OK=Enter  \x1a=Back", WHITE, BLACK);
}

void Menu::run_action() {
    if (selected >= 0 && selected < item_count && items[selected].action) {
        lcd.clear_screen(BLACK);
        items[selected].action(lcd);
        usleep(300000);
        draw();
        bcm2835_gpio_write(SSD1963_LCD_BACKLIGHT, HIGH);
    }
}

void Menu::run() {
    bcm2835_gpio_fsel(BTN_UP, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(BTN_UP, BCM2835_GPIO_PUD_UP);
    bcm2835_gpio_fsel(BTN_DOWN, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(BTN_DOWN, BCM2835_GPIO_PUD_UP);
    bcm2835_gpio_fsel(BTN_SELECT, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(BTN_SELECT, BCM2835_GPIO_PUD_UP);
    bcm2835_gpio_fsel(BTN_BACK, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(BTN_BACK, BCM2835_GPIO_PUD_UP);

    draw();
    exit_flag = false;

    while (!exit_flag) {
        bool pressed = false;

        if (read_button(BTN_UP)) {
            if (selected > 0) {
                draw_item(selected, false);
                selected--;
                draw_item(selected, true);
            }
            pressed = true;
        } else if (read_button(BTN_DOWN)) {
            if (selected < item_count - 1) {
                draw_item(selected, false);
                selected++;
                draw_item(selected, true);
            }
            pressed = true;
        } else if (read_button(BTN_SELECT)) {
            run_action();
            pressed = true;
        } else if (read_button(BTN_BACK)) {
            exit_flag = true;
            pressed = true;
        }

        if (pressed) {
            usleep(DEBOUNCE_US);
            while (read_button(BTN_UP) || read_button(BTN_DOWN) ||
                   read_button(BTN_SELECT) || read_button(BTN_BACK)) {
                usleep(10000);
            }
            usleep(DEBOUNCE_US);
        }

        usleep(10000);
    }
}

// === Built-in actions ===

void menu_color_test(SSD1963& lcd) {
    const uint16_t w = 60, h = 60, gap = 8;
    const uint16_t colors[] = {RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE};
    const char* names[] = {"RED", "GREEN", "BLUE", "YELLOW", "CYAN", "MAGENTA", "WHITE"};
    uint16_t x = (LCD_WIDTH - (7 * w + 6 * gap)) / 2;
    uint16_t y = (LCD_HEIGHT - h) / 2 - 16;

    draw_text_centered(lcd, y - FONT_H - 4, "COLOR TEST", WHITE, BLACK);

    for (int i = 0; i < 7; i++) {
        lcd.draw_block(x, y, w, h, colors[i]);
        draw_text_centered(lcd, y + h + 4, names[i], WHITE, BLACK);
        x += w + gap;
    }

    usleep(3000000);
}

void menu_show_photo(SSD1963& lcd) {
    draw_text_centered(lcd, LCD_HEIGHT / 2 - FONT_H / 2, "Cargando imagen...", WHITE, BLACK);
    lcd.draw_image_rgb565("assets/capibaras.rgb565");
    usleep(4000000);
}

void menu_screen_info(SSD1963& lcd) {
    char buf[64];
    snprintf(buf, sizeof(buf), "Res: %dx%d", LCD_WIDTH, LCD_HEIGHT);
    draw_text(lcd, 20, 30, buf, WHITE, BLACK);
    draw_text(lcd, 20, 50, "Color: RGB565 16-bit", WHITE, BLACK);
    draw_text(lcd, 20, 70, "Ctrl: SSD1963", WHITE, BLACK);
    draw_text(lcd, 20, 90, "IF: 16-bit 8080 parallel", WHITE, BLACK);
    draw_text(lcd, 20, 110, "GPIO: libbcm2835", WHITE, BLACK);
    draw_text(lcd, 20, 130, "FPS: ~30 (GPIO bitbang)", WHITE, BLACK);
    usleep(4000000);
}

void menu_draw_shapes(SSD1963& lcd) {
    lcd.clear_screen(BLACK);

    // Filled blocks
    lcd.draw_block(10, 10, 60, 60, RED);
    lcd.draw_block(80, 10, 60, 60, GREEN);
    lcd.draw_block(150, 10, 60, 60, BLUE);

    // Individual pixels forming lines
    for (uint16_t i = 0; i < 100; i++) {
        lcd.draw_pixel(10 + i, 120, WHITE);
        lcd.draw_pixel(10 + i, 200, WHITE);
    }
    // Vertical lines
    for (uint16_t i = 0; i < 80; i++) {
        lcd.draw_pixel(10, 120 + i, WHITE);
        lcd.draw_pixel(110, 120 + i, WHITE);
    }

    // Diagonal
    for (uint16_t i = 0; i < 80; i++) {
        lcd.draw_pixel(200 + i, 120 + i, CYAN);
    }

    // Checkerboard pattern
    for (uint16_t cy = 0; cy < 4; cy++) {
        for (uint16_t cx = 0; cx < 6; cx++) {
            uint16_t color = ((cx + cy) % 2 == 0) ? WHITE : BLACK;
            lcd.draw_block(340 + cx * 20, 20 + cy * 20, 20, 20, color);
        }
    }

    usleep(4000000);
}

void menu_about(SSD1963& lcd) {
    draw_text_centered(lcd, 30, "SSD1963 Display", WHITE, BLACK);
    draw_text_centered(lcd, 54, "Raspberry Pi Driver", WHITE, BLACK);
    draw_text(lcd, 20, 90, "Controller: SSD1963", WHITE, BLACK);
    draw_text(lcd, 20, 110, "Resolution: 480x272", WHITE, BLACK);
    draw_text(lcd, 20, 130, "Interface: 16-bit 8080", WHITE, BLACK);
    draw_text(lcd, 20, 150, "Library: libbcm2835", WHITE, BLACK);
    draw_text(lcd, 20, 170, "Language: C++17", WHITE, BLACK);
    draw_text(lcd, 20, 200, "Menu System v1.0", WHITE, BLACK);
    usleep(4000000);
}

static void show_qr_result(SSD1963& lcd, const char* text, const char* label) {
    lcd.clear_screen(WHITE);

    uint8_t label_h = (label) ? FONT_H + 6 : 0;
    if (label) {
        draw_text_centered(lcd, 4, label, BLACK, WHITE);
        lcd.draw_block(0, FONT_H + 4, LCD_WIDTH, 1, BLACK);
    }

    int qr_area_w = LCD_WIDTH - 40;
    int qr_area_h = LCD_HEIGHT - label_h - (FONT_H + 8) - 40;
    int qr_area = std::min(qr_area_w, qr_area_h);
    int qr_size_max = qr_area;

    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(text, qrcodegen::QrCode::Ecc::MEDIUM);
    int sz = qr.getSize();
    int module_size = qr_size_max / sz;
    if (module_size < 1) module_size = 1;
    int total = sz * module_size;
    int start_x = (LCD_WIDTH - total) / 2;
    int start_y = label_h + 10 + (qr_area_h - total) / 2;

    for (int qy = 0; qy < sz; qy++) {
        for (int qx = 0; qx < sz; qx++) {
            bool dark = qr.getModule(qx, qy);
            lcd.draw_block(start_x + qx * module_size, start_y + qy * module_size,
                           module_size, module_size, dark ? BLACK : WHITE);
        }
    }

    char info[64];
    snprintf(info, sizeof(info), "\"%s\"  (%dx%d)", text, sz, sz);
    draw_text_centered(lcd, LCD_HEIGHT - FONT_H - 8, info, BLACK, WHITE);
    usleep(5000000);
}

void menu_qr_url(SSD1963& lcd) {
    show_qr_result(lcd, "https://github.com/opencode-ai/opencode", "QR: GitHub URL");
}

void menu_qr_wifi(SSD1963& lcd) {
    show_qr_result(lcd, "WIFI:S:MyNetwork;T:WPA;P:MyPassword123;;", "QR: WiFi Config");
}

void menu_qr_text(SSD1963& lcd) {
    show_qr_result(lcd, "Hello from SSD1963!\nRaspberry Pi QR Demo", "QR: Custom Text");
}

void menu_qr_demo(SSD1963& lcd) {
    menu_qr_url(lcd);
    menu_qr_wifi(lcd);
    menu_qr_text(lcd);
}
