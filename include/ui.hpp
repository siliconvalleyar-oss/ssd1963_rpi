#pragma once
#include <ssd1963.hpp>
#include <font.hpp>
#include <icons.hpp>
#include <color.hpp>
#include <cstring>
#include <cstdio>

// Subtle UI colors
#define UI_HIGHLIGHT  0xDEFB
#define UI_DIM_BG     0x2108
#define UI_BORDER     0x4208
#define UI_ACCENT     CYAN
#define UI_TEXT       WHITE
#define UI_TEXT_DIM   0x8C71

// ─── ProgressBar ───────────────────────────────────────────────
// Smooth, animated bar with gradient-like fill and percentage.

struct ProgressBar {
    uint16_t x, y, w, h;
    uint16_t fg, bg, border;
    float value;
    const char* label;

    ProgressBar(uint16_t x0, uint16_t y0, uint16_t w0, uint16_t h0,
                uint16_t fg0 = UI_ACCENT, uint16_t bg0 = UI_DIM_BG,
                uint16_t border0 = UI_BORDER, const char* lbl = nullptr)
        : x(x0), y(y0), w(w0), h(h0), fg(fg0), bg(bg0), border(border0),
          value(0.0f), label(lbl) {}

    void draw(SSD1963& lcd) const {
        if (label) {
            draw_text(lcd, x, y - FONT_H - 4, label, UI_TEXT, BLACK);
        }
        lcd.draw_rect(x, y, w, h, border);
        uint16_t fill_w = (uint16_t)((w - 2) * value);
        if (fill_w > 0) {
            lcd.draw_block(x + 1, y + 1, fill_w, h - 2, fg);
            if (fill_w > w / 2) {
                lcd.draw_block(x + 1, y + 1, fill_w - (w >> 3), h - 2, fg + 0x0820);
            }
        }
        if (fill_w < w - 2) {
            lcd.draw_block(x + 1 + fill_w, y + 1, w - 2 - fill_w, h - 2, bg);
        }
        char pct[8];
        snprintf(pct, sizeof(pct), "%d%%", (int)(value * 100));
        draw_text(lcd, x + w + 6, y + 1, pct, UI_TEXT_DIM, BLACK);
    }

    void animate_to(SSD1963& lcd, float target, uint32_t steps = 16, uint32_t step_ms = 40) {
        float start = value;
        float inc = (target - start) / steps;
        for (uint32_t i = 1; i <= steps; i++) {
            value = start + inc * i;
            draw(lcd);
            lcd.delay_ms(step_ms);
        }
        value = target;
        draw(lcd);
    }

    void draw_indeterminate(SSD1963& lcd, uint8_t phase) {
        uint16_t bar_w = w >> 3;
        uint16_t pos = (uint16_t)((float)(w - bar_w) * ((sin_lookup(phase * 16) + 1.0f) / 2.0f));
        lcd.draw_rect(x, y, w, h, border);
        lcd.draw_block(x + 1, y + 1, w - 2, h - 2, bg);
        lcd.draw_block(x + 1 + pos, y + 1, bar_w, h - 2, fg);
    }

private:
    static float sin_lookup(int angle) {
        static const int8_t sin_tab[64] = {
            0,3,6,9,12,15,18,21,24,27,30,33,36,38,41,43,
            46,48,50,52,54,56,57,59,60,61,62,63,63,64,64,64,
            64,64,64,63,63,62,61,60,59,57,56,54,52,50,48,46,
            43,41,38,36,33,30,27,24,21,18,15,12,9,6,3,0
        };
        return sin_tab[angle & 63] / 64.0f;
    }
};

// ─── CheckBox ──────────────────────────────────────────────────

struct CheckBox {
    uint16_t x, y;
    bool checked;
    const char* label;
    static constexpr uint16_t BOX = 12;

    CheckBox(uint16_t x0, uint16_t y0, bool chk = false, const char* lbl = nullptr)
        : x(x0), y(y0), checked(chk), label(lbl) {}

