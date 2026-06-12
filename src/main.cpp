#include <ssd1963.hpp>
#include <bcm2835.h>
#include <iostream>
#include <menu.hpp>

int main() {
    SSD1963 ssd1963;

    if (!bcm2835_init()) {
        std::cerr << "Error: bcm2835_init() failed. Run with sudo." << std::endl;
        return 1;
    }

    ssd1963.setup_gpio();
    ssd1963.init();
    ssd1963.clear_screen(0x0000);

    Menu menu(ssd1963);
    menu.set_title("SSD1963 DEMO v1.0");

    menu.add_item("1. Color Test",       menu_color_test);
    menu.add_item("2. Show Photo",       menu_show_photo);
    menu.add_item("3. Screen Info",      menu_screen_info);
    menu.add_item("4. Draw Shapes",      menu_draw_shapes);
    menu.add_item("5. QR: GitHub URL",   menu_qr_url);
    menu.add_item("6. QR: WiFi Config",  menu_qr_wifi);
    menu.add_item("7. QR: Custom Text",  menu_qr_text);
    menu.add_item("8. About",            menu_about);
    menu.add_item("9. Exit",             [](SSD1963&){ /* exit handled by menu */ });

    menu.run();

    ssd1963.clear_screen(BLACK);
    bcm2835_close();
    return 0;
}
