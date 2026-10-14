// main.cpp
//
// Program busca TFT con SSD1963 + Raspberry Pi (bus paralelo de 16 bits).
//
// API base: clase SSD1963 (include/ssd1963.hpp) — versión avanzada del repo.
//
// Merge de demos experimentales de los snapshots de tmp/:
//   - test de llenado de pantalla completa (colores básicos)  [v7/v8]
//   - patrón de barras de color verticales (8 barras)        [v8]
//   - dibujo de rectángulo redondeado                       [v9]
//   - visualización de imagen rgb565 desde assets/          [HEAD]
//
// NOTA: la clase SSD1963 del repo no expone draw_rounded_rect, así que se
// implementa como helper local en este main.cpp (sin tocar ssd1963.cpp/.hpp).

#include <ssd1963.hpp>
#include <config_hw.hpp>
#include <bcm2835.h>
#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <color.hpp>

static void delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

// ---------------------------------------------------------------------------
// Merge de v9: rectángulo redondeado, implementado con fill_rect + draw_pixel
// (la clase del repo no lo incluye; se mantiene ssd1963.cpp/.hpp intactos).
// ---------------------------------------------------------------------------
static void draw_rounded_rect(SSD1963& lcd,
                              int x, int y, int w, int h, int radius,
                              uint16_t fill_color, uint16_t border_color,
                              int border_thickness) {
    // Relleno central
    for (int i = y + radius; i < y + h - radius; i++) {
        lcd.fill_rect(x + radius, static_cast<uint16_t>(i),
                      static_cast<uint16_t>(w - radius), 1, fill_color);
    }

    // Rellenar lados rectos
    for (int i = 0; i < radius; i++) {
        lcd.fill_rect(x + radius, static_cast<uint16_t>(y + i),
                      static_cast<uint16_t>(w - radius), 1, fill_color);
        lcd.fill_rect(x + radius, static_cast<uint16_t>(y + h - 1 - i),
                      static_cast<uint16_t>(w - radius), 1, fill_color);
    }

    // Laterales
    for (int i = y + radius; i < y + h - radius; i++) {
        for (int t = 0; t < radius; t++) {
            lcd.draw_pixel(static_cast<uint16_t>(x + t), static_cast<uint16_t>(i), fill_color);
            lcd.draw_pixel(static_cast<uint16_t>(x + w - 1 - t), static_cast<uint16_t>(i), fill_color);
        }
    }

    // Esquinas redondeadas
    for (int i = 0; i < radius; i++) {
        for (int j = 0; j < radius; j++) {
            if ((i * i + j * j) <= (radius * radius)) {
                lcd.draw_pixel(static_cast<uint16_t>(x + i),       static_cast<uint16_t>(y + j),               fill_color);
                lcd.draw_pixel(static_cast<uint16_t>(x + w - 1 - i), static_cast<uint16_t>(y + j),               fill_color);
                lcd.draw_pixel(static_cast<uint16_t>(x + i),       static_cast<uint16_t>(y + h - 1 - j),        fill_color);
                lcd.draw_pixel(static_cast<uint16_t>(x + w - 1 - i), static_cast<uint16_t>(y + h - 1 - j),        fill_color);
            }
        }
    }

    // Bordes
    if (border_thickness > 0) {
        for (int t = 0; t < border_thickness; t++) {
            lcd.fill_rect(static_cast<uint16_t>(x + t),       static_cast<uint16_t>(y + t),
                          static_cast<uint16_t>(w - 2 * t), 1, border_color);
            lcd.fill_rect(static_cast<uint16_t>(x + t),       static_cast<uint16_t>(y + h - 1 - t),
                          static_cast<uint16_t>(w - 2 * t), 1, border_color);
            lcd.fill_rect(static_cast<uint16_t>(x + t),       static_cast<uint16_t>(y + t),
                          1, static_cast<uint16_t>(h - 2 * t), border_color);
            lcd.fill_rect(static_cast<uint16_t>(x + w - 1 - t), static_cast<uint16_t>(y + t),
                          1, static_cast<uint16_t>(h - 2 * t), border_color);
        }
    }
}

// ---------------------------------------------------------------------------
// Merge de v8: test de pantalla completa (llenado de cada color base)
// ---------------------------------------------------------------------------
static void test_full_screen(SSD1963& lcd) {
    std::cout << "\n=== PRUEBA DE PANTALLA COMPLETA ===" << std::endl;

    const uint16_t fills[] = {BLACK, RED, GREEN, BLUE, WHITE};
    const char* names[] = {"Negro", "Rojo", "Verde", "Azul", "Blanco"};
    for (int i = 0; i < 5; i++) {
        std::cout << names[i] << "..." << std::endl;
        lcd.clear_screen(fills[i]);
        delay_ms(1500);
    }
}

// ---------------------------------------------------------------------------
// Merge de v8: patrón de barras de color verticales (8 barras)
// ---------------------------------------------------------------------------
static void test_color_bars(SSD1963& lcd) {
    std::cout << "\n=== BARRAS DE COLOR ===" << std::endl;

    lcd.clear_screen(BLACK);
    delay_ms(500);

    const uint16_t bars[] = {RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE, BLACK};
    const uint16_t bar_width = LCD_WIDTH / 8;

    for (int i = 0; i < 8; i++) {
        uint16_t x = static_cast<uint16_t>(i * LCD_WIDTH / 8);
        uint16_t w = (i == 7) ? static_cast<uint16_t>(LCD_WIDTH - x) : bar_width;
        lcd.fill_rect(x, 0, w, LCD_HEIGHT, bars[i]);
    }
    delay_ms(5000);
}

int main() {
    if (!bcm2835_init()) {
        std::cerr << "Error al inicializar bcm2835" << std::endl;
        return 1;
    }

    SSD1963 ssd1963;
    ssd1963.setup_gpio();
    ssd1963.init();

    // Pantalla en BLANCO primero (tras la inicialización del SSD1963)
    ssd1963.clear_screen(WHITE);
    delay_ms(500);

    // Fill de colores básicos (merge v8)
    test_full_screen(ssd1963);

    // Barras de color (merge v8)
    test_color_bars(ssd1963);

    // Rectángulo redondeado (merge v9)
    std::cout << "\n=== RECTANGULO REDONDEADO ===" << std::endl;
    draw_rounded_rect(ssd1963, 20, 20, 96, 96, 12, GREEN, WHITE, 2);
    delay_ms(5000);

    // Bloques de color distribuidos (merge HEAD)
    std::cout << "\n=== BLOQUES DE COLOR ===" << std::endl;
    const uint16_t block_width  = 45;
    const uint16_t block_height = 45;
    ssd1963.clear_screen(BLACK);
    const uint16_t bx = 10, by = 10;
    const uint16_t block_colors[] = {RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE};
    const char* block_names[] = {"RED","GREEN","BLUE","YELLOW","CYAN","MAGENTA","WHITE"};
    uint16_t x = bx;
    for (int i = 0; i < 7; i++) {
        std::cout << "Dibujando bloque: " << block_names[i]
                  << " en (" << x << ", " << by << ")" << std::endl;
        ssd1963.fill_rect(x, by, block_width, block_height, block_colors[i]);
        x += block_width + 10;
    }
    delay_ms(3000);

    // Imagen rgb565 desde assets/ (merge HEAD)
    std::cout << "\n=== IMAGEN capibaras.rgb565 ===" << std::endl;
    ssd1963.draw_image_rgb565("assets/capibaras.rgb565");

    bcm2835_close();
    return 0;
}
