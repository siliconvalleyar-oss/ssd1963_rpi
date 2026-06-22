#include <engine/MenuScene.hpp>
#include <engine/GameEngine.hpp>
#include <cstring>
#include <cstdio>
#include <iostream>

static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) { return RGB565CONVERT(r, g, b); }

MenuScene::MenuScene(SSD1963& display, const char* title)
    : Scene(display), m_title(title), m_count(0), m_sel(0)
    , m_timer(0), m_last_cycle(0), m_drawn(false)
{ std::memset(m_items, 0, sizeof(m_items)); }

void MenuScene::add_item(const char* label, Scene* target) {
    if (m_count >= MENU_MAX_ITEMS) return;
    m_items[m_count].label  = label;
    m_items[m_count].target = target;
    m_count++;
}

bool MenuScene::on_enter() {
    m_sel = 0; m_timer = 0; m_last_cycle = 0; m_drawn = false;
    return true;
}

void MenuScene::update(uint32_t dt) {
    m_timer += dt;
    uint32_t cyc = m_timer / CYCLE_MS;
    if (cyc > m_last_cycle) {
        m_last_cycle = cyc;
        m_sel = (m_sel + 1) % m_count;
        m_drawn = false;
        std::cout << "[Menu] -> " << m_items[m_sel].label << "\n";
    }
    if (m_timer >= SELECT_MS) {
        std::cout << "[Menu] SELECT " << m_items[m_sel].label << "\n";
        m_timer = 0; m_last_cycle = 0;
        do_select();
    }
}

void MenuScene::draw() {
    if (m_drawn) return;
    m_drawn = true;

    m_display.clear_screen(rgb(10, 10, 30));

    uint16_t cx = LCD_WIDTH / 2;

    // Título
    m_display.draw_string_centered(cx, 8, m_title, rgb(255, 220, 80), rgb(10, 10, 30));

    // Línea
    for (uint16_t x = 10; x < LCD_WIDTH - 10; x++)
        m_display.draw_pixel(x, 26, rgb(200, 0, 100));

    // Items
    for (uint8_t i = 0; i < m_count; i++) {
        uint16_t iy = 38 + i * 28;
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%s", m_items[i].label);

        if (i == m_sel) {
            // Fondo + borde
            m_display.draw_rect(10, iy, LCD_WIDTH - 20, 24, rgb(60, 100, 200));
            m_display.draw_string(18, iy + 6, buf, rgb(255, 255, 255), rgb(20, 20, 60));
            m_display.draw_string(LCD_WIDTH - 26, iy + 6, ">",
                                  rgb(100, 200, 255), rgb(20, 20, 60));
        } else {
            m_display.draw_string(18, iy + 6, buf, rgb(180, 180, 200), rgb(10, 10, 30));
        }
    }

    // Footer
    char time_buf[16];
    uint32_t remain = (SELECT_MS - m_timer) / 1000;
    std::snprintf(time_buf, sizeof(time_buf), "auto > %lus", remain);
    m_display.draw_string_centered(cx, LCD_HEIGHT - 14, time_buf,
                                   rgb(80, 80, 120), rgb(10, 10, 30));
}

const char* MenuScene::name() const { return "MenuScene"; }

void MenuScene::do_select() {
    if (m_items[m_sel].target && m_engine)
        m_engine->set_scene(m_items[m_sel].target);
    else if (std::strcmp(m_items[m_sel].label, "SALIR") == 0 && m_engine)
        m_engine->quit();
}
