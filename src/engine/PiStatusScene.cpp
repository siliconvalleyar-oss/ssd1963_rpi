#include <engine/PiStatusScene.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <engine/GameEngine.hpp>

static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return RGB565CONVERT(r, g, b);
}

PiStatusScene::PiStatusScene(SSD1963& display)
    : Scene(display), m_ip("?"), m_time("?"), m_location("?"),
      m_btc_price("?"), m_temp("?"), m_uptime("?") {}

bool PiStatusScene::on_enter() {
    refresh_system_info();
    return true;
}

void PiStatusScene::update(uint32_t dt) {
    static uint32_t timer = 0;
    timer += dt;
    if (timer > 10000) {  // Refresh every 10s
        timer = 0;
        refresh_system_info();
    }
    if (timer > 30000) {  // Auto-return after 30s
        if (m_engine && m_engine->menu_scene())
            m_engine->set_scene(m_engine->menu_scene());
    }
}

std::string PiStatusScene::exec_command(const char* cmd) {
    char buffer[256];
    FILE* fp = popen(cmd, "r");
    if (!fp) return "";
    if (fgets(buffer, sizeof(buffer), fp) != nullptr) {
        // Remove trailing newline
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r')) {
            buffer[--len] = '\0';
        }
    } else {
        buffer[0] = '\0';
    }
    pclose(fp);
    return std::string(buffer);
}

void PiStatusScene::refresh_system_info() {
    // Get IP address
    m_ip = exec_command("hostname -I 2>/dev/null");
    if (m_ip.empty() || m_ip.find('.') == std::string::npos) {
        m_ip = "offline";
    }

    // Get current time
    m_time = exec_command("date '+%H:%M:%S' 2>/dev/null");
    if (m_time.empty()) m_time = "--:--:--";

    // Get location (from GPS or fallback to city based on IP)
    m_location = exec_command("cat /tmp/location.txt 2>/dev/null");
    if (m_location.empty()) {
        m_location = "N:?,E:?";
    }

    // Get Bitcoin price
    m_btc_price = exec_command("curl -s --connect-timeout 5 'https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd' 2>/dev/null | head -c 50");
    if (m_btc_price.empty() || m_btc_price.find("bitcoin") == std::string::npos) {
        // Try parsing JSON response
        if (m_btc_price.find("usd") != std::string::npos) {
            m_btc_price = "$" + m_btc_price;
        } else {
            m_btc_price = "N/A";
        }
    } else {
        // Extract USD value from JSON
        size_t pos = m_btc_price.find("usd");
        if (pos != std::string::npos) {
            pos = m_btc_price.find(':', pos);
            if (pos != std::string::npos) {
                m_btc_price = "$" + exec_command(("echo " + m_btc_price.substr(pos+1)).c_str());
            }
        }
    }

    // Get CPU temperature
    m_temp = exec_command("vcgencmd measure_temp 2>/dev/null | cut -d= -f2 | tr -d '\\n'");
    if (m_temp.empty()) m_temp = "?.?C";

    // Get uptime
    m_uptime = exec_command("awk '{print int($1/3600)\"h \"int(($1%3600)/60)\"m\"}' /proc/uptime 2>/dev/null");
    if (m_uptime.empty()) m_uptime = "0h 0m";
}

void PiStatusScene::draw(FrameBuffer& fb) {
    fb.clear(rgb(0, 0, 20));
    uint16_t cx = LCD_WIDTH / 2;

    fb.draw_string_centered_scaled(cx, 4, "SISTEMA", rgb(255, 255, 0), rgb(0, 0, 20), 2);

    uint16_t y = 40;
    int label_w = 100;

    // IP address
    fb.draw_string(10, y, "IP:", rgb(200, 200, 255), rgb(0, 0, 20));
    fb.draw_string(10 + label_w, y, m_ip.c_str(), rgb(0, 255, 0), rgb(0, 0, 20));
    y += 14;

    // Time
    fb.draw_string(10, y, "Hora:", rgb(200, 200, 255), rgb(0, 0, 20));
    fb.draw_string(10 + label_w, y, m_time.c_str(), rgb(0, 255, 0), rgb(0, 0, 20));
    y += 14;

    // Temperature
    fb.draw_string(10, y, "Temp:", rgb(200, 200, 255), rgb(0, 0, 20));
    fb.draw_string(10 + label_w, y, m_temp.c_str(), rgb(255, 100, 0), rgb(0, 0, 20));
    y += 14;

    // Uptime
    fb.draw_string(10, y, "Uptime:", rgb(200, 200, 255), rgb(0, 0, 20));
    fb.draw_string(10 + label_w, y, m_uptime.c_str(), rgb(0, 255, 0), rgb(0, 0, 20));
    y += 14;

    // Bitcoin price
    fb.draw_string(10, y, "BTC:", rgb(200, 200, 255), rgb(0, 0, 20));
    fb.draw_string(10 + label_w, y, m_btc_price.c_str(), rgb(255, 215, 0), rgb(0, 0, 20));
    y += 14;

    // Location
    fb.draw_string(10, y, "Loc:", rgb(200, 200, 255), rgb(0, 0, 20));
    fb.draw_string(10 + label_w, y, m_location.c_str(), rgb(100, 200, 255), rgb(0, 0, 20));
    y += 20;

    // Footer
    fb.draw_string_centered(cx, LCD_HEIGHT - 12,
        "ACTUALIZA CADA 10s", rgb(120, 120, 160), rgb(0, 0, 20));
}
