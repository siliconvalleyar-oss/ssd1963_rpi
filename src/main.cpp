/**
 * @file    main.cpp
 * @brief   Punto de entrada del SSD1963 Game Engine.
 * @details Inicializa el motor de juegos, configura las escenas
 *          y lanza el bucle principal. La navegación entre escenas
 *          se realiza mediante menú interactivo o botones GPIO.
 *
 * ## Escenas disponibles:
 * - **Menú Principal**: Navegación entre todas las opciones
 * - **Color Blocks**: Demo de bloques de colores + imagen
 * - **Image Viewer**: Visualizador de imágenes RGB565
 * - **Paint**: Dibujo automático con patrones
 * - **Pattern Generator**: Patrones de prueba (barras, gradiente, etc.)
 *
 * ## Controles GPIO (opcional):
 * - UP (GPIO 7):   Navegar arriba
 * - DOWN (GPIO 8): Navegar abajo
 * - SELECT (GPIO 9): Seleccionar/Aceptar
 * - BACK (GPIO 10):  Retroceder/Salir
 *
 * Sin botones conectados, el menú avanza con auto-timing.
 *
 * ## Dependencias:
 * - libbcm2835 (http://www.airspayce.com/mikem/bcm2835/)
 * - g++ con soporte C++11
 */

#include <iostream>
#include <engine/GameEngine.hpp>
#include <engine/MenuScene.hpp>
#include <engine/ColorBlocksScene.hpp>
#include <engine/ImageViewerScene.hpp>
#include <engine/PaintScene.hpp>
#include <engine/PatternScene.hpp>

int main() {
    // -----------------------------------------------------------------
    // 1. Instanciar el motor
    // -----------------------------------------------------------------
    GameEngine engine;

    if (!engine.init()) {
        std::cerr << "[MAIN] Error fatal: no se pudo inicializar el engine.\n";
        return 1;
    }

    // -----------------------------------------------------------------
    // 2. Crear escenas (se registrarán en el menú)
    // -----------------------------------------------------------------
    // Las escenas se crean con vida controlada por el stack.
    // Para un sistema más complejo, usar punteros inteligentes o pool.

    MenuScene main_menu(engine.display(), "SSD1963 GAME ENGINE v" ENGINE_VERSION);

    ColorBlocksScene demo_colors(engine.display());
    ImageViewerScene viewer(engine.display(), "assets/capibaras.rgb565");
    PaintScene        painter(engine.display());
    PatternScene      patterns(engine.display());

    // Scene placeholder para futuras expansiones
    // Template: add_item("Nombre", &escena, COLOR);

    // -----------------------------------------------------------------
    // 3. Registrar items del menú principal
    // -----------------------------------------------------------------
    main_menu.add_item("Demo Colores",        &demo_colors, RED);
    main_menu.add_item("Visor de Imagenes",    &viewer,      GREEN);
    main_menu.add_item("Paint (Demo Auto)",    &painter,     BLUE);
    main_menu.add_item("Patrones de Prueba",   &patterns,    YELLOW);
    main_menu.add_item("INFO: SSD1963 v2.0",   nullptr,      CYAN);
    main_menu.add_item("Salir",                nullptr,      WHITE);

    // -----------------------------------------------------------------
    // 4. Registrar el menú en el engine y arrancar
    // -----------------------------------------------------------------
    engine.set_menu_scene(&main_menu);
    engine.set_scene(&main_menu);

    // -----------------------------------------------------------------
    // 5. Bucle principal (no retorna hasta EngineState::EXIT)
    // -----------------------------------------------------------------
    std::cout << "[MAIN] Engine running. Display: " << LCD_WIDTH
              << "x" << LCD_HEIGHT << "\n";

    engine.run();

    std::cout << "[MAIN] Engine stopped. Goodbye.\n";
    return 0;
}
