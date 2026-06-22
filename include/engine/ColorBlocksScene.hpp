#pragma once
#include <engine/Scene.hpp>

class ColorBlocksScene : public Scene {
public:
    explicit ColorBlocksScene(SSD1963& display);

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw() override;
    const char* name() const override;

private:
    struct Block { uint16_t x, y, color; const char* name; };
    Block m_blocks[7];
    bool  m_blocks_drawn;
    bool  m_image_drawn;
    uint32_t m_timer;
    uint8_t  m_phase;

    static constexpr uint16_t BW = 45, BH = 45;
    static constexpr uint32_t T_BLOCKS = 2000;
    static constexpr uint32_t T_IMAGE  = 5000;
};
