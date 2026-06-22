#pragma once

#include <engine/Scene.hpp>

/*! \class PaintScene
 *  \brief Escena de pintura/dibujo automático con auto-retorno. */
class PaintScene : public Scene {
public:
    explicit PaintScene(SSD1963& display);

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw() override;
    uint8_t handle_button(uint8_t btn) override;
    const char* name() const override;

private:
    static constexpr uint16_t BRUSH_SIZE = 6;

    uint16_t    m_cursor_x;
    uint16_t    m_cursor_y;
    uint16_t    m_brush_color;
    uint8_t     m_pattern_step;
    bool        m_demo_mode;
    uint32_t    m_timer;

    static const uint16_t PALETTE[8];
    static constexpr uint32_t AUTO_RETURN_MS = 10000;

    void next_color();
};
