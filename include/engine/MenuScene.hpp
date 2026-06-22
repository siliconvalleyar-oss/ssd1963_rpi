#pragma once
#include <engine/Scene.hpp>

#define MENU_MAX_ITEMS 8

struct MenuItem {
    const char* label;
    Scene*      target;
    uint16_t    color;
};

class MenuScene : public Scene {
public:
    explicit MenuScene(SSD1963& display, const char* title = "ARCADE PHOTO");

    void add_item(const char* label, Scene* target, uint16_t color);

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw() override;
    const char* name() const override;

private:
    const char*  m_title;
    MenuItem     m_items[MENU_MAX_ITEMS];
    uint8_t      m_item_count;
    uint8_t      m_selected;       // item seleccionado actualmente
    uint32_t     m_timer;          // temporizador global del menú
    uint8_t      m_last_cycle;     // último ciclo completado (para detectar cruce)
    bool         m_needs_redraw;   // flag para evitar redibujar cada frame

    static constexpr uint32_t CYCLE_MS  = 2500;  // tiempo entre cada item
    static constexpr uint32_t SELECT_MS = 5000;  // tiempo total antes de auto-seleccionar

    void draw_starfield();
    void draw_title();
    void draw_cabinet_art();
    void draw_item(uint8_t index, bool selected);
    void draw_footer();
    void do_select();
};
