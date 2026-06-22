#include <iostream>
#include <engine/GameEngine.hpp>
#include <engine/MenuScene.hpp>
#include <engine/FontDemoScene.hpp>
#include <engine/SpaceShooterScene.hpp>
#include <engine/ImageViewerScene.hpp>
#include <engine/ColorBlocksScene.hpp>
#include <engine/PatternScene.hpp>
#include <engine/SpriteViewerScene.hpp>
#include <engine/sprite_list.hpp>

int main() {
    GameEngine engine;
    if (!engine.init()) return 1;

    MenuScene         menu(engine.display(), "ARCADE PHOTO v5");
    FontDemoScene     font_demo(engine.display());
    SpaceShooterScene shooter(engine.display());
    ImageViewerScene  viewer(engine.display(), "assets/capibaras.rgb565", "CAPIBARAS");
    ColorBlocksScene  colors(engine.display());
    PatternScene      patterns(engine.display());
    SpriteViewerScene sprites(engine.display());

    populate_sprites(sprites);

    menu.add_item("FONT DEMO",      &font_demo);
    menu.add_item("SPACE SHOOTER",  &shooter);
    menu.add_item("VER FOTO",       &viewer);
    menu.add_item("NAVES",          &sprites);
    menu.add_item("COLORES",        &colors);
    menu.add_item("PATRONES",       &patterns);
    menu.add_item("SALIR",          nullptr);

    engine.set_menu_scene(&menu);
    engine.set_scene(&menu);

    std::cout << "[MAIN] ARCADE v" << ENGINE_VERSION << " iniciado\n";
    engine.run();
    std::cout << "[MAIN] Fin.\n";
    return 0;
}
