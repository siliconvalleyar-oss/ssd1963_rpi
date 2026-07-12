#include <engine/GameEngine.hpp>
#include <bcm2835.h>
#include <iostream>

GameEngine::GameEngine()
    : m_fb()
    , m_active_scene(nullptr)
    , m_menu_scene(nullptr)
    , m_state(EngineState::INIT)
    , m_start_time(0)
{}

GameEngine::~GameEngine() {}

bool GameEngine::init() {
    if (!bcm2835_init()) {
        std::cerr << "[ENGINE] Error: bcm2835_init() failed\n";
        return false;
    }
    m_display.setup_gpio();
    m_display.init();
    m_fb.clear(BLACK);
    m_fb.flush(m_display);

    m_start_time = bcm2835_st_read();
    m_state = EngineState::RUNNING;

    std::cout << "[ENGINE] Arcade " << ENGINE_VERSION
              << " | " << LCD_WIDTH << "x" << LCD_HEIGHT << "\n";
    return true;
}

void GameEngine::run() {
    if (m_state != EngineState::RUNNING) return;
    uint32_t last_time = bcm2835_st_read();
    const uint32_t FRAME_TIME = 33;

    while (m_state == EngineState::RUNNING) {
        uint32_t now = bcm2835_st_read();
        uint32_t dt = (now - last_time) / 1000;
        if (dt < FRAME_TIME) {
            engine_delay(FRAME_TIME - dt);
            now = bcm2835_st_read();
            dt = (now - last_time) / 1000;
        }
        last_time = now;

        if (m_active_scene) {
            m_active_scene->update(dt);
            m_active_scene->draw(m_fb);
            m_fb.flush(m_display);
        }
    }
    std::cout << "[ENGINE] Shutdown.\n";
    bcm2835_close();
}

void GameEngine::quit() { m_state = EngineState::EXIT; }

void GameEngine::set_scene(Scene* scene) {
    if (scene == m_active_scene) return;
    if (m_active_scene) {
        std::cout << "[ENGINE] exit  " << m_active_scene->name() << "\n";
        m_active_scene->on_exit();
    }
    m_active_scene = scene;
    if (m_active_scene) {
        m_active_scene->set_engine(this);
        std::cout << "[ENGINE] enter " << m_active_scene->name() << "\n";
        m_active_scene->on_enter();
    }
}

void GameEngine::set_menu_scene(Scene* scene) { m_menu_scene = scene; }
Scene* GameEngine::menu_scene() const { return m_menu_scene; }
SSD1963& GameEngine::display() { return m_display; }
EngineState GameEngine::state() const { return m_state; }
uint32_t GameEngine::elapsed_ms() const {
    return (bcm2835_st_read() - m_start_time) / 1000;
}
void GameEngine::engine_delay(uint32_t ms) { bcm2835_delay(ms); }
