#include <FrameBuffer.hpp>
#include <font_8x8.hpp>
#include <engine/Sprite.hpp>
#include <cstring>
#include <cstdio>
#include <algorithm>

FrameBuffer::FrameBuffer() {
    m_data = new uint16_t[LCD_WIDTH * LCD_HEIGHT];
    clear(BLACK);
}

FrameBuffer::~FrameBuffer() {
    delete[] m_data;
}

void FrameBuffer::clear(uint16_t color) {
    uint32_t n = (uint32_t)LCD_WIDTH * LCD_HEIGHT;
    for (uint32_t i = 0; i < n; i++) {
        m_data[i] = color;
    }
}

void FrameBuffer::set_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    m_data[y * LCD_WIDTH + x] = color;
}

void FrameBuffer::fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + w > LCD_WIDTH)  w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;

    for (uint16_t row = 0; row < h; row++) {
        uint32_t offset = (y + row) * LCD_WIDTH + x;
        for (uint16_t col = 0; col < w; col++) {
            m_data[offset + col] = color;
        }
    }
}

void FrameBuffer::draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    fill_rect(x, y, w, 1, color);
    fill_rect(x, y + h - 1, w, 1, color);
    fill_rect(x, y, 1, h, color);
    fill_rect(x + w - 1, y, 1, h, color);
}

void FrameBuffer::draw_sprite(int16_t x, int16_t y, const Sprite& sprite) {
    uint16_t sw = sprite.width();
    uint16_t sh = sprite.height();
    const uint16_t* src = sprite.data();

    if (!src) return;

    for (uint16_t row = 0; row < sh; row++) {
        int16_t py = y + row;
        if (py < 0 || py >= (int16_t)LCD_HEIGHT) continue;

        for (uint16_t col = 0; col < sw; col++) {
            int16_t px = x + col;
            if (px < 0 || px >= (int16_t)LCD_WIDTH) continue;
            uint16_t c = src[row * sw + col];
            if (c != 0x0001) {
                m_data[py * LCD_WIDTH + px] = c;
            }
        }
    }
}

void FrameBuffer::draw_buffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT || !data) return;
    if (x + w > LCD_WIDTH)  w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;

    for (uint16_t row = 0; row < h; row++) {
        std::memcpy(
            m_data + (y + row) * LCD_WIDTH + x,
            data + row * w,
            w * sizeof(uint16_t));
    }
}

void FrameBuffer::draw_char(uint16_t x, uint16_t y, char chr, uint16_t color, uint16_t bg) {
    if (chr < 0x20 || chr > 0x7E) chr = 0x20;
    uint8_t idx = chr - 0x20;

    if (bg == 0xFFFF) {
        for (uint8_t row = 0; row < FONT_CHAR_HEIGHT; row++) {
            uint8_t bits = FONT_8X8[idx][row];
            for (uint8_t col = 0; col < FONT_CHAR_WIDTH; col++) {
                if (bits & (1 << (7 - col)))
                    set_pixel(x + col, y + row, color);
            }
        }
    } else {
        uint16_t buf[FONT_CHAR_WIDTH * FONT_CHAR_HEIGHT];
        for (uint8_t row = 0; row < FONT_CHAR_HEIGHT; row++) {
            uint8_t bits = FONT_8X8[idx][row];
            for (uint8_t col = 0; col < FONT_CHAR_WIDTH; col++) {
                buf[row * FONT_CHAR_WIDTH + col] =
                    (bits & (1 << (7 - col))) ? color : bg;
            }
        }
        draw_buffer(x, y, FONT_CHAR_WIDTH, FONT_CHAR_HEIGHT, buf);
    }
}

void FrameBuffer::draw_string(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg) {
    uint16_t cursor_x = x;
    while (*str) {
        if (*str == '\n') {
            cursor_x = x;
            y += FONT_CHAR_HEIGHT + 2;
        } else {
            draw_char(cursor_x, y, *str, color, bg);
            cursor_x += FONT_CHAR_WIDTH + 1;
        }
        str++;
    }
}

void FrameBuffer::draw_string_centered(uint16_t center_x, uint16_t y, const char* str,
                                        uint16_t color, uint16_t bg) {
    uint16_t len = 0;
    const char* p = str;
    while (*p) {
        if (*p != '\n') len++;
        p++;
    }
    uint16_t text_width = len * (FONT_CHAR_WIDTH + 1);
    uint16_t start_x = center_x - (text_width / 2);
    draw_string(start_x, y, str, color, bg);
}

void FrameBuffer::draw_char_scaled(uint16_t x, uint16_t y, char chr,
                                    uint16_t color, uint16_t bg, uint8_t scale) {
    if (chr < 0x20 || chr > 0x7E) chr = 0x20;
    uint8_t idx = chr - 0x20;

    for (uint8_t row = 0; row < 8; row++) {
        uint8_t bits = FONT_8X8[idx][row];
        for (uint8_t col = 0; col < 8; col++) {
            uint16_t px = x + col * scale;
            uint16_t py = y + row * scale;
            if (bits & (1 << (7 - col))) {
                fill_rect(px, py, scale, scale, color);
            } else if (bg != 0xFFFF) {
                fill_rect(px, py, scale, scale, bg);
            }
        }
    }
}

void FrameBuffer::draw_string_scaled(uint16_t x, uint16_t y, const char* str,
                                      uint16_t color, uint16_t bg, uint8_t scale) {
    uint16_t cx = x;
    uint8_t step = 8 * scale + scale;
    while (*str) {
        if (*str == '\n') {
            cx = x;
            y += 8 * scale + scale;
        } else {
            draw_char_scaled(cx, y, *str, color, bg, scale);
            cx += step;
        }
        str++;
    }
}

void FrameBuffer::draw_string_centered_scaled(uint16_t center_x, uint16_t y, const char* str,
                                               uint16_t color, uint16_t bg, uint8_t scale) {
    uint16_t len = 0;
    const char* p = str;
    while (*p) { if (*p != '\n') len++; p++; }
    uint16_t text_w = len * (8 * scale + scale);
    uint16_t sx = (center_x > text_w / 2) ? center_x - text_w / 2 : 0;
    draw_string_scaled(sx, y, str, color, bg, scale);
}

void FrameBuffer::draw_image_rgb565(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        std::cerr << "[FrameBuffer] Error: no se pudo abrir " << filepath << std::endl;
        return;
    }

    uint8_t hdr[4];
    if (fread(hdr, 1, 4, file) != 4) {
        std::cerr << "[FrameBuffer] Error leyendo header de " << filepath << std::endl;
        fclose(file);
        return;
    }
    uint16_t w = (hdr[0] << 8) | hdr[1];
    uint16_t h = (hdr[2] << 8) | hdr[3];

    uint16_t draw_w = (w > LCD_WIDTH) ? LCD_WIDTH : w;
    uint16_t draw_h = (h > LCD_HEIGHT) ? LCD_HEIGHT : h;

    for (uint16_t row = 0; row < draw_h; row++) {
        uint32_t dst_off = row * LCD_WIDTH;
        for (uint16_t col = 0; col < draw_w; col++) {
            uint8_t hi, lo;
            if (fread(&hi, 1, 1, file) != 1 || fread(&lo, 1, 1, file) != 1) {
                m_data[dst_off + col] = BLACK;
                continue;
            }
            m_data[dst_off + col] = (hi << 8) | lo;
        }
    }

    fclose(file);
}

void FrameBuffer::flush(SSD1963& display) const {
    display.draw_buffer(m_data);
}
