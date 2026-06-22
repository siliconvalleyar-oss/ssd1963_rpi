#pragma once
#include <cstdint>
#include <ssd1963.hpp>

#define SPRITE_KEY_COLOR 0x0001

class Sprite {
public:
    Sprite();
    ~Sprite();

    bool load(const char* filepath);
    void destroy();

    void draw(SSD1963& display, int16_t x, int16_t y) const;
    void draw_bg(SSD1963& display, int16_t x, int16_t y, uint16_t bg) const;

    uint16_t width()  const { return m_w; }
    uint16_t height() const { return m_h; }
    bool loaded()     const { return m_data != nullptr; }

private:
    uint16_t* m_data;
    uint16_t  m_w;
    uint16_t  m_h;
};
