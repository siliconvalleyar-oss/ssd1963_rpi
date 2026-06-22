#include <engine/MenuScene.hpp>
#include <engine/GameEngine.hpp>
#include <cstring>
#include <cstdio>
#include <iostream>

static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return RGB565CONVERT(r, g, b);
}

MenuScene::MenuScene(SSD1963& display, const char* title)
    : Scene(display), m_title(title), m_count(0)
    , m_target(0), m_next(0), m_cur(0)
    , m_timer(0), m_drawn(false), m_phase(CYCLE)
{
    std::memset(m_items, 0, sizeof(m_items));
}

void MenuScene::add_item(const char* label, Scene* target) {
    if (m_count >= MENU_MAX_ITEMS) return;
    m_items[m_count].label  = label;
    m_items[m_count].target = target;
    m_count++;
}

bool MenuScene::on_enter() {
    m_target = m_next;
    m_cur    = 0;
    m_phase  = CYCLE;
    m_timer  = 0;
    m_drawn  = false;

    std::cout << "[Menu] -> " << m_items[m_target].label << "\n";
    return true;
}

void MenuScene::update(uint32_t dt) {
    m_timer += dt;

    if (m_phase == CYCLE) {
        // Pasar al siguiente item cada CYCLE_MS
        if (m_timer >= CYCLE_MS) {
            m_timer = 0;
            m_cur = (m_cur + 1) % m_count;
            m_drawn = false;

            // Si ya completamos un ciclo y volvemos a m_target, pasar a HOLD
            if (m_cur == m_target) {
                m_phase = HOLD;
                m_timer = 0;
                std::cout << "[Menu] hold -> " << m_items[m_target].label << "\n";
            }
        }
    } else if (m_phase == HOLD) {
        // Esperar HOLD_MS y seleccionar
        if (m_timer >= HOLD_MS) {
            m_timer = 0;
            do_select();
        }
    }
}

void MenuScene::do_select() {
    m_next = (m_target + 1) % m_count;
    m_phase = DONE;

    std::cout << "[Menu] SELECT " << m_items[m_target].label << "\n";

    if (m_items[m_target].target && m_engine) {
        m_engine->set_scene(m_items[m_target].target);
    } else if (std::strcmp(m_items[m_target].label, "SALIR") == 0 && m_engine) {
        m_engine->quit();
    }
}

void MenuScene::draw() {
    if (m_drawn) return;
    m_drawn = true;

    m_display.clear_screen(rgb(8, 8, 28));
    uint16_t cx = LCD_WIDTH / 2;
    uint8_t  sc = 2;

    // Título
    m_display.draw_string_centered_scaled(cx, 4, m_title,
                                          rgb(255, 200, 80), rgb(8, 8, 28), sc);

    // Línea separadora
    for (uint16_t x = 10; x < LCD_WIDTH - 10; x++)
        m_display.draw_pixel(x, 26, rgb(200, 0, 120));

    // Items
    for (uint8_t i = 0; i < m_count; i++) {
        uint16_t iy = 36 + i * 20;
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%s", m_items[i].label);

        if (i == m_cur) {
            uint16_t bg = (m_phase == HOLD) ? rgb(80, 40, 20) : rgb(30, 50, 100);
            const char* arrow = (m_phase == HOLD) ? ">>" : ">";
            m_display.fill_rect(4, iy - 1, LCD_WIDTH - 8, 18, bg);
            m_display.draw_string_scaled(8,  iy, arrow,
                                         rgb(255, 200, 100), bg, sc);
            m_display.draw_string_scaled(28, iy, buf,
                                         rgb(255, 255, 255), bg, sc);
        } else {
            m_display.draw_string_scaled(28, iy, buf,
                                         rgb(150, 150, 180), rgb(8, 8, 28), sc);
        }
    }

    // Footer
    char footer[24];
    std::snprintf(footer, sizeof(footer), "[%d/%d]", m_target + 1, m_count);
    m_display.draw_string_centered(cx, LCD_HEIGHT - 12, footer,
                                   rgb(80, 80, 120), rgb(8, 8, 28));
}

const char* MenuScene::name() const {
    return "MenuScene";
}