    void draw(SSD1963& lcd) const {
        lcd.draw_block(x, y, BOX, BOX, checked ? UI_ACCENT : 0x0842);
        lcd.draw_rect(x, y, BOX, BOX, UI_BORDER);
        if (checked) {
            // check mark in accent color
            for (int i = 0; i < 4; i++) {
                lcd.draw_pixel(x + 3 + i, y + 5 + i, BLACK);
                lcd.draw_pixel(x + 3 + i, y + 6 + i, BLACK);
            }
            for (int i = 0; i < 5; i++) {
                lcd.draw_pixel(x + 6 + i, y + 2 + i, BLACK);
                lcd.draw_pixel(x + 6 + i, y + 3 + i, BLACK);
            }
        }
        if (label) {
            draw_text(lcd, x + BOX + 6, y - 1, label, UI_TEXT, BLACK);
        }
    }

    void toggle(SSD1963& lcd) {
        checked = !checked;
        draw(lcd);
    }
};

// ─── TextBox ───────────────────────────────────────────────────

struct TextBox {
    uint16_t x, y, w, h;
    uint16_t fg, bg, border;
    char buffer[512];
    uint16_t scroll_offset;

    TextBox(uint16_t x0, uint16_t y0, uint16_t w0, uint16_t h0,
            uint16_t fg0 = UI_TEXT, uint16_t bg0 = BLACK, uint16_t border0 = UI_BORDER)
        : x(x0), y(y0), w(w0), h(h0), fg(fg0), bg(bg0), border(border0),
          scroll_offset(0) { buffer[0] = '\0'; }

    void set_text(const char* text) {
        strncpy(buffer, text, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
    }

    void append(const char* text) {
        size_t len = strlen(buffer);
        strncat(buffer, text, sizeof(buffer) - len - 1);
    }

    void clear() { buffer[0] = '\0'; }

    void draw(SSD1963& lcd) const {
        lcd.draw_block(x, y, w, h, bg);
        lcd.draw_rect(x, y, w, h, border);
        char line_buf[LCD_WIDTH / (FONT_W + 1) + 1];
        uint16_t line_y = y + 4;
        uint16_t max_y = y + h - 4;
        const char* p = buffer;

        while (*p && line_y + FONT_H <= max_y) {
            uint16_t i = 0;
            while (*p && *p != '\n' && i < sizeof(line_buf) - 2) {
                line_buf[i++] = *p++;
            }
            line_buf[i] = '\0';
            if (*p == '\n') p++;
            draw_text(lcd, x + 4, line_y, line_buf, fg, bg);
            line_y += FONT_H + 3;
        }
        if (*p) {
            draw_text(lcd, x + w - 24, y + h - FONT_H - 2, "...", UI_ACCENT, bg);
        }
    }
};

// ─── SelectiveMenu (minimalist, dynamic, with subtle effects) ──

struct SelectiveMenu {
    static constexpr uint8_t MAX_ITEMS = 30;
    static constexpr uint8_t ITEM_H = FONT_H + 6;
    static constexpr uint8_t ICON_OFFSET = 20;

    struct Item {
        const char* label;
        void (*action)(SSD1963&);
        const uint8_t* icon;
        bool checked;
        bool is_checkbox;
        bool visible;
        uint16_t accent;

        Item() : label(nullptr), action(nullptr), icon(nullptr),
                 checked(false), is_checkbox(false), visible(true), accent(UI_HIGHLIGHT) {}
    };

    Item items[MAX_ITEMS];
    uint8_t count;
    int16_t selected;
    uint16_t scroll_top;
    uint16_t visible_rows;
    const char* title;
    SSD1963* lcd;
    bool exit_flag;
    bool dirty;

    // Effect state
    int16_t prev_selected;
    uint16_t anim_frame;

    SelectiveMenu() : count(0), selected(0), scroll_top(0),
                      visible_rows(0), title(nullptr), lcd(nullptr),
                      exit_flag(false), dirty(true), prev_selected(-1), anim_frame(0) {}

    void attach(SSD1963& display) { lcd = &display; }
    void set_title(const char* t) { title = t; }

    void add_item(const char* label, void (*action)(SSD1963&) = nullptr,
                  const uint8_t* icon = nullptr, bool checkbox = false, bool chk = false) {
        if (count >= MAX_ITEMS) return;
        items[count].label = label;
        items[count].action = action;
        items[count].icon = icon;
        items[count].is_checkbox = checkbox;
        items[count].checked = chk;
        items[count].accent = UI_HIGHLIGHT;
        count++;
        dirty = true;
    }

