#include <engine/PaintScene.hpp>
#include <engine/GameEngine.hpp>
#include <cstdio>
#include <cmath>

const uint16_t PaintScene::PALETTE[8] = {
    RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE, RGB565CONVERT(255, 128, 0)
};

PaintScene::PaintScene(SSD1963& display)
    : Scene(display)
    , m_cursor_x(LCD_WIDTH / 2)
    , m_cursor_y(LCD_HEIGHT / 2)
    , m_brush_color(RED)
    , m_pattern_step(0)
    , m_demo_mode(true)
    , m_timer(0)
{
}

bool PaintScene::on_enter() {
    m_cursor_x = LCD_WIDTH / 2;
    m_cursor_y = LCD_HEIGHT / 2;
    m_brush_color = RED;
    m_pattern_step = 0;
    m_demo_mode = true;
    m_timer = 0;
    m_display.clear_screen(BLACK);
    return true;
}

void PaintScene::update(uint32_t dt) {
    m_timer += dt;

    // Auto-retorno al menú
    if (m_timer > AUTO_RETURN_MS) {
        if (m_engine && m_engine->menu_scene()) {
            m_engine->set_scene(m_engine->menu_scene());
        }
        return;
    }

    if (m_demo_mode && m_timer % 50 < dt) {
        float t = m_pattern_step * 0.05f;
        uint16_t cx = LCD_WIDTH / 2;
        uint16_t cy = LCD_HEIGHT / 2;
        uint16_t r = m_pattern_step;

        m_cursor_x = cx + (uint16_t)(r * 0.5f * cosf(t));
        m_cursor_y = cy + (uint16_t)(r * 0.5f * sinf(t));

        if (m_cursor_x < LCD_WIDTH && m_cursor_y < LCD_HEIGHT) {
            m_display.draw_block(m_cursor_x, m_cursor_y,
                                 BRUSH_SIZE, BRUSH_SIZE, m_brush_color);
        }

        if (m_pattern_step % 16 == 0) {
            next_color();
        }
        m_pattern_step++;
        if (m_pattern_step > 200) m_pattern_step = 0;
    }
}

void PaintScene::draw() {
    m_display.draw_pixel(m_cursor_x, m_cursor_y, WHITE);

    uint16_t bar_y = LCD_HEIGHT - 12;
    for (uint8_t i = 0; i < 8; i++) {
        m_display.fill_rect(i * (LCD_WIDTH / 8), bar_y,
                            (LCD_WIDTH / 8) - 1, 10, PALETTE[i]);
    }
    m_display.draw_rect(0, bar_y - 1, LCD_WIDTH, 12, RGB565CONVERT(80, 80, 80));
}

uint8_t PaintScene::handle_button(uint8_t btn) {
    switch (btn) {
        case static_cast<uint8_t>(Button::UP):
            if (m_cursor_y > BRUSH_SIZE) m_cursor_y -= 10;
            break;
        case static_cast<uint8_t>(Button::DOWN):
            if (m_cursor_y < LCD_HEIGHT - BRUSH_SIZE) m_cursor_y += 10;
            break;
        case static_cast<uint8_t>(Button::SELECT):
            next_color();
            break;
        case static_cast<uint8_t>(Button::BACK):
            if (m_engine && m_engine->menu_scene()) {
                m_engine->set_scene(m_engine->menu_scene());
            }
            break;
        default:
            break;
    }
    return 0;
}

const char* PaintScene::name() const {
    return "PaintScene";
}

void PaintScene::next_color() {
    static uint8_t color_index = 0;
    color_index = (color_index + 1) % 8;
    m_brush_color = PALETTE[color_index];
}
