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
    int8_t       m_selected;
    uint32_t     m_timer;
    uint32_t     m_star_timer;

    static constexpr uint32_t CYCLE_MS  = 2500;
    static constexpr uint32_t SELECT_MS = 5000;

    void draw_starfield();
    void draw_cabinet_art();
    void draw_item(uint8_t index, bool selected);
    void do_select();
};
