#pragma once
#include <cstdint>
#include <ssd1963.hpp>
#include <color.hpp>

class GameEngine;

class Scene {
public:
    explicit Scene(SSD1963& display) : m_display(display), m_engine(nullptr) {}
    virtual ~Scene() = default;

    void set_engine(GameEngine* engine) { m_engine = engine; }

    virtual bool on_enter() { return true; }
    virtual void on_exit()  {}
    virtual void update(uint32_t dt) = 0;
    virtual void draw() = 0;
    virtual const char* name() const = 0;

protected:
    SSD1963&     m_display;
    GameEngine*  m_engine;
};
