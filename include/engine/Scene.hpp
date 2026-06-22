#pragma once

#include <cstdint>
#include <ssd1963.hpp>
#include <color.hpp>

/*! \class Scene
 *  \brief Clase base abstracta para todas las escenas del motor.
 *
 *  Cada pantalla o modo del sistema es una Scene. Las escenas
 *  se intercambian mediante GameEngine::set_scene(). */
class Scene {
public:
    /*! \brief Constructor. Recibe referencia al display.
     *  \param display Referencia al SSD1963. */
    explicit Scene(SSD1963& display) : m_display(display) {}

    virtual ~Scene() = default;

    /*! \brief Llamado al entrar a la escena.
     *  \return true si la transición fue exitosa. */
    virtual bool on_enter() { return true; }

    /*! \brief Llamado al salir de la escena. */
    virtual void on_exit() {}

    /*! \brief Actualización por frame.
     *  \param dt Delta time en milisegundos. */
    virtual void update(uint32_t dt) = 0;

    /*! \brief Dibujado por frame. */
    virtual void draw() = 0;

    /*! \brief Procesa entrada de botones.
     *  \param btn Botón presionado.
     *  \return Código de acción (0 = nada, 1 = cambiar escena). */
    virtual uint8_t handle_button(uint8_t btn) { (void)btn; return 0; }

    /*! \brief Obtiene el nombre descriptivo de la escena.
     *  \return Nombre en formato string. */
    virtual const char* name() const = 0;

protected:
    SSD1963& m_display;  ///< Referencia al controlador del display
};
