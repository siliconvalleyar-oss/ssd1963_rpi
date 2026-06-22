#pragma once

#include <engine/Scene.hpp>

#define MENU_MAX_ITEMS 16

struct MenuItem {
    const char* label;
    Scene*      target;
    uint16_t    color;
};

/*! \class MenuScene
 *  \brief Menú interactivo con auto-demo.
 *
 *  Sin botones conectados, avanza automáticamente cada 2 segundos
 *  y selecciona la opción destacada tras 4 segundos.
 *  Con botones GPIO, navegación manual completa. */
class MenuScene : public Scene {
public:
    explicit MenuScene(SSD1963& display, const char* title = "MENU PRINCIPAL");

    void add_item(const char* label, Scene* target, uint16_t color);

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw() override;
    uint8_t handle_button(uint8_t btn) override;
    const char* name() const override;

private:
    const char*  m_title;
    MenuItem     m_items[MENU_MAX_ITEMS];
    uint8_t      m_item_count;
    int8_t       m_selected;
    uint8_t      m_scroll_offset;
    uint32_t     m_anim_timer;
    uint16_t     m_bg_color;

    // Auto-demo timing
    uint32_t     m_auto_timer;     ///< Acumulador para auto-navegación
    uint32_t     m_input_timer;    ///< Tiempo desde última interacción
    bool         m_auto_mode;      ///< true = auto-demo sin botones

    static constexpr uint8_t  VISIBLE_ITEMS    = 6;
    static constexpr uint16_t ITEM_HEIGHT       = 32;
    static constexpr uint16_t ITEM_MARGIN       = 4;
    static constexpr uint32_t AUTO_CYCLE_MS     = 2000;
    static constexpr uint32_t AUTO_SELECT_MS    = 4000;

    void draw_item(uint8_t index, bool selected);
    void draw_scrollbar();
    void do_select();
};
