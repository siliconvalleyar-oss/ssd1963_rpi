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
#include <engine/TouchTestScene.hpp>
#include <engine/PiStatusScene.hpp>
#include <engine/sprite_list.hpp>
#include <tftgl_ads7843.hpp>
#include <bcm2835.h>
#include <unistd.h>

static uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static void test_touch(SSD1963& lcd, ADS7843_Touch& touch) {
    std::cout << "\n=== TOUCH ADS7843 (INT=GPIO18) ===" << std::endl;
    touch.set_calibration(TFTGL_CALIB_MIN_X, 200.0,   0);
    touch.set_calibration(TFTGL_CALIB_MAX_X, 3800.0,  LCD_WIDTH);
    touch.set_calibration(TFTGL_CALIB_MIN_Y, 200.0,   0);
    touch.set_calibration(TFTGL_CALIB_MAX_Y, 3800.0,  LCD_HEIGHT);

    bcm2835_gpio_fsel(TOUCH_INT_PIN, BCM2835_GPIO_FSEL_INPT);

    for (int i = 0; i < 20; i++) {
        if (!bcm2835_gpio_lev(TOUCH_INT_PIN)) {
            unsigned int x = 0, y = 0, z = 0;
            touch.get_touch_raw(&x, &y, &z);
            if (touch.get_touch(&x, &y) == TouchStatus::GOT_TOUCH) {
                std::cout << "touch: x=" << x << " y=" << y << std::endl;
                lcd.draw_pixel(x, y, RED);
            } else {
                std::cout << "sin touch (z=" << z << ")" << std::endl;
            }
        } else {
            std::cout << "sin touch (INT=1)" << std::endl;
        }
        usleep(200000);
    }
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
    TouchTestScene    touch_test(engine.display());
    PiStatusScene     pi_status(engine.display());

    populate_sprites(sprites);

    menu.add_item("FONT DEMO",      &font_demo);
    menu.add_item("SPACE SHOOTER",  &shooter);
    menu.add_item("VER FOTO",       &viewer);
    menu.add_item("NAVES",          &sprites);
    menu.add_item("COLORES",        &color_blocks);
    menu.add_item("PATRONES",       &patterns);
    menu.add_item("TOUCH TEST",     &touch_test);
    menu.add_item("PI STATUS",      &pi_status);
    menu.add_item("SALIR",          nullptr);

    // === TEST: renderizado directo (sin FrameBuffer) ===
    SSD1963& d = engine.display();
    struct { uint8_t r,g,b; const char* name; } cols[] = {
        {255,0,0,"ROJO"}, {0,255,0,"VERDE"}, {0,0,255,"AZUL"},
        {255,255,0,"AMARILLO"}, {255,0,255,"MAGENTA"}, {0,255,255,"CIAN"},
        {255,255,255,"BLANCO"}
    };
    for (uint32_t i = 0; i < 7; i++) {
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

    // === TOUCH ADS7843 TEST ===
    {
        ADS7843_Touch touch;
        if (touch.begin() == TouchStatus::OK) {
            d.clear_screen(BLACK);
            test_touch(d, touch);
        } else {
            std::cerr << "[MAIN] Touch ADS7843 no disponible" << std::endl;
        }
    }
    // === FIN TOUCH ===

    engine.set_menu_scene(&menu);
    engine.set_scene(&menu);

    std::cout << "[MAIN] ARCADE " << ENGINE_VERSION << " iniciado\n";
    engine.run();
    std::cout << "[MAIN] Fin.\n";
    return 0;
}
