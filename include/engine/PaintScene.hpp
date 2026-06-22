#pragma once

#include <engine/Scene.hpp>

/*! \class PaintScene
 *  \brief Escena de pintura simple.
 *
 *  Permite dibujar trazos en pantalla usando patrones
 *  predefinidos o simulando entrada. */
class PaintScene : public Scene {
public:
    explicit PaintScene(SSD1963& display);

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw() override;
    uint8_t handle_button(uint8_t btn) override;
    const char* name() const override;

private:
    static constexpr uint16_t BRUSH_SIZE = 6;  ///< Tamaño del pincel

    uint16_t    m_cursor_x;         ///< Posición X del cursor
    uint16_t    m_cursor_y;         ///< Posición Y del cursor
    uint16_t    m_brush_color;      ///< Color actual del pincel
    uint8_t     m_pattern_step;     ///< Paso del patrón automático
    bool        m_demo_mode;        ///< true = dibujo automático
    uint32_t    m_timer;            ///< Temporizador de animación

    /*! \brief Conjunto de colores para el pincel. */
    static const uint16_t PALETTE[8];

    /*! \brief Cambia al siguiente color de la paleta. */
    void next_color();
};
