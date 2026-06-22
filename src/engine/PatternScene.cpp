#include <engine/PatternScene.hpp>
#include <engine/GameEngine.hpp>
#include <cstdio>
#include <cmath>

PatternScene::PatternScene(SSD1963& display)
    : Scene(display)
    , m_current(PatternType::COLOR_BARS)
    , m_timer(0)
    , m_auto_cycle(true)
    , m_frame_count(0)
{
}

bool PatternScene::on_enter() {
    m_current = PatternType::COLOR_BARS;
    m_timer = 0;
    m_frame_count = 0;
    m_auto_cycle = true;
    return true;
}

void PatternScene::update(uint32_t dt) {
    m_timer += dt;
    m_frame_count++;

    if (m_auto_cycle && m_timer > 3000) {
        m_timer = 0;
        uint8_t next = (static_cast<uint8_t>(m_current) + 1) %
                       static_cast<uint8_t>(PatternType::COUNT);
        m_current = static_cast<PatternType>(next);
    }
}

void PatternScene::draw() {
    switch (m_current) {
        case PatternType::COLOR_BARS:  draw_color_bars();  break;
        case PatternType::GRADIENT:    draw_gradient();     break;
        case PatternType::CHECKERBOARD: draw_checkerboard(); break;
        case PatternType::GRID:        draw_grid();         break;
        case PatternType::RAINBOW:     draw_rainbow();      break;
        default: break;
    }

    char label[32];
    const char* names[] = {
        "COLOR BARS", "GRADIENT", "CHECKERBOARD", "GRID", "RAINBOW"
    };
    uint8_t idx = static_cast<uint8_t>(m_current);
    std::snprintf(label, sizeof(label), "[ %s ]", names[idx]);
    m_display.draw_string_centered(LCD_WIDTH / 2, 4, label, WHITE, BLACK);

    std::snprintf(label, sizeof(label), "Pattern %d/%d", idx + 1,
                  static_cast<int>(PatternType::COUNT));
    m_display.draw_string(4, LCD_HEIGHT - 12, label,
                          RGB565CONVERT(150, 150, 150), BLACK);
}

uint8_t PatternScene::handle_button(uint8_t btn) {
    switch (btn) {
        case static_cast<uint8_t>(Button::UP):
        case static_cast<uint8_t>(Button::DOWN): {
            int8_t delta = (btn == static_cast<uint8_t>(Button::UP)) ? -1 : 1;
            int8_t next = static_cast<int8_t>(m_current) + delta;
            if (next < 0) next = static_cast<int8_t>(PatternType::COUNT) - 1;
            if (next >= static_cast<int8_t>(PatternType::COUNT)) next = 0;
            m_current = static_cast<PatternType>(next);
            m_timer = 0;
            break;
        }
        case static_cast<uint8_t>(Button::BACK):
            return MENU_ACTION_CHANGE;
        default:
            break;
    }
    return MENU_ACTION_NONE;
}

const char* PatternScene::name() const {
    return "PatternScene";
}

void PatternScene::draw_color_bars() {
    static const uint16_t bar_colors[] = {
        WHITE, YELLOW, CYAN, GREEN, MAGENTA, RED, BLUE, BLACK
    };
    const uint8_t NUM_BARS = 8;
    uint16_t bar_w = LCD_WIDTH / NUM_BARS;

    for (uint8_t i = 0; i < NUM_BARS; i++) {
        m_display.fill_rect(i * bar_w, 0, bar_w, LCD_HEIGHT, bar_colors[i]);
    }
}

void PatternScene::draw_gradient() {
    for (uint16_t x = 0; x < LCD_WIDTH; x++) {
        uint8_t h = (x * 240) / LCD_WIDTH;
        for (uint16_t y = 0; y < LCD_HEIGHT; y++) {
            uint16_t color = hsv_to_rgb565(h, 255, 255);
            m_display.draw_pixel(x, y, color);
        }
    }
}

void PatternScene::draw_checkerboard() {
    const uint16_t SIZE = 30;
    bool white = true;
    for (uint16_t y = 0; y < LCD_HEIGHT; y += SIZE) {
        for (uint16_t x = 0; x < LCD_WIDTH; x += SIZE) {
            m_display.fill_rect(x, y, SIZE, SIZE, white ? WHITE : BLACK);
            white = !white;
        }
        white = (y / SIZE) % 2 == 0;
    }
}

void PatternScene::draw_grid() {
    m_display.clear_screen(BLACK);
    for (uint16_t x = 0; x < LCD_WIDTH; x += 20) {
        for (uint16_t y = 0; y < LCD_HEIGHT; y++) {
            m_display.draw_pixel(x, y, RGB565CONVERT(0, 100, 0));
        }
    }
    for (uint16_t y = 0; y < LCD_HEIGHT; y += 20) {
        for (uint16_t x = 0; x < LCD_WIDTH; x++) {
            m_display.draw_pixel(x, y, RGB565CONVERT(0, 100, 0));
        }
    }
}

void PatternScene::draw_rainbow() {
    for (uint16_t y = 0; y < LCD_HEIGHT; y++) {
        uint8_t h = (y * 240) / LCD_HEIGHT;
        uint16_t color = hsv_to_rgb565(h, 255, 255);
        for (uint16_t x = 0; x < LCD_WIDTH; x++) {
            m_display.draw_pixel(x, y, color);
        }
    }
}

uint16_t PatternScene::hsv_to_rgb565(uint8_t h, uint8_t s, uint8_t v) {
    uint8_t r, g, b;
    uint8_t region = h / 43;
    uint8_t remainder = (h - region * 43) * 6;
    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }

    return RGB565CONVERT(r, g, b);
}
