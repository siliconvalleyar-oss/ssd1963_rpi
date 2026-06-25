#include <engine/Sprite.hpp>
#include <cstdio>
#include <cstdlib>
#include <iostream>

Sprite::Sprite() : m_data(nullptr), m_w(0), m_h(0) {}

Sprite::~Sprite() { destroy(); }

bool Sprite::load(const char* filepath) {
    destroy();

    FILE* f = fopen(filepath, "rb");
    if (!f) {
        std::cerr << "[Sprite] No se pudo abrir: " << filepath << "\n";
        return false;
    }

    uint8_t hdr[4];
    if (fread(hdr, 1, 4, f) != 4) {
        std::cerr << "[Sprite] Error leyendo header\n";
        fclose(f);
        return false;
    }
    m_w = (hdr[0] << 8) | hdr[1];
    m_h = (hdr[2] << 8) | hdr[3];

    uint32_t n = (uint32_t)m_w * m_h;
    m_data = new uint16_t[n];
    if (!m_data) {
        std::cerr << "[Sprite] Error asignando memoria\n";
        fclose(f);
        return false;
    }

    for (uint32_t i = 0; i < n; i++) {
        uint8_t hi, lo;
        if (fread(&hi, 1, 1, f) != 1 || fread(&lo, 1, 1, f) != 1) {
            m_data[i] = SPRITE_KEY_COLOR;
            continue;
        }
        m_data[i] = (hi << 8) | lo;
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
        int16_t py = y + row;
        if (py < 0 || py >= 272) continue;

        uint16_t seg_start = 0;
        bool in_seg = false;

        for (uint16_t col = 0; col < m_w; col++) {
            if (m_data[row * m_w + col] != SPRITE_KEY_COLOR) {
                if (!in_seg) {
                    seg_start = col;
                    in_seg = true;
                }
            } else if (in_seg) {
                int16_t px = x + seg_start;
                uint16_t seg_w = col - seg_start;
                if (px + seg_w > 0 && px < 480) {
                    uint16_t clip_off = (px < 0) ? -px : 0;
                    uint16_t clip_x  = (px < 0) ? 0 : px;
                    uint16_t clip_w  = seg_w - clip_off;
                    if (clip_x + clip_w > 480) clip_w = 480 - clip_x;
                    if (clip_w > 0) {
                        display.draw_pixels(clip_x, py, clip_w, 1,
                            m_data + row * m_w + seg_start + clip_off);
                    }
                }
                in_seg = false;
            }
        }

        if (in_seg) {
            int16_t px = x + seg_start;
            uint16_t seg_w = m_w - seg_start;
            if (px + seg_w > 0 && px < 480) {
                uint16_t clip_off = (px < 0) ? -px : 0;
                uint16_t clip_x  = (px < 0) ? 0 : px;
                uint16_t clip_w  = seg_w - clip_off;
                if (clip_x + clip_w > 480) clip_w = 480 - clip_x;
                if (clip_w > 0) {
                    display.draw_pixels(clip_x, py, clip_w, 1,
                        m_data + row * m_w + seg_start + clip_off);
                }
            }
        }
    }
}

void Sprite::draw_bg(SSD1963& display, int16_t x, int16_t y, uint16_t bg) const {
    if (!m_data) return;

    // Rellenar toda el área del sprite con bg (1 operación)
    int16_t px = x;
    int16_t py = y;
    if (px < 0) { px = 0; }
    if (py < 0) { py = 0; }
    uint16_t clip_w = m_w;
    uint16_t clip_h = m_h;
    if (px + clip_w > 480) clip_w = 480 - px;
    if (py + clip_h > 272) clip_h = 272 - py;
    if (clip_w > 0 && clip_h > 0) {
        display.fill_rect(px, py, clip_w, clip_h, bg);
    }

    // Dibujar píxeles no-transparentes encima (usando segmentos optimizados)
    draw(display, x, y);
}
