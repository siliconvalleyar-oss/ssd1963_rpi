#include <FrameBuffer.hpp>
#include <engine/Sprite.hpp>
#include <cstring>
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

void FrameBuffer::flush(SSD1963& display) const {
    display.draw_buffer(m_data);
}
