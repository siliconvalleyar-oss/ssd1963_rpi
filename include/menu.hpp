#pragma once
#include <cstdint>
#include <ssd1963.hpp>
#include <font.hpp>

#define MAX_MENU_ITEMS 12
#define MENU_ITEM_H (FONT_H + 6)

typedef void (*MenuCallback)(SSD1963&);

struct MenuItem {
    const char* label;
    MenuCallback action;
};

class Menu {
public:
    Menu(SSD1963& display);
    void add_item(const char* label, MenuCallback action);
    void draw();
    void run();
    void set_title(const char* title);

private:
    SSD1963& lcd;
    MenuItem items[MAX_MENU_ITEMS];
    uint8_t item_count;
    int8_t selected;
    const char* title;
    bool exit_flag;

    void draw_item(uint8_t index, bool highlighted);
    void run_action();
};

// Built-in menu actions
void menu_color_test(SSD1963& lcd);
void menu_show_photo(SSD1963& lcd);
void menu_screen_info(SSD1963& lcd);
void menu_draw_shapes(SSD1963& lcd);
void menu_about(SSD1963& lcd);
void menu_qr_demo(SSD1963& lcd);
void menu_qr_url(SSD1963& lcd);
void menu_qr_wifi(SSD1963& lcd);
void menu_qr_text(SSD1963& lcd);
