#include <engine/MenuScene.hpp>
#include <engine/GameEngine.hpp>
#include <cstring>
#include <cstdio>

MenuScene::MenuScene(SSD1963& display, const char* title)
    : Scene(display)
    , m_title(title)
    , m_item_count(0)
    , m_selected(0)
    , m_scroll_offset(0)
    , m_anim_timer(0)
    , m_bg_color(RGB565CONVERT(20, 20, 30))
{
    std::memset(m_items, 0, sizeof(m_items));
}

void MenuScene::add_item(const char* label, Scene* target, uint16_t color) {
    if (m_item_count >= MENU_MAX_ITEMS) return;
    m_items[m_item_count].label  = label;
    m_items[m_item_count].target = target;
    m_items[m_item_count].color  = color;
    m_item_count++;
}

bool MenuScene::on_enter() {
    m_selected = 0;
    m_scroll_offset = 0;
    m_anim_timer = 0;
    return true;
}

void MenuScene::on_exit() {}

void MenuScene::update(uint32_t dt) {
    m_anim_timer += dt;
}

void MenuScene::draw() {
    m_display.clear_screen(m_bg_color);

    uint16_t center_x = LCD_WIDTH / 2;

    // Título
    m_display.draw_string_centered(center_x, 8, m_title, CYAN, m_bg_color);

    // Línea separadora
    for (uint16_t x = 20; x < LCD_WIDTH - 20; x++) {
        m_display.draw_pixel(x, 28, RGB565CONVERT(60, 60, 80));
    }

    // Items visibles
    uint8_t start = m_scroll_offset;
    uint8_t end = m_scroll_offset + VISIBLE_ITEMS;
    if (end > m_item_count) end = m_item_count;

    for (uint8_t i = start; i < end; i++) {
        draw_item(i, (i == static_cast<uint8_t>(m_selected)));
    }

    draw_scrollbar();
}

uint8_t MenuScene::handle_button(uint8_t btn) {
    switch (btn) {
        case static_cast<uint8_t>(Button::UP):
            if (m_selected > 0) {
                m_selected--;
                if (m_selected < m_scroll_offset) {
                    m_scroll_offset = m_selected;
                }
            }
            break;

        case static_cast<uint8_t>(Button::DOWN):
            if (m_selected < m_item_count - 1) {
                m_selected++;
                if (m_selected >= m_scroll_offset + VISIBLE_ITEMS) {
                    m_scroll_offset = m_selected - VISIBLE_ITEMS + 1;
                }
            }
            break;

        case static_cast<uint8_t>(Button::SELECT):
            if (m_items[m_selected].target != nullptr) {
                return MENU_ACTION_CHANGE;
            }
            break;

        case static_cast<uint8_t>(Button::BACK):
            return MENU_ACTION_EXIT;

        default:
            break;
    }
    return MENU_ACTION_NONE;
}

const char* MenuScene::name() const {
    return "MenuScene";
}

void MenuScene::draw_item(uint8_t index, bool selected) {
    uint16_t y = 36 + (index - m_scroll_offset) * (ITEM_HEIGHT + ITEM_MARGIN);
    uint16_t item_x = 15;
    uint16_t item_w = LCD_WIDTH - 30;
    uint16_t color = m_items[index].color;

    if (selected) {
        m_display.draw_rect(item_x, y, item_w, ITEM_HEIGHT, RGB565CONVERT(60, 100, 180));
        m_display.fill_rect(item_x + 1, y + 1, item_w - 2, ITEM_HEIGHT - 2,
                            RGB565CONVERT(40, 70, 140));
    }

    uint16_t box_size = ITEM_HEIGHT - 8;
    m_display.fill_rect(item_x + 6, y + 4, box_size, box_size, color);
    m_display.draw_rect(item_x + 6, y + 4, box_size, box_size, WHITE);

    m_display.draw_string(item_x + box_size + 14,
                          y + (ITEM_HEIGHT - 8) / 2,
                          m_items[index].label,
                          selected ? WHITE : RGB565CONVERT(200, 200, 200),
                          selected ? RGB565CONVERT(40, 70, 140) : m_bg_color);

    if (selected) {
        uint16_t pulse = (m_anim_timer / 4) % 255;
        uint16_t indicator_x = item_x + item_w - 14;
        uint16_t indicator_color = RGB565CONVERT(pulse, pulse, 255);
        m_display.draw_string(indicator_x, y + (ITEM_HEIGHT - 8) / 2, ">", indicator_color,
                              RGB565CONVERT(40, 70, 140));
    }
}

void MenuScene::draw_scrollbar() {
    if (m_item_count <= VISIBLE_ITEMS) return;

    uint16_t sb_x = LCD_WIDTH - 6;
    uint16_t sb_y = 36;
    uint16_t sb_h = VISIBLE_ITEMS * (ITEM_HEIGHT + ITEM_MARGIN);

    m_display.draw_rect(sb_x, sb_y, 4, sb_h, RGB565CONVERT(60, 60, 80));

    float thumb_ratio = static_cast<float>(VISIBLE_ITEMS) / m_item_count;
    float pos_ratio = static_cast<float>(m_scroll_offset) / (m_item_count - VISIBLE_ITEMS);
    uint16_t thumb_h = static_cast<uint16_t>(sb_h * thumb_ratio);
    uint16_t thumb_y = sb_y + static_cast<uint16_t>(pos_ratio * (sb_h - thumb_h));

    m_display.fill_rect(sb_x, thumb_y, 4, thumb_h, RGB565CONVERT(100, 150, 255));
}
