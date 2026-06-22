/**
 * @file    main.cpp
 * @brief   ARCADE PHOTO — menú arcade con juego Space Shooter.
 *
 * @details Menú con temática arcade. Escenas:
 *   - SPACE SHOOTER: juego auto-demo con naves, disparos y enemigos
 *   - VER FOTO: galería de imágenes PNG
 *   - PATRONES VIDEO: test de display
 *
 * Auto-navegación sin botones. Cada escena retorna al menú al terminar.
 */

#include <iostream>
#include <engine/GameEngine.hpp>
#include <engine/MenuScene.hpp>
#include <engine/ImageViewerScene.hpp>
#include <engine/SpaceShooterScene.hpp>
#include <engine/PatternScene.hpp>
#include <engine/ColorBlocksScene.hpp>

int main() {
    GameEngine engine;
    if (!engine.init()) {
        std::cerr << "[MAIN] Error en init()\n";
        return 1;
    }

    // Escenas
    MenuScene          menu(engine.display(), "ARCADE PHOTO");
    SpaceShooterScene  shooter(engine.display());
    ImageViewerScene   viewer(engine.display(), "assets/capibaras.rgb565", "CAPIBARAS");
    PatternScene       patterns(engine.display());
    ColorBlocksScene   colors(engine.display());

    // Menú arcade
    menu.add_item("SPACE SHOOTER",  &shooter,  RGB565CONVERT(0,   200, 255));
    menu.add_item("VER FOTO",       &viewer,   RGB565CONVERT(255, 200, 0));
    menu.add_item("DEMO COLORES",   &colors,   RGB565CONVERT(255, 80,  80));
    menu.add_item("PATRONES VIDEO", &patterns, RGB565CONVERT(80,  200, 255));
    menu.add_item("SALIR",          nullptr,   RGB565CONVERT(100, 100, 100));

    // Arranque
    engine.set_menu_scene(&menu);
    engine.set_scene(&menu);

    std::cout << "[MAIN] ARCADE PHOTO v" << ENGINE_VERSION << "\n";
    engine.run();
    std::cout << "[MAIN] Bye.\n";
    return 0;
}
