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
    uint8_t  m_target;   // item a seleccionar esta vez
    uint8_t  m_next;     // item para la próxima vez
    uint8_t  m_cur;      // item actualmente destacado en pantalla
    uint32_t m_timer;
    bool     m_drawn;

    enum Phase { CYCLE, HOLD, DONE };
    Phase    m_phase;

    static constexpr uint32_t CYCLE_MS  = 600;
    static constexpr uint32_t HOLD_MS   = 1500;

    void do_select();
};
