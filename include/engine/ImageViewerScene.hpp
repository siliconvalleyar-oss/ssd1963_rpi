#pragma once

#include <engine/Scene.hpp>

/*! \class ImageViewerScene
 *  \brief Visor de imágenes RGB565 a pantalla completa con auto-retorno. */
class ImageViewerScene : public Scene {
public:
    explicit ImageViewerScene(SSD1963& display, const char* filepath = "assets/capibaras.rgb565");

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw() override;
    uint8_t handle_button(uint8_t btn) override;
    const char* name() const override;

    void set_filepath(const char* filepath);

private:
    const char* m_filepath;
    bool        m_image_loaded;
    uint32_t    m_timer;

    static constexpr uint32_t AUTO_RETURN_MS = 5000;
};