    void remove_all() {
        count = 0;
        selected = 0;
        scroll_top = 0;
        dirty = true;
    }

    void draw() {
        if (!lcd || !dirty) return;
        lcd->clear_screen(BLACK);

        uint16_t y_off = 0;
        if (title) {
            draw_text_centered(*lcd, 3, title, UI_TEXT, BLACK);
            lcd->draw_line_h(2, FONT_H + 5, LCD_WIDTH - 4, UI_BORDER);
            y_off = FONT_H + 9;
        }

        uint16_t avail_h = LCD_HEIGHT - y_off - 2;
        visible_rows = (avail_h - (FONT_H + 2)) / ITEM_H;

        // Clamp
        if (selected < 0) selected = 0;
        if (selected >= count) selected = count - 1;

        // Auto-scroll to keep selection visible
        if (selected < (int16_t)scroll_top)
            scroll_top = selected;
        if (selected >= (int16_t)(scroll_top + visible_rows))
            scroll_top = selected - visible_rows + 1;

        if (visible_rows > count) visible_rows = count;

        // Draw footer indicator
        if (count > visible_rows) {
            char footer[16];
            snprintf(footer, sizeof(footer), "%d/%d", selected + 1, count);
            draw_text_centered(*lcd, LCD_HEIGHT - FONT_H - 2, footer, UI_TEXT_DIM, BLACK);
        }

        for (uint8_t i = 0; i < visible_rows; i++) {
            uint8_t idx = scroll_top + i;
            if (idx >= count) break;

            if (!items[idx].visible) continue;

            bool is_sel = (idx == (uint8_t)selected);
            uint16_t iy = y_off + i * ITEM_H;

            // Background: subtle highlight for selected item
            if (is_sel) {
                lcd->draw_block(2, iy, LCD_WIDTH - 4, ITEM_H, items[idx].accent);
                lcd->draw_line_h(2, iy, LCD_WIDTH - 4, items[idx].accent + 0x0821);
                lcd->draw_line_h(2, iy + ITEM_H - 1, LCD_WIDTH - 4, items[idx].accent - 0x0821);
            } else {
                lcd->draw_block(2, iy, LCD_WIDTH - 4, ITEM_H, BLACK);
            }

            uint16_t tx = 6;

            // Icon or checkbox
            if (items[idx].is_checkbox) {
                CheckBox cb(tx, iy + 2, items[idx].checked);
                cb.draw(*lcd);
                tx += CheckBox::BOX + 8;
            } else if (items[idx].icon) {
                draw_icon(*lcd, tx, iy + (ITEM_H - ICON_H) / 2, items[idx].icon,
                         is_sel ? BLACK : UI_TEXT, is_sel ? items[idx].accent : BLACK);
                tx += ICON_W + 6;
            }

            // Label
            uint16_t text_fg = is_sel ? BLACK : UI_TEXT;
            draw_text(*lcd, tx, iy + 2, items[idx].label, text_fg, is_sel ? items[idx].accent : BLACK);

            // Arrow indicator for actionable items
            if (items[idx].action && !items[idx].is_checkbox) {
                draw_text(*lcd, LCD_WIDTH - 18, iy + 2, "\x10", text_fg, is_sel ? items[idx].accent : BLACK);
            }
        }

        dirty = false;
        prev_selected = selected;
    }

    void select_next() {
        if (selected < count - 1) {
            prev_selected = selected;
            selected++;
            dirty = true;
        }
    }

    void select_prev() {
        if (selected > 0) {
            prev_selected = selected;
            selected--;
            dirty = true;
        }
    }

    void toggle_current() {
        if (selected >= 0 && selected < count && items[selected].is_checkbox) {
            items[selected].checked = !items[selected].checked;
            dirty = true;
        }
    }

    void activate_current() {
        if (selected >= 0 && selected < count && items[selected].action && !items[selected].is_checkbox) {
            dirty = true;
            lcd->clear_screen(BLACK);
            items[selected].action(*lcd);
            dirty = true;
            draw();
        }
    }

    void run() {
        if (!lcd) return;
        exit_flag = false;
        dirty = true;
        draw();

        while (!exit_flag) {
            usleep(20000);
            anim_frame++;
        }
    }
};
