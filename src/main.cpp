#include <ssd1963.hpp>
//#include <ssd1963_cmd.hpp>
#include <bcm2835.h>
#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <color.hpp>



void delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}
//version 3


int main() {

    SSD1963 ssd1963;
    if (!bcm2835_init()) {
        std::cerr << "Error al inicializar bcm2835" << std::endl;
        return 1;
    }
    ssd1963.setup_gpio();
    ssd1963.init();
    // Limpiar toda la pantalla a negro primero
    ssd1963.clear_screen(BLACK);
    delay_ms(500);
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
//    std::cout << "Fin del test de colores." << std::endl;
 delay_ms(900);

    ssd1963.draw_image_rgb565("assets/capibaras.rgb565");
    bcm2835_close();
    return 0;
}

