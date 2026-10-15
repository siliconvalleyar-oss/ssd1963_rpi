#include <engine/TouchTestScene.hpp>
#include <tftgl_ads7843.hpp>
#include <color.hpp>
#include <engine/GameEngine.hpp>
#include <cstdio>

static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return RGB565CONVERT(r, g, b);
}

TouchTestScene::TouchTestScene(SSD1963& display)
    : Scene(display), m_timer(0), m_drawn(false), m_last_x(0), m_last_y(0), m_touched(false) {}

bool TouchTestScene::on_enter() {
    m_timer = 0;
    m_drawn = false;
    m_touched = false;
    return true;
}

void TouchTestScene::update(uint32_t dt) {
    static ADS7843_Touch touch;
    static bool initialized = false;

    if (!initialized) {
        touch.set_calibration(TFTGL_CALIB_MIN_X, 200.0,   0);
        touch.set_calibration(TFTGL_CALIB_MAX_X, 3800.0,  LCD_WIDTH);
        touch.set_calibration(TFTGL_CALIB_MIN_Y, 200.0,   0);
        touch.set_calibration(TFTGL_CALIB_MAX_Y, 3800.0,  LCD_HEIGHT);
        touch.begin();
        bcm2835_gpio_fsel(TOUCH_INT_PIN, BCM2835_GPIO_FSEL_INPT);
        initialized = true;
    }

    m_timer += dt;
    if (m_timer > 5000) {
        if (m_engine && m_engine->menu_scene())
            m_engine->set_scene(m_engine->menu_scene());
    }

    if (!bcm2835_gpio_lev(TOUCH_INT_PIN)) {
        unsigned int x = 0, y = 0;
        if (touch.get_touch(&x, &y) == TouchStatus::GOT_TOUCH) {
            m_last_x = x;
            m_last_y = y;
            m_touched = true;
        }
    }
}

void TouchTestScene::draw(FrameBuffer& fb) {
    if (m_drawn) return;
    m_drawn = true;

    fb.clear(rgb(10, 10, 30));
    uint16_t cx = LCD_WIDTH / 2;

    fb.draw_string_centered_scaled(cx, 6, "TOUCH TEST",
                                  rgb(255, 255, 255), rgb(10, 10, 30), 3);

    fb.draw_string_centered(cx, 60,
        "Presiona la pantalla", rgb(200, 200, 255), rgb(10, 10, 30));

    fb.draw_string_centered(cx, 100, "para ver coordenadas",
        rgb(200, 200, 255), rgb(10, 10, 30));

    char buf[64];
    if (m_touched) {
        snprintf(buf, sizeof(buf), "x=%u  y=%u", m_last_x, m_last_y);
        fb.draw_string_centered(cx, 180, buf, rgb(0, 255, 0), rgb(10, 10, 30));
        fb.set_pixel(m_last_x, m_last_y, rgb(255, 0, 0));
    }

    fb.draw_string_centered(cx, 240,
        "Vuelve al menu en 5s", rgb(120, 120, 160), rgb(10, 10, 30));
}

const char* TouchTestScene::name() const { return "TouchTestScene"; }
