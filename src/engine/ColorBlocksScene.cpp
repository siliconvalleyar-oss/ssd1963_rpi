#include <engine/ColorBlocksScene.hpp>
#include <engine/GameEngine.hpp>
#include <cstdio>

ColorBlocksScene::ColorBlocksScene(SSD1963& display)
    : Scene(display)
    , m_blocks_drawn(false)
    , m_image_drawn(false)
    , m_timer(0)
    , m_phase(0)
{
    m_blocks[0] = {  10, 10, RED,     "ROJO"    };
    m_blocks[1] = {  80, 10, GREEN,   "VERDE"   };
    m_blocks[2] = { 150, 10, BLUE,    "AZUL"    };
    m_blocks[3] = { 220, 10, YELLOW,  "AMARILLO"};
    m_blocks[4] = { 290, 10, CYAN,    "CIAN"    };
    m_blocks[5] = { 360, 10, MAGENTA, "MAGENTA" };
    m_blocks[6] = { 430, 10, WHITE,   "BLANCO"  };
}

bool ColorBlocksScene::on_enter() {
    m_blocks_drawn = false;
    m_image_drawn = false;
    m_timer = 0;
    m_phase = 0;
    m_display.clear_screen(BLACK);
    return true;
}

void ColorBlocksScene::update(uint32_t dt) {
    m_timer += dt;

    if (m_phase == 0 && m_blocks_drawn && m_timer > PHASE_BLOCKS_MS) {
        m_phase = 1;
        m_timer = 0;
    }

    // Auto-retorno al menú
    if (m_phase == 1 && m_image_drawn && m_timer > PHASE_IMAGE_MS) {
        if (m_engine && m_engine->menu_scene()) {
            m_engine->set_scene(m_engine->menu_scene());
        }
    }
}

void ColorBlocksScene::draw() {
    if (!m_blocks_drawn) {
        for (uint8_t i = 0; i < 7; i++) {
            m_display.draw_block(m_blocks[i].x, m_blocks[i].y,
                                 BLOCK_W, BLOCK_H, m_blocks[i].color);
            m_display.draw_string(m_blocks[i].x + 2,
                                  m_blocks[i].y + BLOCK_H + 4,
                                  m_blocks[i].name,
                                  m_blocks[i].color, BLACK);
        }
        m_blocks_drawn = true;
        std::cout << "[ColorBlocks] 7 bloques de color dibujados\n";
    }

    if (m_phase == 1 && !m_image_drawn) {
        m_display.draw_image_rgb565("assets/capibaras.rgb565");
        m_image_drawn = true;
        std::cout << "[ColorBlocks] Imagen mostrada\n";
    }
}

uint8_t ColorBlocksScene::handle_button(uint8_t btn) {
    if (btn == static_cast<uint8_t>(Button::SELECT) ||
        btn == static_cast<uint8_t>(Button::BACK)) {
        if (m_engine && m_engine->menu_scene()) {
            m_engine->set_scene(m_engine->menu_scene());
        }
    }
    return 0;
}

const char* ColorBlocksScene::name() const {
    return "ColorBlocksScene";
}
