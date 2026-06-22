#pragma once

#include <engine/Scene.hpp>

/*! \class ColorBlocksScene
 *  \brief Demo de bloques de colores.
 *
 *  Muestra 7 bloques de colores básicos (RGBYCMW)
 *  y luego transiciona a imagen o menú. */
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
    static constexpr uint16_t BLOCK_GAP = 10;

    ColorBlock  m_blocks[7];       ///< Array de bloques de color
    bool        m_blocks_drawn;    ///< Flag de bloques dibujados
    bool        m_image_drawn;     ///< Flag de imagen mostrada
    uint32_t    m_timer;           ///< Temporizador de transición
    uint8_t     m_phase;           ///< Fase: 0=bloques, 1=imagen
};
