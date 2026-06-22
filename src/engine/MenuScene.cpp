#include <engine/MenuScene.hpp>
#include <engine/GameEngine.hpp>
#include <cstring>

static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return RGB565CONVERT(r, g, b);
}

MenuScene::MenuScene(SSD1963& display, const char* title)
    : Scene(display)
    , m_title(title)
    , m_item_count(0)
    , m_selected(0)
    , m_timer(0)
    , m_last_cycle(0)
    , m_needs_redraw(true)
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
    m_timer = 0;
    m_last_cycle = 0;
    m_needs_redraw = true;
    return true;
}

void MenuScene::update(uint32_t dt) {
    m_timer += dt;

    // --- Auto-cycle: avanzar highlight cada CYCLE_MS ---
    uint8_t cycle_count = m_timer / CYCLE_MS;
    if (cycle_count > m_last_cycle) {
        m_last_cycle = cycle_count;
        m_selected = (m_selected + 1) % m_item_count;
        m_needs_redraw = true;
        std::cout << "[Menu] item " << (int)m_selected
                  << ": " << m_items[m_selected].label << "\n";
    }

    // --- Auto-select: lanzar demo tras SELECT_MS ---
    if (m_timer >= SELECT_MS) {
        std::cout << "[Menu] auto-select item " << (int)m_selected << "\n";
        m_timer = 0;
        m_last_cycle = 0;
        do_select();
    }
}

void MenuScene::draw() {
    // Optimización: no redibujar cada frame, solo en cambios
    if (!m_needs_redraw) return;
    m_needs_redraw = false;

    // Fondo: starfield
    draw_starfield();

    // Decoración arcade
    draw_cabinet_art();

    // Título
    draw_title();

    // Items del menú
    uint16_t start_y = 44;
    for (uint8_t i = 0; i < m_item_count; i++) {
        draw_item(i, i == m_selected);
    }

    // Footer
    draw_footer();
}

const char* MenuScene::name() const { return "MenuScene"; }

// ----------------------------------------------------------------
// Secciones de dibujo
// ----------------------------------------------------------------
void MenuScene::draw_starfield() {
    static uint8_t seed = 0;
    seed = (seed + 1) % 255;

    m_display.fill_rect(0, 0, LCD_WIDTH, LCD_HEIGHT, rgb(8, 8, 28));

    for (int i = 0; i < 60; i++) {
        uint16_t sx = (i * 137 + seed * 3) % LCD_WIDTH;
        uint16_t sy = (i * 251 + seed * 7) % LCD_HEIGHT;
        uint8_t  br = 80 + ((i * 13 + seed) % 60);
        m_display.draw_pixel(sx, sy, rgb(br / 3, br / 3, br));
    }
}

void MenuScene::draw_title() {
    uint16_t cx = LCD_WIDTH / 2;
    // Sombra
    m_display.draw_string_centered(cx, 14, m_title, rgb(200, 150, 0),   rgb(8, 8, 28));
    m_display.draw_string_centered(cx, 13, m_title, rgb(255, 220, 80),  rgb(8, 8, 28));
    m_display.draw_string_centered(cx, 12, m_title, rgb(255, 255, 200), rgb(8, 8, 28));
}

void MenuScene::draw_cabinet_art() {
    // Barras laterales decorativas
    for (uint16_t y = 32; y < LCD_HEIGHT - 20; y++) {
        m_display.draw_pixel(2,  y, rgb(255, 0, 80));
        m_display.draw_pixel(3,  y, rgb(200, 0, 60));
        m_display.draw_pixel(LCD_WIDTH - 3, y, rgb(0, 200, 255));
        m_display.draw_pixel(LCD_WIDTH - 4, y, rgb(0, 150, 200));
    }
    // Separador neón
    for (uint16_t x = 30; x < LCD_WIDTH - 30; x++) {
        uint16_t c = ((x / 4) % 2) ? rgb(255, 0, 128) : rgb(0, 200, 255);
        m_display.draw_pixel(x, 30, c);
    }
}

void MenuScene::draw_item(uint8_t index, bool selected) {
    uint16_t iy = 44 + index * 38;
    uint16_t ix = 16;
    uint16_t iw = LCD_WIDTH - 32;
    uint16_t col = m_items[index].color;

    if (selected) {
        // Fondo del item seleccionado con borde pulsante
        uint16_t glow = 120 + ((m_timer / 3) % 80);
        m_display.draw_rect(ix, iy, iw, 30, rgb(glow, glow, 255));
        m_display.fill_rect(ix + 1, iy + 1, iw - 2, 28, rgb(20, 20, 60));
    } else {
        // Fondo del item no seleccionado (sutil)
        m_display.fill_rect(ix, iy, iw, 30, rgb(12, 12, 35));
    }

    // Caja de color (cartucho)
    m_display.fill_rect(ix + 4, iy + 4, 22, 22, col);
    m_display.draw_rect(ix + 4, iy + 4, 22, 22, selected ? WHITE : rgb(60, 60, 80));

    // Nombre del item
    uint16_t text_col = selected ? WHITE : rgb(180, 180, 200);
    m_display.draw_string(ix + 32, iy + 8, m_items[index].label, text_col,
                          selected ? rgb(20, 20, 60) : rgb(12, 12, 35));

    if (selected) {
        uint16_t pulse = 128 + ((m_timer / 2) % 128);
        m_display.draw_string(ix + iw - 14, iy + 8, ">",
                              rgb(pulse, pulse, 255), rgb(20, 20, 60));
    }
}

void MenuScene::draw_footer() {
    uint16_t cx = LCD_WIDTH / 2;
    m_display.draw_string_centered(cx, LCD_HEIGHT - 14,
                                   "AUTO-DEMO MODE",
                                   rgb(80, 80, 120), rgb(8, 8, 28));

    // Tick de progreso visual
    uint8_t ticks = (m_timer * 8) / SELECT_MS;
    for (uint8_t i = 0; i < 8; i++) {
        uint16_t tx = cx - 28 + i * 8;
        uint16_t tc = (i < ticks) ? rgb(255, 200, 0) : rgb(40, 40, 60);
        m_display.fill_rect(tx, LCD_HEIGHT - 6, 6, 4, tc);
    }
}

void MenuScene::do_select() {
    if (m_items[m_selected].target != nullptr) {
        if (m_engine) {
            m_engine->set_scene(m_items[m_selected].target);
        }
    } else {
        if (std::strcmp(m_items[m_selected].label, "SALIR") == 0) {
            if (m_engine) m_engine->quit();
        }
    }
}
