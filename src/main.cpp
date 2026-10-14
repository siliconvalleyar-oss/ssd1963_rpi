// main.cpp
//
// Programa bus TFT SSD1963 + Raspberry Pi (bus paralelo 16 bits).
//
// API base: clase SSD1963 (include/ssd1963.hpp) — versión avanzada del repo.
// Touch: driver ADS7843 vía SPI (include/tftgl_ads7843.hpp), puerto C++ de
//        tmp/tftgl_ads7843.h (autónomo, sin EGL).
//
// Merge de demos de los snapshots de tmp/ (v7/v8/v9):
//   - llenado de pantalla completa con colores básicos   [v7/v8]
//   - patrón de 8 barras verticales de color               [v8]
//   - rectángulo redondeado (helper local)                [v9]
//   - draw de imagen rgb565 desde assets/                 [HEAD]
//   - lectura de touch ADS7843                            [legacy/tftgl_ads7843.h]

#include <ssd1963.hpp>
#include <config_hw.hpp>
#include <color.hpp>
#include <tftgl_ads7843.hpp>
#include <bcm2835.h>
#include <unistd.h>
#include <cstdint>
#include <iostream>

static void delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

// ---------------------------------------------------------------------------
// Merge v9: rectángulo redondeado. La clase SSD1963 del repo no lo expone,
// así que se implementa como helper local (sin tocar ssd1963.cpp/.hpp).
// ---------------------------------------------------------------------------
static void draw_rounded_rect(SSD1963& lcd,
                              int x, int y, int w, int h, int radius,
                              uint16_t fill_color, uint16_t border_color,
                              int border_thickness) {
    // Relleno central
    for (int i = y + radius; i < y + h - radius; i++) {
        lcd.fill_rect(x + radius, static_cast<uint16_t>(i),
                      static_cast<uint16_t>(w - 2 * radius), 1, fill_color);
    }

    // Rellenar lados rectos
    for (int i = 0; i < radius; i++) {
        lcd.fill_rect(x + radius, static_cast<uint16_t>(y + i),
                      static_cast<uint16_t>(w - 2 * radius), 1, fill_color);
        lcd.fill_rect(x + radius, static_cast<uint16_t>(y + h - 1 - i),
                      static_cast<uint16_t>(w - 2 * radius), 1, fill_color);
    }

    // Laterales
    for (int i = y + radius; i < y + h - radius; i++) {
        for (int t = 0; t < radius; t++) {
            lcd.draw_pixel(static_cast<uint16_t>(x + t),       static_cast<uint16_t>(i), fill_color);
            lcd.draw_pixel(static_cast<uint16_t>(x + w - 1 - t), static_cast<uint16_t>(i), fill_color);
        }
    }

    // Esquinas redondeadas
    for (int i = 0; i < radius; i++) {
        for (int j = 0; j < radius; j++) {
            if ((i * i + j * j) <= (radius * radius)) {
                lcd.draw_pixel(static_cast<uint16_t>(x + i),         static_cast<uint16_t>(y + j),              fill_color);
                lcd.draw_pixel(static_cast<uint16_t>(x + w - 1 - i), static_cast<uint16_t>(y + j),              fill_color);
                lcd.draw_pixel(static_cast<uint16_t>(x + i),         static_cast<uint16_t>(y + h - 1 - j),       fill_color);
                lcd.draw_pixel(static_cast<uint16_t>(x + w - 1 - i), static_cast<uint16_t>(y + h - 1 - j),       fill_color);
            }
        }
    }

    // Bordes
    if (border_thickness > 0) {
        for (int t = 0; t < border_thickness; t++) {
            lcd.fill_rect(static_cast<uint16_t>(x + t),         static_cast<uint16_t>(y + t),
                          static_cast<uint16_t>(w - 2 * t), 1, border_color);
            lcd.fill_rect(static_cast<uint16_t>(x + t),         static_cast<uint16_t>(y + h - 1 - t),
                          static_cast<uint16_t>(w - 2 * t), 1, border_color);
            lcd.fill_rect(static_cast<uint16_t>(x + t),         static_cast<uint16_t>(y + t),
                          1, static_cast<uint16_t>(h - 2 * t), border_color);
            lcd.fill_rect(static_cast<uint16_t>(x + w - 1 - t), static_cast<uint16_t>(y + t),
                          1, static_cast<uint16_t>(h - 2 * t), border_color);
        }
    }
}

