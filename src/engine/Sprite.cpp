#include <engine/Sprite.hpp>
#include <cstdio>
#include <cstdlib>

Sprite::Sprite() : m_data(nullptr), m_w(0), m_h(0) {}

Sprite::~Sprite() { destroy(); }

bool Sprite::load(const char* filepath) {
    destroy();

    FILE* f = fopen(filepath, "rb");
    if (!f) {
        std::cerr << "[Sprite] No se pudo abrir: " << filepath << "\n";
        return false;
    }

    // Leer header: width, height (uint16 LE)
    uint8_t hdr[4];
    if (fread(hdr, 1, 4, f) != 4) {
        std::cerr << "[Sprite] Error leyendo header\n";
        fclose(f);
        return false;
    }
    m_w = hdr[0] | (hdr[1] << 8);
    m_h = hdr[2] | (hdr[3] << 8);

    uint32_t n = (uint32_t)m_w * m_h;
    m_data = new uint16_t[n];
    if (!m_data) {
        std::cerr << "[Sprite] Error asignando memoria\n";
        fclose(f);
        return false;
    }

    // Leer pixels (uint16 LE)
    for (uint32_t i = 0; i < n; i++) {
        uint8_t lo, hi;
        if (fread(&lo, 1, 1, f) != 1 || fread(&hi, 1, 1, f) != 1) {
            m_data[i] = SPRITE_KEY_COLOR;
            continue;
        }
        m_data[i] = lo | (hi << 8);
    }

    fclose(f);
    std::cout << "[Sprite] Cargado: " << filepath
              << " (" << m_w << "x" << m_h << ")\n";
    return true;
}

void Sprite::destroy() {
    delete[] m_data;
    m_data = nullptr;
    m_w = m_h = 0;
}

void Sprite::draw(SSD1963& display, int16_t x, int16_t y) const {
    if (!m_data) return;

    for (uint16_t row = 0; row < m_h; row++) {
        for (uint16_t col = 0; col < m_w; col++) {
            uint16_t c = m_data[row * m_w + col];
            if (c != SPRITE_KEY_COLOR) {
                int16_t px = x + col;
                int16_t py = y + row;
                if (px >= 0 && px < 480 && py >= 0 && py < 272) {
                    display.draw_pixel(px, py, c);
                }
            }
        }
    }
}

void Sprite::draw_bg(SSD1963& display, int16_t x, int16_t y, uint16_t bg) const {
    if (!m_data) return;

    for (uint16_t row = 0; row < m_h; row++) {
        for (uint16_t col = 0; col < m_w; col++) {
            uint16_t c = m_data[row * m_w + col];
            int16_t px = x + col;
            int16_t py = y + row;
            if (px < 0 || px >= 480 || py < 0 || py >= 272) continue;
            if (c != SPRITE_KEY_COLOR) {
                display.draw_pixel(px, py, c);
            } else {
                display.draw_pixel(px, py, bg);
            }
        }
    }
}
