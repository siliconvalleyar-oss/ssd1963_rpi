#pragma once
#include <cstdint>
#include <ssd1963.hpp>
#include <FrameBuffer.hpp>
#include <engine/Scene.hpp>
#include <config_hw.hpp>
#include <color.hpp>

#define ENGINE_VERSION VERSION_TAG

enum class EngineState : uint8_t { INIT, RUNNING, EXIT };

class GameEngine {
public:
    GameEngine();
    ~GameEngine();

    bool init();
    void run();
    void quit();

    void set_scene(Scene* scene);
    void set_menu_scene(Scene* scene);
    Scene* menu_scene() const;
    SSD1963& display();
    EngineState state() const;
    uint32_t elapsed_ms() const;

private:
    SSD1963      m_display;
    FrameBuffer  m_fb;
    Scene*       m_active_scene;
    Scene*       m_menu_scene;
    EngineState  m_state;
    uint32_t     m_start_time;

    void engine_delay(uint32_t ms);
};
