#include <engine/GameEngine.hpp>
#include <unistd.h>
#include <iostream>
#include <cstdint>
#include <ctime>

GameEngine::GameEngine()
    : m_fb()
    , m_active_scene(nullptr)
    , m_menu_scene(nullptr)
    , m_state(EngineState::INIT)
    , m_start_time(0)
{}

GameEngine::~GameEngine() {}

static uint64_t get_time_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;
}

bool GameEngine::init() {
    m_display.setup_gpio();
    m_display.init();
    m_fb.clear(BLACK);
    m_fb.flush(m_display);

    m_start_time = get_time_us();
    m_state = EngineState::RUNNING;

    std::cout << "[ENGINE] Arcade " << ENGINE_VERSION
              << " | " << LCD_WIDTH << "x" << LCD_HEIGHT << "\n";
    return true;
}

void GameEngine::run() {
    if (m_state != EngineState::RUNNING) return;
    uint64_t last_time = get_time_us();
    const uint32_t FRAME_TIME = 33333;  // microseconds

    while (m_state == EngineState::RUNNING) {
        uint64_t now = get_time_us();
        uint64_t dt_us = now - last_time;
        if (dt_us < FRAME_TIME) {
            usleep(FRAME_TIME - dt_us);
            now = get_time_us();
            dt_us = now - last_time;
        }
        last_time = now;

        if (m_active_scene) {
            m_active_scene->update(dt_us / 1000);
            m_active_scene->draw(m_fb);
            m_fb.flush(m_display);
        }
    }
    std::cout << "[ENGINE] Shutdown.\n";
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
    return (get_time_us() - m_start_time) / 1000;
}
void GameEngine::engine_delay(uint32_t ms) { usleep(ms * 1000); }