#pragma once
#include <engine/Scene.hpp>

class TouchTestScene : public Scene {
public:
    explicit TouchTestScene(SSD1963& display);

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw(FrameBuffer& fb) override;
    const char* name() const override;

private:
    uint32_t m_timer;
    bool m_drawn;
    unsigned int m_last_x, m_last_y;
    bool m_touched;
};
