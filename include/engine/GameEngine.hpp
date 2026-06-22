#pragma once

#include <cstdint>
#include <ssd1963.hpp>
#include <engine/Scene.hpp>
#include <config_hw.hpp>
#include <color.hpp>

#define ENGINE_VERSION "2.1.0"

/*! \enum EngineState
 *  \brief Estados del ciclo de vida del motor. */
enum class EngineState : uint8_t {
    INIT,
    RUNNING,
    EXIT
};

/*! \enum Button
 *  \brief Mapeo de botones GPIO para entrada de usuario. */
enum class Button : uint8_t {
    NONE   = 0,
    UP     = 1,
    DOWN   = 2,
    SELECT = 3,
    BACK   = 4
};

/*! \class GameEngine
 *  \brief Motor principal.
 *
 *  Gestiona el ciclo de escenas, entrada de usuario, temporización.
 *  Las escenas pueden solicitar cambios mediante get_engine(). */
class GameEngine {
public:
    GameEngine();
    ~GameEngine();

    bool init();
    void run();
    void quit();

    void set_scene(Scene* scene);
    void set_menu_scene(Scene* scene);
    Scene* menu_scene() const;

    SSD1963& display();

    Button read_buttons();
    EngineState state() const;
    uint32_t elapsed_ms() const;
    void enable_buttons(bool enable);

private:
    SSD1963      m_display;
    Scene*       m_active_scene;  ///< Escena activa actual
    Scene*       m_menu_scene;    ///< Escena del menú (para retorno)
    EngineState  m_state;
    uint32_t     m_start_time;
    bool         m_buttons_enabled;

    void process_input();
    void engine_delay(uint32_t ms);
};
