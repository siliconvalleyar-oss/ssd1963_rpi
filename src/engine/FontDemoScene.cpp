#include <engine/FontDemoScene.hpp>
#include <engine/GameEngine.hpp>
#include <cstdio>

static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return RGB565CONVERT(r, g, b);
}

FontDemoScene::FontDemoScene(SSD1963& display)
    : Scene(display), m_timer(0), m_phase_timer(0), m_phase(0), m_drawn(false) {}

bool FontDemoScene::on_enter() {
    m_timer = m_phase_timer = 0;
    m_phase = 0;
    m_drawn = false;
    m_display.clear_screen(rgb(10, 10, 30));
    return true;
}

void FontDemoScene::update(uint32_t dt) {
    m_timer += dt;
    if (m_timer > SHOW_MS) {
        if (m_engine && m_engine->menu_scene())
            m_engine->set_scene(m_engine->menu_scene());
    }
}

void FontDemoScene::draw() {
    if (m_drawn) return;
    m_drawn = true;

    uint16_t cx = LCD_WIDTH / 2;

    // Título principal escala 3 (24x24)
    m_display.draw_string_centered_scaled(cx, 6,  "FONT DEMO",
                                          rgb(255, 200, 0), rgb(10, 10, 30), 3);

    // Separador
    for (uint16_t x = 20; x < LCD_WIDTH - 20; x++)
        m_display.draw_pixel(x, 38, rgb(255, 0, 128));

    // Lorem ipsum escala 1 (8x8)
    m_display.draw_string(10, 48,
        "LOREM IPSUM DOLOR SIT AMET,",
        rgb(200, 200, 255), rgb(10, 10, 30));
    m_display.draw_string(10, 58,
        "CONSECTETUR ADIPISCING ELIT.",
        rgb(200, 200, 255), rgb(10, 10, 30));
    m_display.draw_string(10, 68,
        "SED DO EIUSMOD TEMPOR INCIDIDUNT",
        rgb(200, 200, 255), rgb(10, 10, 30));
    m_display.draw_string(10, 78,
        "UT LABORE ET DOLORE MAGNA ALIQUA.",
        rgb(200, 200, 255), rgb(10, 10, 30));

    // Escala 2 (16x16)
    m_display.draw_string_scaled(10, 98, "SCALE x2",
                                 rgb(100, 255, 100), rgb(10, 10, 30), 2);
    m_display.draw_string_scaled(10, 118, "LOREM IPSUM",
                                 rgb(100, 255, 100), rgb(10, 10, 30), 2);

    // Escala 1 — muestra de caracteres
    m_display.draw_string(10, 148,
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
        rgb(180, 180, 255), rgb(10, 10, 30));
    m_display.draw_string(10, 158,
        "abcdefghijklmnopqrstuvwxyz",
        rgb(180, 180, 255), rgb(10, 10, 30));
    m_display.draw_string(10, 168,
        "0123456789 !@#$%^&*()_+-=[]",
        rgb(180, 180, 255), rgb(10, 10, 30));

    // Footer con cuenta regresiva
    m_display.draw_string_centered(cx, 240,
        "FONT BITMAP 8x8  -  SCALABLE x2 x3",
        rgb(120, 120, 160), rgb(10, 10, 30));
}

const char* FontDemoScene::name() const { return "FontDemoScene"; }