// ---------------------------------------------------------------------------
// Merge v8: test de pantalla completa (llenado de cada color base)
// ---------------------------------------------------------------------------
static void test_full_screen(SSD1963& lcd) {
    std::cout << "\n=== PRUEBA DE PANTALLA COMPLETA ===" << std::endl;
    const uint16_t fills[] = {BLACK, RED, GREEN, BLUE, WHITE};
    const char* names[]  = {"Negro", "Rojo", "Verde", "Azul", "Blanco"};
    for (int i = 0; i < 5; i++) {
        std::cout << names[i] << "..." << std::endl;
        lcd.clear_screen(fills[i]);
        delay_ms(1500);
    }
}

// ---------------------------------------------------------------------------
// Merge v8: patron de barras de color verticales (8 barras)
// ---------------------------------------------------------------------------
static void test_color_bars(SSD1963& lcd) {
    std::cout << "\n=== BARRAS DE COLOR ===" << std::endl;
    lcd.clear_screen(BLACK);
    delay_ms(500);

    const uint16_t bars[] = {RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE, BLACK};
    const uint16_t bar_width = static_cast<uint16_t>(LCD_WIDTH / 8);

    for (int i = 0; i < 8; i++) {
        uint16_t x = static_cast<uint16_t>(i * (LCD_WIDTH / 8));
        uint16_t w = (i == 7) ? static_cast<uint16_t>(LCD_WIDTH - x) : bar_width;
        lcd.fill_rect(x, 0, w, LCD_HEIGHT, bars[i]);
    }
    delay_ms(5000);
}

// ---------------------------------------------------------------------------
// Demo de touch ADS7843: lee muestras y dibuja el punto tocado.
// ---------------------------------------------------------------------------
static void test_touch(SSD1963& lcd, ADS7843_Touch& touch) {
    std::cout << "\n=== TOUCH ADS7843 (Ctrl+C para salir) ===" << std::endl;
    // Calibración de ejemplo (raw -> pixel). Ajustar a la pantalla real.
    touch.set_calibration(TFTGL_CALIB_MIN_X, 200.0,   0);
    touch.set_calibration(TFTGL_CALIB_MAX_X, 3800.0,  LCD_WIDTH);
    touch.set_calibration(TFTGL_CALIB_MIN_Y, 200.0,   0);
    touch.set_calibration(TFTGL_CALIB_MAX_Y, 3800.0,  LCD_HEIGHT);

    for (int i = 0; i < 20; i++) {
        unsigned int x = 0, y = 0, z = 0;
        touch.get_touch_raw(&x, &y, &z);
        if (touch.get_touch(&x, &y) == TouchStatus::GOT_TOUCH) {
            std::cout << "touch: x=" << x << " y=" << y << std::endl;
            lcd.draw_pixel(x, y, RED);
        } else {
            std::cout << "sin touch (z=" << z << ")" << std::endl;
        }
        delay_ms(200);
    }
}

int main() {
    if (!bcm2835_init()) {
        std::cerr << "Error al inicializar bcm2835" << std::endl;
        return 1;
    }

    SSD1963 ssd1963;
    ssd1963.setup_gpio();
    ssd1963.init();

    // Pantalla en BLANCO tras la inicialización del SSD1963
    ssd1963.clear_screen(WHITE);
    delay_ms(500);

    // Demos visuales (merge v8/v9/HEAD)
    test_full_screen(ssd1963);
    test_color_bars(ssd1963);

    std::cout << "\n=== RECTANGULO REDONDEADO ===" << std::endl;
    draw_rounded_rect(ssd1963, 20, 20, 96, 96, 12, GREEN, WHITE, 2);
    delay_ms(5000);

    std::cout << "\n=== BLOQUES DE COLOR ===" << std::endl;
    ssd1963.clear_screen(BLACK);
    const uint16_t block_w = 45, block_h = 45;
    const uint16_t colors[] = {RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE};
    const char* names[]  = {"RED","GREEN","BLUE","YELLOW","CYAN","MAGENTA","WHITE"};
    uint16_t x = 10;
    for (int i = 0; i < 7; i++) {
        std::cout << "bloque: " << names[i] << " (" << x << ",10)" << std::endl;
        ssd1963.fill_rect(x, 10, block_w, block_h, colors[i]);
        x += block_w + 10;
    }
    delay_ms(3000);

    // Driver de touch ADS7843 (puerto de tmp/tftgl_ads7843.h)
    {
        ADS7843_Touch touch;
        if (touch.begin() == TouchStatus::OK) {
            ssd1963.clear_screen(BLACK);
            test_touch(ssd1963, touch);
        } else {
            std::cerr << "Touch ADS7843 no disponible" << std::endl;
        }
    }

    // Imagen rgb565 desde assets/ (merge HEAD)
    std::cout << "\n=== IMAGEN capibaras.rgb565 ===" << std::endl;
    ssd1963.draw_image_rgb565("assets/capibaras.rgb565");

    bcm2835_close();
    return 0;
}
