#pragma once

#include <cstdint>
#include <ssd1963.hpp>
#include <engine/Scene.hpp>
#include <config_hw.hpp>
#include <color.hpp>

#define ENGINE_VERSION "2.0.0"

/*! \enum EngineState
 *  \brief Estados del ciclo de vida del motor. */
enum class EngineState : uint8_t {
    INIT,       ///< Inicializando componentes
    RUNNING,    ///< Ejecutando escena activa
    SLEEP,      ///< Suspensión de bajo consumo
    EXIT        ///< Solicitud de salida
};

/*! \enum Button
 *  \brief Mapeo de botones GPIO para entrada de usuario. */
enum class Button : uint8_t {
    NONE   = 0,
    UP     = 1,  ///< GPIO 7  - Navegar arriba / anterior
    DOWN   = 2,  ///< GPIO 8  - Navegar abajo / siguiente
    SELECT = 3,  ///< GPIO 9  - Seleccionar / entrar
    BACK   = 4   ///< GPIO 10 - Retroceder / salir
};

/*! \class GameEngine
 *  \brief Motor de juegos principal.
 *
 *  Gestiona el ciclo de vida de escenas, entrada de usuario,
 *  temporización y control de GPIO. Sigue un patrón de máquina
 *  de estados con escenas intercambiables. */
class GameEngine {
public:
    GameEngine();
    ~GameEngine();

    /*! \brief Inicializa GPIO, driver LCD y engine.
     *  \return true si todo se inicializa correctamente. */
    bool init();

    /*! \brief Bucle principal del motor. */
    void run();

    /*! \brief Solicita la salida del bucle principal. */
    void quit();

    /*! \brief Cambia a una nueva escena.
     *  \param scene Puntero a la escena (el engine NO toma ownership). */
    void set_scene(Scene* scene);

    /*! \brief Obtiene el driver del display.
     *  \return Referencia al SSD1963. */
    SSD1963& display();

    /*! \brief Lee el estado actual de los botones GPIO.
     *  \return Botón presionado (o NONE). */
    Button read_buttons();

    /*! \brief Obtiene el estado del motor.
     *  \return EngineState actual. */
    EngineState state() const;

    /*! \brief Obtiene el tiempo transcurrido desde el inicio en ms.
     *  \return Milisegundos desde init(). */
    uint32_t elapsed_ms() const;

    /*! \brief Configura pines GPIO como entrada para botones.
     *  \param enable true para habilitar, false para deshabilitar. */
    void enable_buttons(bool enable);

private:
    SSD1963         m_display;     ///< Driver del display TFT
    Scene*          m_active_scene; ///< Escena actualmente activa
    EngineState     m_state;       ///< Estado interno del motor
    uint32_t        m_start_time;  ///< Timestamp de inicio (us)
    bool            m_buttons_enabled; ///< Flag de botones habilitados

    /*! \brief Procesa la entrada de usuario y la delega a la escena. */
    void process_input();

    /*! \brief Temporizador de precisión usando busy-wait.
     *  \param ms Milisegundos a esperar. */
    void engine_delay(uint32_t ms);
};
