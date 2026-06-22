#pragma once
#include <engine/Scene.hpp>

class FontDemoScene : public Scene {
public:
    explicit FontDemoScene(SSD1963& display);

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw() override;
    const char* name() const override;

private:
    uint32_t m_timer;
    uint32_t m_phase_timer;
    uint8_t  m_phase;
    bool     m_drawn;
    static constexpr uint32_t SHOW_MS = 8000;
};
