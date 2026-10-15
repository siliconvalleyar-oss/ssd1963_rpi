#pragma once
#include <engine/Scene.hpp>
#include <string>

class PiStatusScene : public Scene {
public:
    explicit PiStatusScene(SSD1963& display);

    bool on_enter() override;
    void update(uint32_t dt) override;
    void draw(FrameBuffer& fb) override;
    const char* name() const override { return "PiStatusScene"; }

private:
    std::string m_ip;
    std::string m_time;
    std::string m_location;
    std::string m_btc_price;
    std::string m_temp;
    std::string m_uptime;
    std::string m_memory;

    void refresh_system_info();
    std::string exec_command(const char* cmd);
};
