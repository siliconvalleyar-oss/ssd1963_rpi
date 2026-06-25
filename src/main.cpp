#include <cstdio>
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
    struct { uint8_t r,g,b; const char* name; } cols[] = {
        {255,0,0,"ROJO"}, {0,255,0,"VERDE"}, {0,0,255,"AZUL"},
        {255,255,0,"AMARILLO"}, {255,0,255,"MAGENTA"}, {0,255,255,"CIAN"},
        {255,255,255,"BLANCO"}
    };
    // Primero los que fallaban (magenta, cian, blanco)
    for (int32_t i = 6; i >= 4; i--) {
        uint16_t color = rgb565(cols[i].r, cols[i].g, cols[i].b);
        printf("[TEST] %s -> rgb(%3hhu,%3hhu,%3hhu) = 0x%04X\n",
               cols[i].name, cols[i].r, cols[i].g, cols[i].b, color);
        d.clear_screen(color);
        usleep(2000000);
    }
    // Luego los que funcionaban (rojo, verde, azul, amarillo)
    for (int32_t i = 3; i >= 0; i--) {
        uint16_t color = rgb565(cols[i].r, cols[i].g, cols[i].b);
        printf("[TEST] %s -> rgb(%3hhu,%3hhu,%3hhu) = 0x%04X\n",
               cols[i].name, cols[i].r, cols[i].g, cols[i].b, color);
        d.clear_screen(color);
        usleep(2000000);
    }
    uint16_t bg = rgb565(8,8,28);
    printf("[TEST] FONDO -> rgb(8,8,28) = 0x%04X\n", bg);
    d.clear_screen(bg);
    // === FIN TEST ===

    engine.set_menu_scene(&menu);
    engine.set_scene(&menu);

    std::cout << "[MAIN] ARCADE " << ENGINE_VERSION << " iniciado\n";
    engine.run();
    std::cout << "[MAIN] Fin.\n";
    return 0;
}
