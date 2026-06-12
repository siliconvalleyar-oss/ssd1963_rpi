#pragma once
#include <algorithm>
#include <cstdio>
#include <ssd1963.hpp>
#include <qrcodegen.hpp>
#include <color.hpp>
#include <font.hpp>

inline void draw_qr(SSD1963& lcd, const char* text,
                    uint16_t x_offset = 0, uint16_t y_offset = 0,
                    uint16_t fg = BLACK, uint16_t bg = WHITE)
{
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(text, qrcodegen::QrCode::Ecc::MEDIUM);
    int qr_size = qr.getSize();

    int available = (x_offset == 0 && y_offset == 0)
        ? std::min(LCD_WIDTH, LCD_HEIGHT) - 20
        : std::min(LCD_WIDTH - x_offset, LCD_HEIGHT - y_offset) - 4;
    int module_size = available / qr_size;
    if (module_size < 1) module_size = 1;

    int total_px = qr_size * module_size;
    int start_x = x_offset + (LCD_WIDTH - x_offset - total_px) / 2;
    int start_y = y_offset + (LCD_HEIGHT - y_offset - total_px) / 2;

    for (int qy = 0; qy < qr_size; qy++) {
        for (int qx = 0; qx < qr_size; qx++) {
            bool dark = qr.getModule(qx, qy);
            if (module_size == 1) {
                lcd.draw_pixel(start_x + qx, start_y + qy, dark ? fg : bg);
            } else {
                lcd.draw_block(start_x + qx * module_size, start_y + qy * module_size,
                               module_size, module_size, dark ? fg : bg);
            }
        }
    }
}

inline void draw_qr_with_label(SSD1963& lcd, const char* text, const char* label,
                                uint16_t fg = BLACK, uint16_t bg = WHITE)
{
    lcd.clear_screen(bg);

    if (label) {
        draw_text_centered(lcd, 4, label, fg, bg);
        draw_qr(lcd, text, 0, FONT_H + 8, fg, bg);
    } else {
        draw_qr(lcd, text, 0, 0, fg, bg);
    }

    char info[32];
    snprintf(info, sizeof(info), "Press OK to return");
    draw_text(lcd, 2, LCD_HEIGHT - FONT_H - 2, info, fg, bg);
}
