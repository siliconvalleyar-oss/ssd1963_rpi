#include "ssd1963.hpp"
#include "color.hpp"
#include <unistd.h>

int main() {
    SSD1963 display;

    display.setup_gpio();
    display.init();

    const uint16_t colors[] = {
        RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE, BLACK
    };
    const int num_colors = sizeof(colors) / sizeof(colors[0]);

    while (true) {
        for (int i = 0; i < num_colors; i++) {
            display.clear_screen(colors[i]);
            usleep(1000000);
        }
    }

    return 0;
}