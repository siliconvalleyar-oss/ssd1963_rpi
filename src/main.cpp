#include <ssd1963.hpp>
#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <color.hpp>


int main() {

    SSD1963 ssd1963;
    ssd1963.setup_gpio();
    ssd1963.init();
    
    // Test 1: Color blocks - keep visible
    std::cout << "=== Test 1: Color blocks ===" << std::endl;
    struct ColorBlock {
        uint16_t x;
        uint16_t y;
        uint16_t color;
        const char* name;
    };
    ColorBlock blocks[] = {
        {10, 10, RED, "RED"},
        {80, 10, GREEN, "GREEN"},
        {150, 10, BLUE, "BLUE"},
        {220, 10, YELLOW, "YELLOW"},
        {290, 10, CYAN, "CYAN"},
        {360, 10, MAGENTA, "MAGENTA"},
        {430, 10, WHITE, "WHITE"},
    };
    const uint16_t block_width = 60;
    const uint16_t block_height = 60;
    for (const auto& block : blocks) {
        std::cout << "Dibujando: " << block.name << std::endl;
        ssd1963.draw_block(block.x, block.y, block_width, block_height, block.color);
    }
    std::cout << "Color blocks done. Waiting 5s..." << std::endl;
    usleep(5000000);  // 5 seconds to verify colors
    
    // Test 2: Clear screen
    std::cout << "=== Test 2: Clear screen BLACK ===" << std::endl;
    ssd1963.clear_screen(BLACK);
    usleep(1000000);
    
    // Test 3: Clear screen WHITE
    std::cout << "=== Test 3: Clear screen WHITE ===" << std::endl;
    ssd1963.clear_screen(WHITE);
    usleep(1000000);
    
    // Test 4: Image in smaller area (not full screen)
    std::cout << "=== Test 4: Image 240x136 at center ===" << std::endl;
    // Draw image at center, half size
    ssd1963.draw_image_rgb565("assets/capibaras.rgb565");
    
    std::cout << "Done. Waiting..." << std::endl;
    usleep(10000000);
    return 0;
}