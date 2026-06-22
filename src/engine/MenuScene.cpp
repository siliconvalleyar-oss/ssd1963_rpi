#include <engine/MenuScene.hpp>
#include <engine/GameEngine.hpp>
#include <cstring>
#include <cstdlib>

static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return RGB565CONVERT(r, g, b);
}

MenuScene::MenuScene(SSD1963& display, const char* title)
    : Scene(display)
    , m_title(title)
    , m_item_count(0)
    , m_selected(0)
    , m_timer(0)
    , m_star_timer(0)
{
    std::memset(m_items, 0, sizeof(m_items));
}

void MenuScene::add_item(const char* label, Scene* target, uint16_t color) {
    if (m_item_count >= MENU_MAX_ITEMS) return;
    m_items[m_item_count].label  = label;
    m_items[m_item_count].target = target;
    m_items[m_item_count].color  = color;
    m_item_count++;
}

bool MenuScene::on_enter() {
    m_selected = 0;
    m_timer = 0;
    m_star_timer = 0;
    return true;
}

void MenuScene::update(uint32_t dt) {
    m_timer += dt;
    m_star_timer += dt;

    if (m_star_timer > 50) m_star_timer = 0;

    // Auto-cycle highlight
    if (m_timer >= CYCLE_MS && m_timer < SELECT_MS) {
        m_timer = 0;
        m_selected = (m_selected + 1) % m_item_count;
    }

    // Auto-select
    if (m_timer >= SELECT_MS) {
        m_timer = 0;
        do_select();
    }
}

void MenuScene::draw() {
    draw_starfield();
    draw_cabinet_art();

    // Título arcade centrado
    uint16_t cx = LCD_WIDTH / 2;
    uint16_t ty = 14;
    m_display.draw_string_centered(cx, ty,     m_title, rgb(255, 200, 0),   rgb(10, 10, 30));
    m_display.draw_string_centered(cx, ty + 1, m_title, rgb(200, 150, 0),  rgb(10, 10, 30));
    m_display.draw_string_centered(cx, ty + 2, m_title, rgb(255, 220, 80), rgb(10, 10, 30));

    // Separador neón
    for (uint16_t x = 30; x < LCD_WIDTH - 30; x++) {
        uint16_t c = (x / 4) % 2 ? rgb(255, 0, 128) : rgb(0, 200, 255);
        m_display.draw_pixel(x, 30, c);
    }

    // Items
    uint16_t start_y = 44;
    for (uint8_t i = 0; i < m_item_count; i++) {
        draw_item(i, i == static_cast<uint8_t>(m_selected));
    }

    // Footer arcade
    m_display.draw_string_centered(cx, LCD_HEIGHT - 14,
                                   "AUTO-DEMO MODE",
                                   rgb(80, 80, 120), rgb(10, 10, 30));
}

const char* MenuScene::name() const { return "MenuScene"; }

void MenuScene::draw_starfield() {
    static uint8_t seed = 0;
    seed++;
    m_display.fill_rect(0, 0, LCD_WIDTH, LCD_HEIGHT, rgb(8, 8, 28));

    // Estrellas pseudoaleatorias animadas
    for (int i = 0; i < 60; i++) {
        uint16_t sx = (i * 137 + seed * 3) % LCD_WIDTH;
        uint16_t sy = (i * 251 + seed * 7) % LCD_HEIGHT;
        uint8_t  br = 80 + ((i * 13 + seed) % 60);
        m_display.draw_pixel(sx, sy, rgb(br / 3, br / 3, br));
    }
}

void MenuScene::draw_cabinet_art() {
    // Panel lateral izquierdo (decoración arcade)
    for (uint16_t y = 32; y < LCD_HEIGHT - 20; y++) {
        m_display.draw_pixel(2, y,  rgb(255, 0, 80));
        m_display.draw_pixel(3, y,  rgb(200, 0, 60));
        m_display.draw_pixel(LCD_WIDTH - 3, y, rgb(0, 200, 255));
        m_display.draw_pixel(LCD_WIDTH - 4, y, rgb(0, 150, 200));
    }
}

void MenuScene::draw_item(uint8_t index, bool selected) {
    uint16_t iy = 44 + index * 38;
    uint16_t ix = 16;
    uint16_t iw = LCD_WIDTH - 32;
    uint16_t col = m_items[index].color;

    if (selected) {
        // Brillo pulsante
        uint16_t glow = 120 + ((m_timer / 3) % 80);
        m_display.draw_rect(ix, iy, iw, 30, rgb(glow, glow, 255));
        m_display.fill_rect(ix + 1, iy + 1, iw - 2, 28, rgb(20, 20, 60));
    }

    // Caja de color (como un cartucho)
    m_display.fill_rect(ix + 4, iy + 4, 22, 22, col);
    m_display.draw_rect(ix + 4, iy + 4, 22, 22, selected ? WHITE : rgb(60, 60, 80));

    // Nombre del item
    uint16_t text_col = selected ? WHITE : rgb(180, 180, 200);
    m_display.draw_string(ix + 32, iy + 8, m_items[index].label, text_col,
                          selected ? rgb(20, 20, 60) : rgb(8, 8, 28));

    if (selected) {
        // Indicador "▶" animado
        uint16_t pulse = 128 + ((m_timer / 2) % 128);
        m_display.draw_string(ix + iw - 16, iy + 8, "\x10",
                              rgb(pulse, pulse, 255), rgb(20, 20, 60));
    }
}

void MenuScene::do_select() {
    if (m_items[m_selected].target != nullptr) {
        if (m_engine) m_engine->set_scene(m_items[m_selected].target);
    } else {
        if (std::strcmp(m_items[m_selected].label, "SALIR") == 0) {
            if (m_engine) m_engine->quit();
        }
    }
}
