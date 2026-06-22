/**
 * @file    main.cpp
 * @brief   ARCADE PHOTO — galería de imágenes con menú arcade auto-demo.
 *
 * @details Sistema de menú con temática de máquina arcade que carga
 *          y muestra imágenes PNG (convertidas a .rgb565) en un
 *          display TFT SSD1963 de 480x272. Sin necesidad de botones:
 *          la navegación y selección son completamente automáticas.
 *
 * ## Escenas:
 *   - MenuScene: menú arcade con starfield y cartuchos
 *   - ImageViewerScene: muestra imágenes con marco
 *   - ColorBlocksScene: demo de colores + foto
 *   - PatternScene: patrones de prueba de video
 *
 * ## Flujo:
 *   Menu → auto-select → ImageViewer → auto-return → Menu → ...
 *
 * ## Dependencias:
 *   - libbcm2835, g++ C++11, Make
 */

#include <iostream>
#include <engine/GameEngine.hpp>
#include <engine/MenuScene.hpp>
#include <engine/ColorBlocksScene.hpp>
#include <engine/ImageViewerScene.hpp>
#include <engine/PatternScene.hpp>

int main() {
    GameEngine engine;
    if (!engine.init()) {
        std::cerr << "[MAIN] Error en init()\n";
        return 1;
    }

    // Escenas
    MenuScene         menu(engine.display(), "ARCADE PHOTO");
    ColorBlocksScene  colors(engine.display());
    ImageViewerScene  viewer(engine.display(), "assets/capibaras.rgb565", "CAPIBARAS");
    PatternScene      patterns(engine.display());

    // Menú arcade
    menu.add_item("VER FOTO",         &viewer,   RGB565CONVERT(255, 200, 0));
    menu.add_item("DEMO COLORES",     &colors,   RGB565CONVERT(255, 80,  80));
    menu.add_item("PATRONES VIDEO",   &patterns, RGB565CONVERT(80,  200, 255));
    menu.add_item("SALIR",            nullptr,   RGB565CONVERT(100, 100, 100));

    // Arranque
    engine.set_menu_scene(&menu);
    engine.set_scene(&menu);

    std::cout << "[MAIN] ARCADE PHOTO v" << ENGINE_VERSION << " running\n";
    engine.run();
    std::cout << "[MAIN] Bye.\n";
    return 0;
}
