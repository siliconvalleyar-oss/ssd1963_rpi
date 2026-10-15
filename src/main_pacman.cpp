/**
 * Pac-Man arcade para SSD1963 (480x272) - punto de entrada
 *
 * Inicializa bcm2835 + display, luego corre el juego a 60 Hz con paso fijo.
 * Botones (activos en bajo con pull-up): GPIO 8/9/10/11 = arriba/abajo/izq/der,
 * GPIO 7 = inicio / pausa.
 */
#include <bcm2835.h>
#include <iostream>
#include <time.h>
#include <stdlib.h>

#include "ssd1963.hpp"
#include "pacman.hpp"

static const long TICK_NS = 16666667L; // 60 Hz

static long now_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

int main() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Pac-Man Arcade - SSD1963" << std::endl;
    std::cout << "Display: 480x272, 16-bit RGB565" << std::endl;
    std::cout << "Botones: GPIO 8/9/10/11 (mov.) GPIO 7 (inicio/pausa)" << std::endl;
    std::cout << "========================================\n" << std::endl;

    if (!bcm2835_init()) {
        std::cerr << "Failed to initialize bcm2835" << std::endl;
        return 1;
    }

    SSD1963 display;
    display.setup_gpio();
    display.init();

    srand((unsigned)time(NULL));

    PacmanGame game(display);
    game.init();

    std::cout << "Game initialized. Starting main loop (Ctrl+C para salir)..." << std::endl;

    long next = now_ns();
    while (game.is_running()) {
        game.frame();

        next += TICK_NS;
        long now = now_ns();
        long sleep_ns = next - now;
        if (sleep_ns > 0) {
            struct timespec ts;
            ts.tv_sec = sleep_ns / 1000000000L;
            ts.tv_nsec = sleep_ns % 1000000000L;
            nanosleep(&ts, NULL);
        } else if (sleep_ns > -5 * TICK_NS) {
            next = now; // atrasos pequeños: re-sincronizar sin acumular
        } // si atrasó mucho, dejar que el bucle se ponga al día
    }

    display.clear_screen(0x0000);
    bcm2835_close();
    return 0;
}
