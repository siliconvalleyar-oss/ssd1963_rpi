#include <engine/GameEngine.hpp>
#include <bcm2835.h>
#include <unistd.h>
#include <iostream>

// Pines GPIO para botones (no conflictivos con el display)
#define BTN_UP     RPI_GPIO_P1_26
#define BTN_DOWN   RPI_GPIO_P1_24
#define BTN_SELECT RPI_GPIO_P1_21
#define BTN_BACK   RPI_GPIO_P1_19

GameEngine::GameEngine()
    : m_active_scene(nullptr)
    , m_state(EngineState::INIT)
    , m_start_time(0)
    , m_buttons_enabled(false)
{
}

GameEngine::~GameEngine() {
    if (m_buttons_enabled) {
        enable_buttons(false);
    }
}

bool GameEngine::init() {
    if (!bcm2835_init()) {
        std::cerr << "[ENGINE] Error: bcm2835_init() failed\n";
        return false;
    }

    m_display.setup_gpio();
    m_display.init();
    m_display.clear_screen(BLACK);

    m_start_time = bcm2835_st_read();
    m_state = EngineState::RUNNING;

    std::cout << "[ENGINE] SSD1963 Game Engine v" << ENGINE_VERSION << " initialized\n";
    std::cout << "[ENGINE] Display: " << LCD_WIDTH << "x" << LCD_HEIGHT << " @ RGB565\n";
    return true;
}

void GameEngine::run() {
    if (m_state != EngineState::RUNNING) return;

    uint32_t last_time = bcm2835_st_read();
    const uint32_t FRAME_TIME = 33; // ~30 FPS

    while (m_state == EngineState::RUNNING) {
        uint32_t now = bcm2835_st_read();
        uint32_t dt = (now - last_time) / 1000;

        if (dt < FRAME_TIME) {
            engine_delay(FRAME_TIME - dt);
            now = bcm2835_st_read();
            dt = (now - last_time) / 1000;
        }
        last_time = now;

        process_input();

        if (m_active_scene) {
            m_active_scene->update(dt);
            m_active_scene->draw();
        }
    }

    std::cout << "[ENGINE] Shutting down.\n";
    bcm2835_close();
}

void GameEngine::quit() {
    m_state = EngineState::EXIT;
}

void GameEngine::set_scene(Scene* scene) {
    if (m_active_scene) {
        m_active_scene->on_exit();
    }
    m_active_scene = scene;
    if (m_active_scene) {
        std::cout << "[ENGINE] Entering scene: " << m_active_scene->name() << "\n";
        m_active_scene->on_enter();
    }
}

SSD1963& GameEngine::display() {
    return m_display;
}

Button GameEngine::read_buttons() {
    if (!m_buttons_enabled) return Button::NONE;

    if (bcm2835_gpio_lev(BTN_UP) == LOW)     return Button::UP;
    if (bcm2835_gpio_lev(BTN_DOWN) == LOW)   return Button::DOWN;
    if (bcm2835_gpio_lev(BTN_SELECT) == LOW)  return Button::SELECT;
    if (bcm2835_gpio_lev(BTN_BACK) == LOW)    return Button::BACK;

    return Button::NONE;
}

EngineState GameEngine::state() const {
    return m_state;
}

uint32_t GameEngine::elapsed_ms() const {
    return (bcm2835_st_read() - m_start_time) / 1000;
}

void GameEngine::enable_buttons(bool enable) {
    if (enable) {
        bcm2835_gpio_fsel(BTN_UP,     BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_fsel(BTN_DOWN,   BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_fsel(BTN_SELECT, BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_fsel(BTN_BACK,   BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_set_pud(BTN_UP,     BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(BTN_DOWN,   BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(BTN_SELECT, BCM2835_GPIO_PUD_UP);
        bcm2835_gpio_set_pud(BTN_BACK,   BCM2835_GPIO_PUD_UP);
        m_buttons_enabled = true;
    } else {
        m_buttons_enabled = false;
    }
}

void GameEngine::process_input() {
    if (!m_active_scene) return;

    Button btn = read_buttons();
    if (btn != Button::NONE) {
        uint8_t action = m_active_scene->handle_button(static_cast<uint8_t>(btn));
        if (action == MENU_ACTION_EXIT) {
            quit();
        }
        engine_delay(200);
    }
}

void GameEngine::engine_delay(uint32_t ms) {
    bcm2835_delay(ms);
}
