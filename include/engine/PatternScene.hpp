#pragma once

#include <engine/Scene.hpp>

/*! \enum PatternType
 *  \brief Tipos de patrones de prueba. */
enum class PatternType : uint8_t {
    COLOR_BARS    = 0,  ///< Barras de color verticales
    GRADIENT      = 1,  ///< Gradiente suave
    CHECKERBOARD  = 2,  ///< Patrón de damero
    GRID          = 3,  ///< Cuadrícula fina
    RAINBOW       = 4,  ///< Arcoíris horizontal
    COUNT         = 5   ///< Número total de patrones
};

/*! \class PatternScene
 *  \brief Escena de patrones de prueba para display.
 *
 *  Genera varios patrones de video útiles para verificar
 *  la calidad de imagen y calibración del TFT. */
class PatternScene : public Scene {
public:
    explicit PatternScene(SSD1963& display);

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw() override;
    uint8_t handle_button(uint8_t btn) override;
    const char* name() const override;

private:
    PatternType m_current;           ///< Patrón activo
    uint32_t    m_timer;             ///< Temporizador de auto-cambio
    bool        m_auto_cycle;        ///< true = cambia solo cada N segundos
    uint32_t    m_frame_count;       ///< Contador de frames para animación

    /*! \brief Dibuja barras de color verticales. */
    void draw_color_bars();

    /*! \brief Dibuja gradiente suave. */
    void draw_gradient();

    /*! \brief Dibuja patrón de damero. */
    void draw_checkerboard();

    /*! \brief Dibuja cuadrícula. */
    void draw_grid();

    /*! \brief Dibuja arcoíris. */
    void draw_rainbow();

    /*! \brief Convierte HSV a RGB565 para gradientes. */
    uint16_t hsv_to_rgb565(uint8_t h, uint8_t s, uint8_t v);
};
