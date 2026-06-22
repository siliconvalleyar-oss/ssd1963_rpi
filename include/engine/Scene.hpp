#pragma once

#include <cstdint>
#include <ssd1963.hpp>
#include <color.hpp>

// Forward declaration para evitar dependencia circular
class GameEngine;

/*! \class Scene
 *  \brief Clase base abstracta para todas las escenas del motor.
 *
 *  Cada pantalla o modo del sistema es una Scene. Las escenas
 *  se intercambian mediante GameEngine::set_scene(). */
class Scene {
public:
    explicit Scene(SSD1963& display) : m_display(display), m_engine(nullptr) {}
    virtual ~Scene() = default;

    /*! \brief Asigna el motor que gestiona esta escena.
     *  \param engine Puntero al GameEngine. */
    void set_engine(GameEngine* engine) { m_engine = engine; }

    virtual bool on_enter() { return true; }
    virtual void on_exit() {}
    virtual void update(uint32_t dt) = 0;
    virtual void draw() = 0;
    virtual uint8_t handle_button(uint8_t btn) { (void)btn; return 0; }
    virtual const char* name() const = 0;

protected:
    SSD1963&     m_display; ///< Referencia al controlador del display
    GameEngine*  m_engine;  ///< Motor que gestiona esta escena
};
