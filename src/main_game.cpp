/**
 * Mario-style platformer main entry point
 */
#include <bcm2835.h>
#include <iostream>
#include <unistd.h>
#include "ssd1963.hpp"
#include "game.hpp"
#include "config_hw.hpp"
#include "color.hpp"

static uint32_t g_tick = 0;

static uint32_t millis() {
    return g_tick;
}

void timer_increment() {
    g_tick++;
}

int main() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Mario-style Platformer - SSD1963" << std::endl;
    std::cout << "Display: 480x272, 16-bit RGB565" << std::endl;
    std::cout << "========================================\n" << std::endl;

    if (!bcm2835_init()) {
        std::cerr << "Failed to initialize bcm2835" << std::endl;
        return 1;
    }

    SSD1963_HAL_RPi hal;
    SSD1963 display(hal);
    display.setup_gpio();
    display.init();

    Game game(display);
    game.init();

    std::cout << "Game initialized. Starting main loop..." << std::endl;

    while (true) {
        // Update game
        // game.update();
        
        // Simple frame rate control
        usleep(16666); // ~60 FPS
        
        static uint32_t frame_count = 0;
        frame_count++;
        
        if (frame_count % 60 == 0) {
            std::cout << "Frame: " << frame_count << std::endl;
        }
    }

    bcm2835_close();
    return 0;
}