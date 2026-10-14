#include <bcm2835.h>
#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <tft_ssd1963.hpp>
#include <tft_cmd_ssd1963.hpp>
#include <ssd1963.hpp>

namespace SSD1963_DRV{
        
    void Ssd1963_t::delay_ms(uint32_t ms) {
        usleep(ms * 1000);
    }

    void Ssd1963_t::write_data_bus(uint16_t data) {
        // Escribir los 16 bits directamente sin limpiar todos los pines
        bcm2835_gpio_write(SSD1963_LCD_D0, (data >> 0) & 1);
        bcm2835_gpio_write(SSD1963_LCD_D1, (data >> 1) & 1);
        bcm2835_gpio_write(SSD1963_LCD_D2, (data >> 2) & 1);
        bcm2835_gpio_write(SSD1963_LCD_D3, (data >> 3) & 1);
        bcm2835_gpio_write(SSD1963_LCD_D4, (data >> 4) & 1);
        bcm2835_gpio_write(SSD1963_LCD_D5, (data >> 5) & 1);
        bcm2835_gpio_write(SSD1963_LCD_D6, (data >> 6) & 1);
        bcm2835_gpio_write(SSD1963_LCD_D7, (data >> 7) & 1);
        bcm2835_gpio_write(SSD1963_LCD_D8, (data >> 8) & 1);
        bcm2835_gpio_write(SSD1963_LCD_D9, (data >> 9) & 1);
        bcm2835_gpio_write(SSD1963_LCD_D10, (data >> 10) & 1);
        bcm2835_gpio_write(SSD1963_LCD_D11, (data >> 11) & 1);
        bcm2835_gpio_write(SSD1963_LCD_D12, (data >> 12) & 1);
        bcm2835_gpio_write(SSD1963_LCD_D13, (data >> 13) & 1);
        bcm2835_gpio_write(SSD1963_LCD_D14, (data >> 14) & 1);
        bcm2835_gpio_write(SSD1963_LCD_D15, (data >> 15) & 1);
    }

    void Ssd1963_t::write_command(uint8_t cmd) {
        RS_LOW();
        CS_LOW();
        write_data_bus(cmd);
        WR_LOW();
        delay_us(1);
        WR_HIGH();
        CS_HIGH();
    }

 /*   void Ssd1963_t::write_data(uint16_t data) {
        RS_HIGH();
        CS_LOW();
        write_data_bus(data);
        WR_LOW();
        delay_us(1);
        WR_HIGH();
        CS_HIGH();
    }
*/
    void Ssd1963_t::set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
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

    void Ssd1963_t::clear_screen(uint16_t color) {
        set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
        write_command(SSD1963_WRITE_MEMORY_START);
        
        uint32_t total_pixels = LCD_WIDTH * LCD_HEIGHT;
        for (uint32_t i = 0; i < total_pixels; i++) {
            write_data(color);  // En 16 bits, enviamos el color completo de una vez
        }
    }

    void Ssd1963_t::ssd1963_init() {
        std::cout << "Inicializando SSD1963 en modo 16 bits..." << std::endl;
        
        // 1. Reset físico
        RESET_LOW();
        delay_ms(20);
        RESET_HIGH();
        delay_ms(20);
        
        // 2. Soft Reset
        write_command(SSD1963_SOFT_RESET);
        delay_ms(10);
        
        // 3. Configurar PLL
        write_command(SSD1963_SET_PLL_MN);
        write_data(49);     // PLLclk = REFclk * 50
        write_data(4);      // SYSclk = PLLclk / 5
        write_data(4);      // Dummy
        
        write_command(SSD1963_SET_PLL);
        write_data(0x01);
        delay_ms(10);
        
        write_command(SSD1963_SET_PLL);
        write_data(0x03);
        delay_ms(10);
        
        // 4. Configurar modo LCD - CLAVE: usar 0x20
        write_command(SSD1963_SET_LCD_MODE);
        write_data(0x20);    // TFT mode, 24-bit data
        write_data(0x00);    // Hsync+Vsync mode
        write_data((LCD_WIDTH - 1) >> 8);
        write_data((LCD_WIDTH - 1) & 0xFF);
        write_data((LCD_HEIGHT - 1) >> 8);
        write_data((LCD_HEIGHT - 1) & 0xFF);
        write_data(0x00);    // RGB sequence
        
        // 5. Interfaz de datos - RGB565 16 bits
        write_command(SSD1963_SET_PIXEL_DATA_INTERFACE);
        write_data(SSD1963_PDI_16BIT565);
        
        // 6. Frecuencia LSHIFT
        write_command(SSD1963_SET_LSHIFT_FREQ);
        write_data((LCD_FPR >> 16) & 0xFF);
        write_data((LCD_FPR >> 8) & 0xFF);
        write_data(LCD_FPR & 0xFF);
        
        // 7. Timing horizontal
        write_command(SSD1963_SET_HOR_PERIOD);
        write_data((TFT_HSYNC_PERIOD >> 8) & 0xFF);
        write_data(TFT_HSYNC_PERIOD & 0xFF);
        write_data((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH) >> 8);
        write_data((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH) & 0xFF);
        write_data(TFT_HSYNC_PULSE);
        write_data(0x00);
        write_data(0x00);
        write_data(0x00);
        
        // 8. Timing vertical
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
        
        // 10. Encender backlight
        BACKLIGHT_ON();
        
        std::cout << "SSD1963 inicializado correctamente!" << std::endl;
    }

