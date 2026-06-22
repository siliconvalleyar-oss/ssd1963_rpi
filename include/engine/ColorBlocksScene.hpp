#pragma once

#include <engine/Scene.hpp>

/*! \class ColorBlocksScene
 *  \brief Demo de bloques de colores con auto-retorno al menú. */
class ColorBlocksScene : public Scene {
public:
    explicit ColorBlocksScene(SSD1963& display);

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw() override;
    uint8_t handle_button(uint8_t btn) override;
    const char* name() const override;

private:
    struct ColorBlock {
        uint16_t x;
        uint16_t y;
        uint16_t color;
        const char* name;
    };

    static constexpr uint16_t BLOCK_W = 45;
    static constexpr uint16_t BLOCK_H = 45;

    ColorBlock  m_blocks[7];
    bool        m_blocks_drawn;
    bool        m_image_drawn;
    uint32_t    m_timer;
    uint8_t     m_phase;

    static constexpr uint32_t PHASE_BLOCKS_MS = 2000;
    static constexpr uint32_t PHASE_IMAGE_MS  = 5000;
};
