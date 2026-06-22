#include <engine/SpriteViewerScene.hpp>
#include <engine/GameEngine.hpp>
#include <cstring>
#include <cstdio>
#include <iostream>

static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return RGB565CONVERT(r, g, b);
}

SpriteViewerScene::SpriteViewerScene(SSD1963& display)
    : Scene(display), m_count(0), m_idx(0), m_timer(0), m_drawn(false)
{
    std::memset(m_entries, 0, sizeof(m_entries));
}

bool SpriteViewerScene::add_sprite(const char* filepath, const char* label) {
    if (m_count >= SPRITE_VIEWER_MAX) return false;
    Entry& e = m_entries[m_count];
    e.label  = label;
    e.loaded = e.sprite.load(filepath);
    m_count++;
    return e.loaded;
}

bool SpriteViewerScene::on_enter() {
    m_idx = 0;
    m_timer = 0;
    m_drawn = false;
    std::cout << "[Sprites] Showing " << (int)m_count << " sprites\n";
    return true;
}

void SpriteViewerScene::update(uint32_t dt) {
    m_timer += dt;
    if (m_timer >= SPRITE_MS) {
        m_timer = 0;
        m_idx++;
        m_drawn = false;
        if (m_idx >= m_count) {
            std::cout << "[Sprites] Done, returning to menu\n";
            if (m_engine && m_engine->menu_scene())
                m_engine->set_scene(m_engine->menu_scene());
        }
    }
}

void SpriteViewerScene::draw() {
    if (m_drawn) return;
    m_drawn = true;

    m_display.clear_screen(rgb(10, 10, 30));

    if (m_idx >= m_count) return;

    Entry& e = m_entries[m_idx];
    if (!e.loaded) {
        m_display.draw_string_centered(LCD_WIDTH / 2, LCD_HEIGHT / 2 - 4,
                                       "NO IMAGE", rgb(255, 0, 0), rgb(10, 10, 30));
        return;
    }

    // Centrar sprite en pantalla
    int16_t sx = ((int16_t)LCD_WIDTH  - (int16_t)e.sprite.width())  / 2;
    int16_t sy = ((int16_t)LCD_HEIGHT - (int16_t)e.sprite.height()) / 2;
    if (sx < 0) sx = 0;
    if (sy < 0) sy = 0;

    e.sprite.draw(m_display, sx, sy);

    // Label abajo
    char buf[64];
    std::snprintf(buf, sizeof(buf), "[%d/%d] %s", m_idx + 1, m_count, e.label);
    m_display.draw_string_centered(LCD_WIDTH / 2, LCD_HEIGHT - 10,
                                   buf, rgb(200, 200, 100), rgb(10, 10, 30));
}

const char* SpriteViewerScene::name() const {
    return "SpriteViewerScene";
}
