#include <engine/MenuScene.hpp>
#include <engine/GameEngine.hpp>
#include <cstring>
#include <cstdio>
#include <iostream>

static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return RGB565CONVERT(r, g, b);
}

MenuScene::MenuScene(SSD1963& display, const char* title)
    : Scene(display), m_title(title), m_count(0), m_sel(0), m_next_sel(0)
    , m_timer(0), m_drawn(false)
{
    std::memset(m_items, 0, sizeof(m_items));
}

void MenuScene::add_item(const char* label, Scene* target) {
    if (m_count >= MENU_MAX_ITEMS) return;
    m_items[m_count].label = label;
    m_items[m_count].target = target;
    m_count++;
}

bool MenuScene::on_enter() {
    m_sel = m_next_sel;
    m_timer = 0;
    m_drawn = false;

    std::cout << "[Menu] item " << (int)m_sel << ": "
              << m_items[m_sel].label << "\n";
    return true;
}

void MenuScene::update(uint32_t dt) {
    m_timer += dt;
    if (m_timer >= HIGHLIGHT_MS) {
        m_timer = 0;
        do_select();
    }
}

void MenuScene::draw() {
    if (m_drawn) return;
    m_drawn = true;

    m_display.clear_screen(rgb(8, 8, 28));

    uint16_t cx = LCD_WIDTH / 2;
    uint8_t  sc = 2; // escala 2 para todo

    // Título centrado, escala sc
    m_display.draw_string_centered_scaled(cx, 4, m_title,
                                          rgb(255, 200, 80), rgb(8, 8, 28), sc);

    // Línea separadora (y después del título: 4 + 16 + 4 = 24)
    for (uint16_t x = 10; x < LCD_WIDTH - 10; x++)
        m_display.draw_pixel(x, 26, rgb(200, 0, 120));

    // Items — todos visibles, el actual destacado
    for (uint8_t i = 0; i < m_count; i++) {
        uint16_t iy = 36 + i * 20;
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%s", m_items[i].label);

        if (i == m_sel) {
            // Item seleccionado: fondo + texto blanco
            m_display.fill_rect(4, iy - 1, LCD_WIDTH - 8, 18,
                                rgb(30, 50, 100));
            m_display.draw_string_scaled(12, iy, ">", rgb(100, 200, 255),
                                         rgb(30, 50, 100), sc);
            m_display.draw_string_scaled(28, iy, buf, rgb(255, 255, 255),
                                         rgb(30, 50, 100), sc);
        } else {
            m_display.draw_string_scaled(12, iy, " ", rgb(0, 0, 0),
                                         rgb(8, 8, 28), sc);
            m_display.draw_string_scaled(28, iy, buf, rgb(150, 150, 180),
                                         rgb(8, 8, 28), sc);
        }
    }

    // Footer: item actual / total
    char footer[24];
    std::snprintf(footer, sizeof(footer), "[%d/%d]", m_sel + 1, m_count);
    m_display.draw_string_centered(cx, LCD_HEIGHT - 12, footer,
                                   rgb(80, 80, 120), rgb(8, 8, 28));
}

const char* MenuScene::name() const {
    return "MenuScene";
}

void MenuScene::do_select() {
    m_next_sel = (m_sel + 1) % m_count;

    std::cout << "[Menu] SELECT " << m_items[m_sel].label << "\n";

    if (m_items[m_sel].target && m_engine) {
        m_engine->set_scene(m_items[m_sel].target);
    } else if (std::strcmp(m_items[m_sel].label, "SALIR") == 0 && m_engine) {
        m_engine->quit();
    }
}
