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
#include <unistd.h>

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

int main() {
    GameEngine engine;
    if (!engine.init()) return 1;

    MenuScene         menu(engine.display(), "ARCADE " ENGINE_VERSION);
    FontDemoScene     font_demo(engine.display());
    SpaceShooterScene shooter(engine.display());
    ImageViewerScene  viewer(engine.display(), "assets/capibaras.rgb565", "CAPIBARAS");
    ColorBlocksScene  color_blocks(engine.display());
    PatternScene      patterns(engine.display());
    SpriteViewerScene sprites(engine.display());

    populate_sprites(sprites);

    menu.add_item("FONT DEMO",      &font_demo);
    menu.add_item("SPACE SHOOTER",  &shooter);
    menu.add_item("VER FOTO",       &viewer);
    menu.add_item("NAVES",          &sprites);
    menu.add_item("COLORES",        &color_blocks);
    menu.add_item("PATRONES",       &patterns);
    menu.add_item("SALIR",          nullptr);

    // === TEST: renderizado directo (sin FrameBuffer) ===
    SSD1963& d = engine.display();
    uint16_t cols[] = { rgb565(255,0,0), rgb565(0,255,0), rgb565(0,0,255),
                        rgb565(255,255,0), rgb565(255,0,255), rgb565(0,255,255),
                        rgb565(255,255,255) };
    for (uint32_t flash = 0; flash < 7; flash++) {
        d.clear_screen(cols[flash]);
        d.draw_rect(10, 10, 460, 252, rgb565(255,255,0));
        d.draw_string(20, 20, "TEST DIRECTO", rgb565(255,255,255), cols[flash]);
        d.draw_string(20, 30, "SIN FRAMEBUFFER", rgb565(200,200,200), cols[flash]);
        usleep(300000);
    }
    d.clear_screen(rgb565(8,8,28));
    // === FIN TEST ===

    engine.set_menu_scene(&menu);
    engine.set_scene(&menu);

    std::cout << "[MAIN] ARCADE " << ENGINE_VERSION << " iniciado\n";
    engine.run();
    std::cout << "[MAIN] Fin.\n";
    return 0;
}
