#include <ssd1963.hpp>
#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <color.hpp>


int main() {

    SSD1963 ssd1963;
    ssd1963.setup_gpio();
    ssd1963.init();
    // Pantalla en BLANCO primero (tras la 2a inicializacion del SSD1963)
    ssd1963.clear_screen(WHITE);
    usleep(500000);
    struct ColorBlock {
        uint16_t x;
        uint16_t y;
        uint16_t color;
        const char* name;
    };
    // Bloques grandes 60x60, distribuidos horizontalmente
    ColorBlock blocks[] = {
        {10, 10, RED, "RED"},
        {80, 10, GREEN, "GREEN"},
        {150, 10, BLUE, "BLUE"},
        {220, 10, YELLOW, "YELLOW"},
        {290, 10, CYAN, "CYAN"},
        {360, 10, MAGENTA, "MAGENTA"},
        {430, 10, WHITE, "WHITE"},
        // Aquí podrías agregar más o cambiar posición si tu pantalla es más ancha
    };
    const uint16_t block_width = 45;
    const uint16_t block_height = 45;
    for (const auto& block : blocks) {
        std::cout << "Dibujando bloque color: " << block.name << " en (" << block.x << ", " << block.y << ")" << std::endl;
        ssd1963.draw_block(block.x, block.y, block_width, block_height, block.color);
        // Sin delay para que quede fijo rápido
    }
    usleep(900000);

    ssd1963.draw_image_rgb565("assets/capibaras.rgb565");
    return 0;
}