    void Ssd1963_t::setup_gpio() {
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

    void Ssd1963_t::draw_block(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
        set_area(x, y, x + width - 1, y + height - 1);
        write_command(SSD1963_WRITE_MEMORY_START);
        
        uint32_t pixels = width * height;
        for (uint32_t i = 0; i < pixels; i++) {
            write_data(color);
        }
    }

    void Ssd1963_t::draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
        set_area(x, y, x, y);
        write_command(SSD1963_WRITE_MEMORY_START);
        write_data(color);
    }

    void Ssd1963_t::draw_image_rgb565(const char* filepath) {
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

    void Ssd1963_t::draw_sprite_frame(const char* filepath, uint16_t frame_width, uint16_t frame_height, uint16_t frame_index, uint16_t x, uint16_t y) {
        FILE* file = fopen(filepath, "rb");
        if (!file) {
            std::cerr << "No se pudo abrir: " << filepath << std::endl;
            return;
        }
        
        uint32_t bytes_per_frame = frame_width * frame_height * 2;
        uint32_t offset = bytes_per_frame * frame_index;
        fseek(file, offset, SEEK_SET);
        
        set_area(x, y, x + frame_width - 1, y + frame_height - 1);
        write_command(SSD1963_WRITE_MEMORY_START);
        
        for (uint32_t i = 0; i < frame_width * frame_height; i++) {
            uint8_t high, low;
            if (fread(&high, 1, 1, file) != 1) break;
            if (fread(&low, 1, 1, file) != 1) break;
            uint16_t color = (high << 8) | low;
            write_data(color);
        }
        
        fclose(file);
    }
     
    void Ssd1963_t::draw_rounded_rect(int x, int y, int w, int h, int radius, uint16_t fill_color, uint16_t border_color, int border_thickness) {
        // Relleno del centro
        for (int i = y + radius; i < y + h - radius; i++) {
            set_area(x + radius, i, x + w - radius - 1, i);
            write_command(SSD1963_WRITE_MEMORY_START);
            for (int j = x + radius; j < x + w - radius; j++) {
                write_data(fill_color);
            }
        }

        // Rellenar lados rectos
        for (int i = 0; i < radius; i++) {
            set_area(x + radius, y + i, x + w - radius - 1, y + i);
            write_command(SSD1963_WRITE_MEMORY_START);
            for (int j = x + radius; j < x + w - radius; j++) {
                write_data(fill_color);
            }

            set_area(x + radius, y + h - 1 - i, x + w - radius - 1, y + h - 1 - i);
            write_command(SSD1963_WRITE_MEMORY_START);
            for (int j = x + radius; j < x + w - radius; j++) {
                write_data(fill_color);
            }
        }

        // Laterales
        for (int i = y + radius; i < y + h - radius; i++) {
            for (int t = 0; t < radius; t++) {
                draw_pixel(x + t, i, fill_color);
                draw_pixel(x + w - 1 - t, i, fill_color);
            }
        }

        // Esquinas redondeadas
        for (int i = 0; i < radius; i++) {
            for (int j = 0; j < radius; j++) {
                if ((i * i + j * j) <= (radius * radius)) {
                    draw_pixel(x + i, y + j, fill_color);
                    draw_pixel(x + w - 1 - i, y + j, fill_color);
                    draw_pixel(x + i, y + h - 1 - j, fill_color);
                    draw_pixel(x + w - 1 - i, y + h - 1 - j, fill_color);
                }
            }
        }

        // Bordes
        if (border_thickness > 0) {
            for (int t = 0; t < border_thickness; t++) {
                draw_block(x + t, y + t, w - 2 * t, 1, border_color);
                draw_block(x + t, y + h - 1 - t, w - 2 * t, 1, border_color);
                draw_block(x + t, y + t, 1, h - 2 * t, border_color);
                draw_block(x + w - 1 - t, y + t, 1, h - 2 * t, border_color);
            }
        }
    }
}


// En ssd1963.cpp, añade esta función antes de write_data

namespace SSD1963_DRV{
    
    // Función temporal para compensar falta de rojo
    uint16_t fix_missing_red(uint16_t color) {
        // Extraer componentes RGB565
        uint16_t r = (color >> 11) & 0x1F;
        uint16_t g = (color >> 5) & 0x3F;
        uint16_t b = color & 0x1F;
        
        // Si el color es rojo puro (r>0, g=0, b=0)
        if(r > 0 && g == 0 && b == 0) {
            // Usar el canal verde para simular rojo (solución temporal)
            // Esto hará que el rojo se vea como un tono amarillo/verde
            return (g << 5);  // Solo verde
        }
        
        // Si es blanco (todos los componentes altos)
        if(r > 20 && g > 50 && b > 20) {
            // Compensar falta de rojo aumentando verde y azul
            uint16_t new_r = 0;  // El rojo no funciona
            uint16_t new_g = g;
            uint16_t new_b = b;
            return (new_g << 5) | new_b;
        }
        
        // Para otros colores, mantener original
        return color;
    }
    
    // Modificar write_data para aplicar corrección
    void Ssd1963_t::write_data(uint16_t data) {
        // Aplicar corrección temporal
        data = fix_missing_red(data);
        
        RS_HIGH();
        CS_LOW();
        write_data_bus(data);
        WR_LOW();
        delay_us(1);
        WR_HIGH();
        CS_HIGH();
    }
    
    // El resto del código permanece igual...
}
