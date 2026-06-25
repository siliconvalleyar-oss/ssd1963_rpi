#include <engine/PatternScene.hpp>
#include <engine/GameEngine.hpp>
#include <cstdio>

PatternScene::PatternScene(SSD1963& display) : Scene(display)
    , m_current(PatternType::COLOR_BARS), m_timer(0), m_total(0) {}

bool PatternScene::on_enter() {
    m_current = PatternType::COLOR_BARS;
    m_timer = m_total = 0;
    return true;
}

void PatternScene::update(uint32_t dt) {
    m_timer += dt; m_total += dt;
    if (m_timer > PAT_MS) {
        m_timer = 0;
        uint8_t n = (static_cast<uint8_t>(m_current) + 1) %
                    static_cast<uint8_t>(PatternType::COUNT);
        m_current = static_cast<PatternType>(n);
    }
    if (m_total > TOTAL_MS) {
        if (m_engine && m_engine->menu_scene())
            m_engine->set_scene(m_engine->menu_scene());
    }
}

void PatternScene::draw(FrameBuffer& fb) {
    switch (m_current) {
        case PatternType::COLOR_BARS:   draw_color_bars(fb);   break;
        case PatternType::GRADIENT:     draw_gradient(fb);     break;
        case PatternType::CHECKERBOARD: draw_checkerboard(fb); break;
        case PatternType::GRID:         draw_grid(fb);         break;
        case PatternType::RAINBOW:      draw_rainbow(fb);      break;
        default: break;
    }
    const char* names[] = {"COLOR BARS","GRADIENT","CHECKER","GRID","RAINBOW"};
    uint8_t idx = static_cast<uint8_t>(m_current);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "[ %s ]", names[idx]);
    fb.draw_string_centered(LCD_WIDTH/2, 4, buf, WHITE, BLACK);
    std::snprintf(buf, sizeof(buf), "Pattern %d/%d", idx+1, (int)PatternType::COUNT);
    fb.draw_string(4, LCD_HEIGHT-12, buf, RGB565CONVERT(150,150,150), BLACK);
}

const char* PatternScene::name() const { return "PatternScene"; }

void PatternScene::draw_color_bars(FrameBuffer& fb) {
    const uint16_t cols[] = {WHITE,YELLOW,CYAN,GREEN,MAGENTA,RED,BLUE,BLACK};
    uint16_t w = LCD_WIDTH / 8;
    for (uint8_t i = 0; i < 8; i++)
        fb.fill_rect(i*w, 0, w, LCD_HEIGHT, cols[i]);
}

void PatternScene::draw_gradient(FrameBuffer& fb) {
    for (uint16_t x = 0; x < LCD_WIDTH; x++)
        for (uint16_t y = 0; y < LCD_HEIGHT; y++)
            fb.set_pixel(x, y, hsv((x*240)/LCD_WIDTH, 255, 255));
}

void PatternScene::draw_checkerboard(FrameBuffer& fb) {
    const uint16_t SZ = 30;
    for (uint16_t y = 0; y < LCD_HEIGHT; y += SZ)
        for (uint16_t x = 0; x < LCD_WIDTH; x += SZ)
            fb.fill_rect(x, y, SZ, SZ,
                        ((x/SZ)+(y/SZ))%2 ? BLACK : WHITE);
}

void PatternScene::draw_grid(FrameBuffer& fb) {
    fb.clear(BLACK);
    for (uint16_t x = 0; x < LCD_WIDTH; x += 20)
        for (uint16_t y = 0; y < LCD_HEIGHT; y++)
            fb.set_pixel(x, y, RGB565CONVERT(0,100,0));
    for (uint16_t y = 0; y < LCD_HEIGHT; y += 20)
        for (uint16_t x = 0; x < LCD_WIDTH; x++)
            fb.set_pixel(x, y, RGB565CONVERT(0,100,0));
}

void PatternScene::draw_rainbow(FrameBuffer& fb) {
    for (uint16_t y = 0; y < LCD_HEIGHT; y++)
        for (uint16_t x = 0; x < LCD_WIDTH; x++)
            fb.set_pixel(x, y, hsv((y*240)/LCD_HEIGHT, 255, 255));
}

uint16_t PatternScene::hsv(uint8_t h, uint8_t s, uint8_t v) {
    uint8_t r,g,b, reg = h/43, rem = (h-reg*43)*6;
    uint8_t p = (v*(255-s))>>8, q = (v*(255-((s*rem)>>8)))>>8;
    uint8_t t = (v*(255-((s*(255-rem))>>8)))>>8;
    switch(reg) {
        case 0: r=v;g=t;b=p;break; case 1: r=q;g=v;b=p;break;
        case 2: r=p;g=v;b=t;break; case 3: r=p;g=q;b=v;break;
        case 4: r=t;g=p;b=v;break; default: r=v;g=p;b=q;break;
    }
    return RGB565CONVERT(r,g,b);
}
