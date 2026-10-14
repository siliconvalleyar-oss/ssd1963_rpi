#include <bcm2835.h>
#include <iostream>
#include <ssd1963.hpp>
#include <color_rgb565.hpp>

// ---------------------------------------------------------------------------
// Merged demo program (v9 base + color-bars full-screen test from v8).
// El ctor de Ssd1963_t ejecuta setup_gpio() y ssd1963_init() automáticamente;
// por eso basta inicializar bcm2835 antes de instanciarla.
// ---------------------------------------------------------------------------

// Patrón de barras de colores completo (merge de testFullScreen de v8).
// Usa draw_block (v9) = drawRectangle (v8). 8 barras verticales.
static void test_color_bars(SSD1963_DRV::Ssd1963_t& lcd) {
    std::cout << "\n=== PRUEBA DE PANTALLA COMPLETA (barras de color) ===" << std::endl;

    // Rellenos completos de cada color base
    const uint16_t fills[] = {BLACK, RED, GREEN, BLUE, WHITE};
    const char* names[] = {"Negro", "Rojo", "Verde", "Azul", "Blanco"};
    for (int i = 0; i < 5; i++) {
        std::cout << names[i] << "..." << std::endl;
        lcd.clear_screen(fills[i]);
        lcd.delay_ms(1500);
    }

    // 8 barras verticales de color
    lcd.clear_screen(BLACK);
    lcd.delay_ms(500);

    const int bar_width = LCD_WIDTH / 8;
    const uint16_t bars[] = {RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE, BLACK};

    for (int i = 0; i < 8; i++) {
        uint16_t x = static_cast<uint16_t>(i * LCD_WIDTH / 8);
        uint16_t w = (i == 7) ? static_cast<uint16_t>(LCD_WIDTH - x)
                              : static_cast<uint16_t>(bar_width);
        lcd.draw_block(x, 0, w, LCD_HEIGHT, bars[i]);
    }
    lcd.delay_ms(5000);
}

int main() {
    if (!bcm2835_init()) {
        std::cerr << "Error al inicializar bcm2835" << std::endl;
        return 1;
    }

    SSD1963_DRV::Ssd1963_t lcd;

    // Limpiar toda la pantalla a negro
    lcd.clear_screen(BLACK);
    lcd.delay_ms(500);

    // Prueba de colores básicos
    std::cout << "Probando Rojo..." << std::endl;
    lcd.clear_screen(RED);
    lcd.delay_ms(2000);

    std::cout << "Probando Verde..." << std::endl;
    lcd.clear_screen(GREEN);
    lcd.delay_ms(2000);

    std::cout << "Probando Azul..." << std::endl;
    lcd.clear_screen(BLUE);
    lcd.delay_ms(2000);

    std::cout << "Probando Blanco..." << std::endl;
    lcd.clear_screen(WHITE);
    lcd.delay_ms(2000);

    std::cout << "Probando Negro..." << std::endl;
    lcd.clear_screen(BLACK);
    lcd.delay_ms(2000);

    // Barras de color (merge de v8)
    test_color_bars(lcd);

    // Dibujar rectángulo redondeado (v9)
    lcd.draw_rounded_rect(20, 20, 96, 96, 12, GREEN, WHITE, 2);
    lcd.delay_ms(5000);

    bcm2835_close();
    return 0;
}
