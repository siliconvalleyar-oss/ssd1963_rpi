#pragma once
#include <cstdint>
#include <cstring>
#include <config_hw.hpp>
#include <ssd1963.hpp>
#include <color.hpp>

class Sprite;

class FrameBuffer {
public:
    FrameBuffer();
    ~FrameBuffer();

    void clear(uint16_t color);
    void set_pixel(uint16_t x, uint16_t y, uint16_t color);
    void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void draw_sprite(int16_t x, int16_t y, const Sprite& sprite);
    void draw_buffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);

    void draw_char(uint16_t x, uint16_t y, char chr, uint16_t color, uint16_t bg);
    void draw_string(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg);
    void draw_string_centered(uint16_t center_x, uint16_t y, const char* str,
                              uint16_t color, uint16_t bg);
    void draw_char_scaled(uint16_t x, uint16_t y, char chr,
                          uint16_t color, uint16_t bg, uint8_t scale);
    void draw_string_scaled(uint16_t x, uint16_t y, const char* str,
                            uint16_t color, uint16_t bg, uint8_t scale);
    void draw_string_centered_scaled(uint16_t center_x, uint16_t y, const char* str,
                                     uint16_t color, uint16_t bg, uint8_t scale);

    void draw_image_rgb565(const char* filepath);

    void flush(SSD1963& display) const;

    uint16_t* data() { return m_data; }
    const uint16_t* data() const { return m_data; }

private:
    uint16_t* m_data;
};
