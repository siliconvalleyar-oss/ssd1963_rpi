#pragma once

#include <engine/Scene.hpp>

#define MENU_MAX_ITEMS 16

/*! \brief Estructura de un elemento del menú. */
struct MenuItem {
    const char* label;      ///< Texto visible
    Scene*      target;     ///< Escena destino al seleccionar
    uint16_t    color;      ///< Color del icono/item
};

/*! \enum MenuAction
 *  \brief Acciones de retorno del menú. */
enum MenuAction : uint8_t {
    MENU_ACTION_NONE    = 0,
    MENU_ACTION_CHANGE  = 1,
    MENU_ACTION_EXIT    = 2
};

/*! \class MenuScene
 *  \brief Escena de menú interactivo con navegación.
 *
 *  Muestra una lista vertical de opciones con highlight
 *  y soporte para scroll cuando hay más items que los
 *  que caben en pantalla. */
class MenuScene : public Scene {
public:
    /*! \brief Constructor.
     *  \param display Referencia al SSD1963.
     *  \param title Título del menú. */
    explicit MenuScene(SSD1963& display, const char* title = "MENU PRINCIPAL");

    /*! \brief Agrega un item al menú.
     *  \param label Texto del item.
     *  \param target Escena destino.
     *  \param color Color distintivo. */
    void add_item(const char* label, Scene* target, uint16_t color);

    bool on_enter() override;
    void on_exit() override;
    void update(uint32_t dt) override;
    void draw() override;
    uint8_t handle_button(uint8_t btn) override;
    const char* name() const override;

private:
    const char*  m_title;                          ///< Título del menú
    MenuItem     m_items[MENU_MAX_ITEMS];          ///< Array de items
    uint8_t      m_item_count;                     ///< Número de items registrados
    int8_t       m_selected;                       ///< Índice del item seleccionado
    uint8_t      m_scroll_offset;                  ///< Desplazamiento vertical para scroll
    uint32_t     m_anim_timer;                     ///< Timer para animación de selección
    uint16_t     m_bg_color;                       ///< Color de fondo

    static constexpr uint8_t VISIBLE_ITEMS = 6;    ///< Items visibles sin scroll
    static constexpr uint16_t ITEM_HEIGHT  = 32;   ///< Alto de cada item en píxeles
    static constexpr uint16_t ITEM_MARGIN  = 4;    ///< Margen entre items

    /*! \brief Dibuja un item individual.
     *  \param index Índice del item.
     *  \param selected Si está seleccionado. */
    void draw_item(uint8_t index, bool selected);

    /*! \brief Dibuja un indicador de scroll. */
    void draw_scrollbar();
};
