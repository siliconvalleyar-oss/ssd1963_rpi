#pragma once
#include <engine/Scene.hpp>

class ImageViewerScene : public Scene {
public:
    explicit ImageViewerScene(SSD1963& display, const char* filepath = "assets/capibaras.rgb565",
                              const char* caption = "ARCADE PHOTO");

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw() override;
    const char* name() const override;

    void set_filepath(const char* filepath, const char* caption);

private:
    const char* m_filepath;
    const char* m_caption;
    bool        m_image_loaded;
    uint32_t    m_timer;

    static constexpr uint32_t SHOW_MS = 6000;

    void draw_frame();
};
