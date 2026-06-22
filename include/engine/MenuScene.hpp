#pragma once
#include <engine/Scene.hpp>

#define MENU_MAX_ITEMS 8

class MenuScene : public Scene {
public:
    explicit MenuScene(SSD1963& display, const char* title);

    void add_item(const char* label, Scene* target);

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw() override;
    const char* name() const override;

private:
    const char* m_title;
    struct Item { const char* label; Scene* target; };
    Item     m_items[MENU_MAX_ITEMS];
    uint8_t  m_count;
    uint8_t  m_sel;        // item actual
    uint8_t  m_next_sel;   // item para la próxima vez que entre al menú
    uint32_t m_timer;
    bool     m_drawn;

    static constexpr uint32_t HIGHLIGHT_MS = 2500;

    void do_select();
};
