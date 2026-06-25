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

    MenuScene         menu(engine.display(), "ARCADE " ENGINE_VERSION);
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

    // === TEST: renderizado directo (sin FrameBuffer) ===
    SSD1963& d = engine.display();
    static constexpr uint16_t RGB(uint8_t r, uint8_t g, uint8_t b) {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
    uint16_t colors[] = { RGB(255,0,0), RGB(0,255,0), RGB(0,0,255),
                          RGB(255,255,0), RGB(255,0,255), RGB(0,255,255),
                          RGB(255,255,255) };
    for (uint32_t flash = 0; flash < 7; flash++) {
        d.clear_screen(colors[flash % 7]);
        d.draw_rect(10, 10, 460, 252, RGB(255,255,0));
        d.draw_string(20, 20, "TEST DIRECTO", RGB(255,255,255), colors[flash % 7]);
        d.draw_string(20, 30, "SIN FRAMEBUFFER", RGB(200,200,200), colors[flash % 7]);
        engine.engine_delay(300);
    }
    d.clear_screen(RGB(8,8,28));

    // === FIN TEST ===

    engine.set_menu_scene(&menu);
    engine.set_scene(&menu);

    std::cout << "[MAIN] ARCADE " << ENGINE_VERSION << " iniciado\n";
    engine.run();
    std::cout << "[MAIN] Fin.\n";
    return 0;
}
