#include <engine/ImageViewerScene.hpp>
#include <engine/GameEngine.hpp>
#include <cstdio>

ImageViewerScene::ImageViewerScene(SSD1963& display, const char* filepath)
    : Scene(display)
    , m_filepath(filepath)
    , m_image_loaded(false)
    , m_timer(0)
{
}

bool ImageViewerScene::on_enter() {
    m_image_loaded = false;
    m_timer = 0;
    return true;
}

void ImageViewerScene::update(uint32_t dt) {
    m_timer += dt;

    // Auto-retorno al menú
    if (m_image_loaded && m_timer > AUTO_RETURN_MS) {
        if (m_engine && m_engine->menu_scene()) {
            m_engine->set_scene(m_engine->menu_scene());
        }
    }
}

void ImageViewerScene::draw() {
    if (!m_image_loaded) {
        m_display.clear_screen(BLACK);
        m_display.draw_image_rgb565(m_filepath);
        m_image_loaded = true;
        std::cout << "[ImageViewer] Cargado: " << m_filepath << "\n";
    }

    char info[64];
    std::snprintf(info, sizeof(info), "%s", m_filepath);
    m_display.draw_string(4, LCD_HEIGHT - 12, info,
                          RGB565CONVERT(150, 150, 150), BLACK);
}

uint8_t ImageViewerScene::handle_button(uint8_t btn) {
    (void)btn;
    if (m_engine && m_engine->menu_scene()) {
        m_engine->set_scene(m_engine->menu_scene());
    }
    return 0;
}

const char* ImageViewerScene::name() const {
    return "ImageViewerScene";
}

void ImageViewerScene::set_filepath(const char* filepath) {
    m_filepath = filepath;
    m_image_loaded = false;
}
