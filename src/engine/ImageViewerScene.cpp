#include <engine/ImageViewerScene.hpp>
#include <engine/GameEngine.hpp>
#include <cstdio>
#include <iostream>

static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return RGB565CONVERT(r, g, b);
}

ImageViewerScene::ImageViewerScene(SSD1963& display, const char* filepath,
                                   const char* caption)
    : Scene(display)
    , m_filepath(filepath)
    , m_caption(caption)
    , m_image_loaded(false)
    , m_timer(0)
{}

bool ImageViewerScene::on_enter() {
    m_image_loaded = false;
    m_timer = 0;
    return true;
}

void ImageViewerScene::update(uint32_t dt) {
    m_timer += dt;
    if (m_image_loaded && m_timer > SHOW_MS) {
        if (m_engine && m_engine->menu_scene()) {
            m_engine->set_scene(m_engine->menu_scene());
        }
    }
}

void ImageViewerScene::draw(FrameBuffer& fb) {
    if (!m_image_loaded) {
        fb.clear(BLACK);
        fb.draw_image_rgb565(m_filepath);
        m_image_loaded = true;
        std::cout << "[Gallery] " << m_filepath << "\n";
    }

    draw_frame(fb);

    // Créditos/caption abajo
    fb.draw_string_centered(LCD_WIDTH / 2, LCD_HEIGHT - 10,
                           m_caption, rgb(180, 180, 200), BLACK);
}

const char* ImageViewerScene::name() const {
    return "ImageViewerScene";
}

void ImageViewerScene::set_filepath(const char* filepath, const char* caption) {
    m_filepath = filepath;
    m_caption  = caption;
    m_image_loaded = false;
}

void ImageViewerScene::draw_frame(FrameBuffer& fb) {
    for (uint16_t x = 0; x < LCD_WIDTH; x++) {
        fb.set_pixel(x, 0,                       rgb(255, 200, 0));
        fb.set_pixel(x, LCD_HEIGHT - 1,           rgb(255, 200, 0));
    }
    for (uint16_t y = 0; y < LCD_HEIGHT; y++) {
        fb.set_pixel(0, y,                       rgb(255, 200, 0));
        fb.set_pixel(LCD_WIDTH - 1, y,           rgb(255, 200, 0));
    }
}
