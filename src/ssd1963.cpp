// ssd1963.cpp

#include <unistd.h>
#include <iostream>
#include <config_hw.hpp>
#include <ssd1963_cmd.hpp>
#include <ssd1963.hpp>
#include <color.hpp>


SSD1963::SSD1963() {}

void SSD1963::delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

void SSD1963::setup_gpio() {
    // Configurar pines de datos como salida
    for (uint8_t pin = SSD1963_LCD_D0; pin <= SSD1963_LCD_D15; pin++) {
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
    }
    // Configurar pines de control como salida
    bcm2835_gpio_fsel(SSD1963_LCD_RS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_WR, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_CS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_RESET, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_BACKLIGHT, BCM2835_GPIO_FSEL_OUTP);
    // Estados iniciales
    CS_HIGH();
    WR_HIGH();
    RS_HIGH();
    RESET_HIGH();
    BACKLIGHT_OFF();
}



void  SSD1963::write_command(uint8_t cmd) {
    RS_LOW();
    CS_LOW();
    write_data_bus(cmd);
    WR_LOW();
   //  usleep(10); // o eliminarlo
    WR_HIGH();
    CS_HIGH();
}
void SSD1963::write_data(uint16_t data) {
    RS_HIGH();
    CS_LOW();
    write_data_bus(data);
    WR_LOW();
     //usleep(10); // o eliminarlo
    WR_HIGH();
    CS_HIGH();
}
 

void SSD1963::set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    write_command(SSD1963_SET_COLUMN_ADDRESS);
    write_data(x1 >> 8);
    write_data(x1 & 0xFF);
    write_data(x2 >> 8);
    write_data(x2 & 0xFF);
    write_command(SSD1963_SET_PAGE_ADDRESS);
    write_data(y1 >> 8);
    write_data(y1 & 0xFF);
    write_data(y2 >> 8);
    write_data(y2 & 0xFF);
}



void SSD1963::clear_screen(uint16_t color) {
    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    for (uint32_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        write_data(color);
        write_data(color>>8);
    }
}



void SSD1963::init() {

    // 1. Reset físico (mismo estilo que STM32)
    RESET_LOW();
    delay_ms(10); // equivalente a un pequeño ciclo de espera
    RESET_HIGH();
    delay_ms(10);
    // 2. Soft Reset (ANTES del PLL)
    write_command(SSD1963_SOFT_RESET);
    delay_ms(10);
    // 3. Configurar PLL (para REFclk = 10 MHz → PLLclk = 500MHz, SYSclk = 100MHz)
    write_command(SSD1963_SET_PLL_MN);
    write_data(49);     // PLLclk = REFclk * 50
    write_data(4);      // SYSclk = PLLclk / 5
    write_data(4);      // Dummy (según código STM32)
    // Activar PLL
    write_command(SSD1963_SET_PLL);
    write_data(0x01);
    delay_ms(10);  // largo retardo
    write_command(SSD1963_SET_PLL);
    write_data(0x03);
    delay_ms(10);
    // 4. Configurar modo LCD
    write_command(SSD1963_SET_LCD_MODE);
    write_data(0x0C);    // DE mode
    write_data(0x00);    // RGB565
    write_data((LCD_WIDTH - 1) >> 8);
    write_data((LCD_WIDTH - 1) & 0xFF);
    write_data((LCD_HEIGHT - 1) >> 8);
    write_data((LCD_HEIGHT - 1) & 0xFF);
    write_data(0x00);    // RGB
    // 5. Interfaz de datos (RGB565)
    write_command(SSD1963_SET_PIXEL_DATA_INTERFACE);
    write_data(SSD1963_PDI_16BIT565);  // Interfaz 16-bit 565
    // 6. Frecuencia LSHIFT
    uint32_t LCD_FPR = 0x01E848; // ejemplo: 2.0MHz para SYSCLK=100MHz
    write_command(SSD1963_SET_LSHIFT_FREQ);
    write_data((LCD_FPR >> 16) & 0xFF);
    write_data((LCD_FPR >> 8) & 0xFF);
    write_data(LCD_FPR & 0xFF);
    // 7. Tiempos de sincronización horizontales
    write_command(SSD1963_SET_HOR_PERIOD);
    write_data((TFT_HSYNC_PERIOD >> 8) & 0xFF);
    write_data(TFT_HSYNC_PERIOD & 0xFF);
    write_data((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH) >> 8);
    write_data((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH) & 0xFF);
    write_data(TFT_HSYNC_PULSE);
    write_data(0x00); // LPS = 0
    write_data(0x00); // Opcional
    write_data(0x00); // Opcional
    // 8. Tiempos de sincronización verticales
    write_command(SSD1963_SET_VER_PERIOD);
    write_data((TFT_VSYNC_PERIOD >> 8) & 0xFF);
    write_data(TFT_VSYNC_PERIOD & 0xFF);
    write_data((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH) >> 8);
    write_data((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH) & 0xFF);
    write_data(TFT_VSYNC_PULSE);
    write_data(0x00);
    write_data(0x00);
    // 9. Encender display
    write_command(SSD1963_ON_DISPLAY);
    delay_ms(50);
      // 12. Encender luz de fondo (GPIO)
    BACKLIGHT_ON();
}

 

void SSD1963::draw_block(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    set_area(x, y, x + width - 1, y + height - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    for (uint32_t i = 0; i < width * height; i++) {
        write_data(color);
    }
}

void SSD1963::draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    set_area(x, y, x, y);  // Define el área como 1x1 píxel
    write_command(SSD1963_WRITE_MEMORY_START);
    write_data(color);     // Envía el color (pixel)
}


void SSD1963::draw_image_rgb565(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        std::cerr << "No se pudo abrir la imagen: " << filepath << std::endl;
        return;
    }

    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    write_command(SSD1963_WRITE_MEMORY_START);

    for (uint32_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        uint8_t high, low;
        if (fread(&high, 1, 1, file) != 1) break;
        if (fread(&low, 1, 1, file) != 1) break;

        uint16_t color = (high << 8) | low;
        write_data(color);
    }

    fclose(file);
}

void SSD1963::write_data_bus(uint16_t data) {
    // Limpia todos los pines de datos
    for (uint8_t pin = SSD1963_LCD_D0; pin <= SSD1963_LCD_D15; pin++) {
        bcm2835_gpio_write(pin, LOW);
    }
    // Escribe los bits correspondientes a HIGH
    for (uint8_t i = 0; i < 16; i++) {
        if (data & (1 << i)) {
            bcm2835_gpio_write(SSD1963_LCD_D0 + i, HIGH);
        }
    }
}

