#pragma once

#include <engine/Scene.hpp>

enum class PatternType : uint8_t {
    COLOR_BARS    = 0,
    GRADIENT      = 1,
    CHECKERBOARD  = 2,
    GRID          = 3,
    RAINBOW       = 4,
    COUNT         = 5
};

/*! \class PatternScene
 *  \brief Generador de patrones de prueba con auto-ciclo y auto-retorno. */
class PatternScene : public Scene {
public:
    explicit PatternScene(SSD1963& display);

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw() override;
    uint8_t handle_button(uint8_t btn) override;
    const char* name() const override;

private:
    PatternType m_current;
    uint32_t    m_timer;
    bool        m_auto_cycle;
    uint32_t    m_frame_count;
    uint32_t    m_total_timer;

    static constexpr uint32_t PATTERN_CYCLE_MS = 3000;
    static constexpr uint32_t AUTO_RETURN_MS   = 18000;

    void draw_color_bars();
    void draw_gradient();
    void draw_checkerboard();
    void draw_grid();
    void draw_rainbow();
    uint16_t hsv_to_rgb565(uint8_t h, uint8_t s, uint8_t v);
};
