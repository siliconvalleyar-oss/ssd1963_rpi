#pragma once
#include <engine/Scene.hpp>
#include <FrameBuffer.hpp>

enum class PatternType : uint8_t {
    COLOR_BARS, GRADIENT, CHECKERBOARD, GRID, RAINBOW, COUNT
};

class PatternScene : public Scene {
public:
    explicit PatternScene(SSD1963& display);

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw(FrameBuffer& fb) override;
    const char* name() const override;

private:
    PatternType m_current;
    uint32_t    m_timer;
    uint32_t    m_total;

    static constexpr uint32_t PAT_MS   = 3000;
    static constexpr uint32_t TOTAL_MS = 18000;

    void draw_color_bars(FrameBuffer& fb);
    void draw_gradient(FrameBuffer& fb);
    void draw_checkerboard(FrameBuffer& fb);
    void draw_grid(FrameBuffer& fb);
    void draw_rainbow(FrameBuffer& fb);
    uint16_t hsv(uint8_t h, uint8_t s, uint8_t v);
};
