/**
 * @file    ssd1963.cpp
 * @brief   Implementación del driver SSD1963 para Raspberry Pi.
 * @details Control de display TFT mediante bus paralelo 8080
 *          con GPIO bit-banging via libbcm2835.
 */

#include <unistd.h>
#include <iostream>
#include <cstring>
#include <config_hw.hpp>
#include <ssd1963_cmd.hpp>
#include <ssd1963.hpp>
#include <color.hpp>

// =========================================================================
// Fuente bitmap 8x8 rotada 90° sentido horario
// =========================================================================
// Almacenada como columnas: font[idx][col] = byte, bit row = pixel (col, row)
// Esta fuente está rotada 90° CW respecto a la original.
const uint8_t SSD1963::m_font_8x8[95][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ' '
    {0x00, 0x00, 0x00, 0x5F, 0x5F, 0x00, 0x00, 0x00}, // '!'
    {0x00, 0x07, 0x07, 0x00, 0x07, 0x07, 0x00, 0x00}, // '"'
    {0x14, 0x7F, 0x7F, 0x14, 0x7F, 0x7F, 0x14, 0x00}, // '#'
    {0x00, 0x24, 0x2E, 0x6B, 0x6B, 0x3A, 0x12, 0x00}, // '$'
    {0x4C, 0x6A, 0x36, 0x18, 0x6C, 0x56, 0x32, 0x00}, // '%'
    {0x30, 0x7E, 0x4F, 0x59, 0x37, 0x7A, 0x48, 0x00}, // '&'
    {0x00, 0x00, 0x00, 0x07, 0x07, 0x00, 0x00, 0x00}, // '\''
    {0x00, 0x00, 0x1C, 0x3E, 0x63, 0x41, 0x00, 0x00}, // '('
    {0x00, 0x00, 0x41, 0x63, 0x3E, 0x1C, 0x00, 0x00}, // ')'
    {0x08, 0x2A, 0x3E, 0x1C, 0x1C, 0x3E, 0x2A, 0x08}, // '*'
    {0x00, 0x08, 0x08, 0x3E, 0x3E, 0x08, 0x08, 0x00}, // '+'
    {0x00, 0x00, 0x80, 0xE0, 0x60, 0x00, 0x00, 0x00}, // ','
    {0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00}, // '-'
    {0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00}, // '.'
    {0x00, 0x40, 0x70, 0x38, 0x0E, 0x07, 0x01, 0x00}, // '/'
    {0x00, 0x3E, 0x7F, 0x4D, 0x59, 0x7F, 0x3E, 0x00}, // '0'
    {0x00, 0x40, 0x42, 0x7F, 0x7F, 0x40, 0x40, 0x00}, // '1'
    {0x00, 0x62, 0x73, 0x59, 0x49, 0x4F, 0x46, 0x00}, // '2'
    {0x00, 0x22, 0x63, 0x49, 0x49, 0x7F, 0x36, 0x00}, // '3'
    {0x18, 0x1C, 0x16, 0x13, 0x7F, 0x7F, 0x10, 0x00}, // '4'
    {0x00, 0x27, 0x67, 0x45, 0x45, 0x7D, 0x39, 0x00}, // '5'
    {0x00, 0x3E, 0x7F, 0x49, 0x49, 0x7B, 0x32, 0x00}, // '6'
    {0x00, 0x01, 0x71, 0x79, 0x0D, 0x07, 0x03, 0x00}, // '7'
    {0x00, 0x36, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00}, // '8'
    {0x00, 0x26, 0x6F, 0x49, 0x49, 0x7F, 0x3E, 0x00}, // '9'
    {0x00, 0x00, 0x00, 0x66, 0x66, 0x00, 0x00, 0x00}, // ':'
    {0x00, 0x00, 0x80, 0xE6, 0x66, 0x00, 0x00, 0x00}, // ';'
    {0x00, 0x08, 0x1C, 0x36, 0x63, 0x41, 0x00, 0x00}, // '<'
    {0x00, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00}, // '='
    {0x00, 0x00, 0x41, 0x63, 0x36, 0x1C, 0x08, 0x00}, // '>'
    {0x00, 0x02, 0x03, 0x59, 0x5D, 0x07, 0x02, 0x00}, // '?'
    {0x00, 0x3E, 0x7F, 0x41, 0x4D, 0x6F, 0x2E, 0x00}, // '@'
    {0x00, 0x7C, 0x7E, 0x13, 0x13, 0x7E, 0x7C, 0x00}, // 'A'
    {0x00, 0x7F, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00}, // 'B'
    {0x00, 0x3E, 0x7F, 0x41, 0x41, 0x63, 0x22, 0x00}, // 'C'
    {0x00, 0x7F, 0x7F, 0x41, 0x63, 0x3E, 0x1C, 0x00}, // 'D'
    {0x00, 0x7F, 0x7F, 0x49, 0x49, 0x49, 0x41, 0x00}, // 'E'
    {0x00, 0x7F, 0x7F, 0x09, 0x09, 0x09, 0x01, 0x00}, // 'F'
    {0x00, 0x3E, 0x7F, 0x41, 0x49, 0x7B, 0x7A, 0x00}, // 'G'
    {0x00, 0x7F, 0x7F, 0x08, 0x08, 0x7F, 0x7F, 0x00}, // 'H'
    {0x00, 0x41, 0x41, 0x7F, 0x7F, 0x41, 0x41, 0x00}, // 'I'
    {0x00, 0x20, 0x60, 0x40, 0x40, 0x7F, 0x3F, 0x00}, // 'J'
    {0x00, 0x7F, 0x7F, 0x1C, 0x36, 0x63, 0x41, 0x00}, // 'K'
    {0x00, 0x7F, 0x7F, 0x40, 0x40, 0x40, 0x40, 0x00}, // 'L'
    {0x7F, 0x7F, 0x06, 0x0C, 0x06, 0x7F, 0x7F, 0x00}, // 'M'
    {0x00, 0x7F, 0x7F, 0x0E, 0x1C, 0x7F, 0x7F, 0x00}, // 'N'
    {0x00, 0x3E, 0x7F, 0x41, 0x41, 0x7F, 0x3E, 0x00}, // 'O'
    {0x00, 0x7F, 0x7F, 0x09, 0x09, 0x0F, 0x06, 0x00}, // 'P'
    {0x00, 0x1E, 0x3F, 0x21, 0x71, 0x7F, 0x5E, 0x00}, // 'Q'
    {0x00, 0x7F, 0x7F, 0x09, 0x19, 0x7F, 0x66, 0x00}, // 'R'
    {0x00, 0x26, 0x6F, 0x49, 0x49, 0x7B, 0x32, 0x00}, // 'S'
    {0x00, 0x01, 0x01, 0x7F, 0x7F, 0x01, 0x01, 0x00}, // 'T'
    {0x00, 0x3F, 0x7F, 0x40, 0x40, 0x7F, 0x3F, 0x00}, // 'U'
    {0x00, 0x1F, 0x3F, 0x60, 0x60, 0x3F, 0x1F, 0x00}, // 'V'
    {0x7F, 0x7F, 0x30, 0x18, 0x30, 0x7F, 0x7F, 0x00}, // 'W'
    {0x00, 0x63, 0x77, 0x1C, 0x1C, 0x77, 0x63, 0x00}, // 'X'
    {0x00, 0x07, 0x0F, 0x78, 0x78, 0x0F, 0x07, 0x00}, // 'Y'
    {0x00, 0x61, 0x71, 0x59, 0x4D, 0x47, 0x43, 0x00}, // 'Z'
    {0x00, 0x00, 0x7F, 0x7F, 0x41, 0x41, 0x00, 0x00}, // '['
    {0x00, 0x01, 0x07, 0x0E, 0x38, 0x70, 0x40, 0x00}, // '\\'
    {0x00, 0x00, 0x41, 0x41, 0x7F, 0x7F, 0x00, 0x00}, // ']'
    {0x00, 0x04, 0x06, 0x03, 0x03, 0x06, 0x04, 0x00}, // '^'
    {0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00}, // '_'
    {0x00, 0x00, 0x00, 0x03, 0x07, 0x04, 0x00, 0x00}, // '`'
    {0x00, 0x20, 0x74, 0x54, 0x54, 0x7C, 0x78, 0x00}, // 'a'
    {0x00, 0x7F, 0x7F, 0x44, 0x44, 0x7C, 0x38, 0x00}, // 'b'
    {0x00, 0x38, 0x7C, 0x44, 0x44, 0x6C, 0x28, 0x00}, // 'c'
    {0x00, 0x38, 0x7C, 0x44, 0x44, 0x7F, 0x7F, 0x00}, // 'd'
    {0x00, 0x38, 0x7C, 0x54, 0x54, 0x5C, 0x18, 0x00}, // 'e'
    {0x00, 0x08, 0x7E, 0x7F, 0x09, 0x0B, 0x02, 0x00}, // 'f'
    {0x00, 0x18, 0xBC, 0xA4, 0xA4, 0xFC, 0x7C, 0x00}, // 'g'
    {0x00, 0x7F, 0x7F, 0x04, 0x04, 0x7C, 0x78, 0x00}, // 'h'
    {0x00, 0x00, 0x44, 0x7D, 0x7D, 0x40, 0x00, 0x00}, // 'i'
    {0x00, 0x80, 0x84, 0xFD, 0x7D, 0x00, 0x00, 0x00}, // 'j'
    {0x00, 0x7F, 0x7F, 0x10, 0x38, 0x6C, 0x44, 0x00}, // 'k'
    {0x00, 0x00, 0x41, 0x7F, 0x7F, 0x40, 0x00, 0x00}, // 'l'
    {0x7C, 0x7C, 0x0C, 0x18, 0x0C, 0x7C, 0x78, 0x00}, // 'm'
    {0x00, 0x7C, 0x7C, 0x04, 0x04, 0x7C, 0x78, 0x00}, // 'n'
    {0x00, 0x38, 0x7C, 0x44, 0x44, 0x7C, 0x38, 0x00}, // 'o'
    {0x00, 0xFC, 0xFC, 0x24, 0x24, 0x3C, 0x18, 0x00}, // 'p'
    {0x00, 0x18, 0x3C, 0x24, 0x24, 0xFC, 0xFC, 0x00}, // 'q'
    {0x00, 0x7C, 0x7C, 0x04, 0x04, 0x0C, 0x08, 0x00}, // 'r'
    {0x00, 0x48, 0x5C, 0x54, 0x54, 0x74, 0x24, 0x00}, // 's'
    {0x00, 0x04, 0x3F, 0x7F, 0x44, 0x44, 0x00, 0x00}, // 't'
    {0x00, 0x3C, 0x7C, 0x40, 0x40, 0x7C, 0x7C, 0x00}, // 'u'
    {0x00, 0x1C, 0x3C, 0x60, 0x60, 0x3C, 0x1C, 0x00}, // 'v'
    {0x1C, 0x7C, 0x60, 0x30, 0x60, 0x7C, 0x3C, 0x00}, // 'w'
    {0x00, 0x44, 0x6C, 0x38, 0x38, 0x6C, 0x44, 0x00}, // 'x'
    {0x00, 0x1C, 0xBC, 0xA0, 0xA0, 0xFC, 0x7C, 0x00}, // 'y'
    {0x00, 0x44, 0x64, 0x74, 0x5C, 0x4C, 0x44, 0x00}, // 'z'
    {0x00, 0x08, 0x08, 0x3E, 0x77, 0x41, 0x41, 0x00}, // '{'
    {0x00, 0x00, 0x00, 0x77, 0x77, 0x00, 0x00, 0x00}, // '|'
    {0x00, 0x41, 0x41, 0x77, 0x3E, 0x08, 0x08, 0x00}, // '}'
    {0x02, 0x03, 0x01, 0x03, 0x02, 0x03, 0x01, 0x00}, // '~'
};

// =========================================================================
// Constructor
// =========================================================================
SSD1963::SSD1963() {}

// =========================================================================
// Inicialización
// =========================================================================
void SSD1963::delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

void SSD1963::setup_gpio() {
    // Configurar pines de datos D0-D15 como salida
    for (uint8_t pin = SSD1963_LCD_D0; pin <= SSD1963_LCD_D15; pin++) {
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
    }
    // Configurar pines de control como salida
    bcm2835_gpio_fsel(SSD1963_LCD_RS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_WR, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_CS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_RESET, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_BACKLIGHT, BCM2835_GPIO_FSEL_OUTP);
    // Estados iniciales (inactivos)
    CS_HIGH();
    WR_HIGH();
    RS_HIGH();
    RESET_HIGH();
    BACKLIGHT_OFF();
}

void SSD1963::init() {
    // 1. Reset físico por GPIO
    RESET_LOW();
    delay_ms(10);
    RESET_HIGH();
    delay_ms(10);

    // 2. Soft reset por comando
    write_command(SSD1963_SOFT_RESET);
    delay_ms(10);

    // 3. Configurar PLL
    //    REFclk = 10 MHz → PLLclk = 500 MHz (M=50x), SYSclk = 100 MHz (N=/5)
    write_command(SSD1963_SET_PLL_MN);
    write_data(49);  // Multiplicador M (PLLclk = REFclk * (M+1))
    write_data(4);   // Divisor N (SYSclk = PLLclk / (N+1))
    write_data(4);   // Dummy (reservado)

    // Activar PLL en dos pasos
    write_command(SSD1963_SET_PLL);
    write_data(0x01);
    delay_ms(10);
    write_command(SSD1963_SET_PLL);
    write_data(0x03);
    delay_ms(10);

    // 4. Configurar modo LCD (DE mode, RGB565, 480x272)
    write_command(SSD1963_SET_LCD_MODE);
    write_data(0x0C);    // TFT + DE mode
    write_data(0x00);    // RGB565
    write_data((LCD_WIDTH - 1) >> 8);
    write_data((LCD_WIDTH - 1) & 0xFF);
    write_data((LCD_HEIGHT - 1) >> 8);
    write_data((LCD_HEIGHT - 1) & 0xFF);
    write_data(0x00);    // RGB orden

    // 5. Interfaz de datos de píxel (16-bit RGB565)
    write_command(SSD1963_SET_PIXEL_DATA_INTERFACE);
    write_data(SSD1963_PDI_16BIT565);

    // 6. Frecuencia LSHIFT (pixel clock)
    write_command(SSD1963_SET_LSHIFT_FREQ);
    write_data((LCD_FPR >> 16) & 0xFF);
    write_data((LCD_FPR >> 8) & 0xFF);
    write_data(LCD_FPR & 0xFF);

    // 7. Temporización horizontal
    write_command(SSD1963_SET_HOR_PERIOD);
    write_data((TFT_HSYNC_PERIOD >> 8) & 0xFF);
    write_data(TFT_HSYNC_PERIOD & 0xFF);
    write_data((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH) >> 8);
    write_data((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH) & 0xFF);
    write_data(TFT_HSYNC_PULSE);
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);

    // 8. Temporización vertical
    write_command(SSD1963_SET_VER_PERIOD);
    write_data((TFT_VSYNC_PERIOD >> 8) & 0xFF);
    write_data(TFT_VSYNC_PERIOD & 0xFF);
    write_data((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH) >> 8);
    write_data((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH) & 0xFF);
    write_data(TFT_VSYNC_PULSE);
    write_data(0x00);
    write_data(0x00);

    // 9. Encender display y backlight
    write_command(SSD1963_ON_DISPLAY);
    delay_ms(50);
    BACKLIGHT_ON();
}

// =========================================================================
// Primitivas del bus 8080
// =========================================================================
void SSD1963::write_command(uint8_t cmd) {
    RS_LOW();
    CS_LOW();
    write_data_bus(cmd);
    WR_LOW();
    WR_HIGH();
    CS_HIGH();
}

void SSD1963::write_data(uint16_t data) {
    RS_HIGH();
    CS_LOW();
    write_data_bus(data);
    WR_LOW();
    WR_HIGH();
    CS_HIGH();
}

void SSD1963::write_data_bus(uint16_t data) {
    // Limpiar todos los pines de datos
    for (uint8_t pin = SSD1963_LCD_D0; pin <= SSD1963_LCD_D15; pin++) {
        bcm2835_gpio_write(pin, LOW);
    }
    // Escribir bits activos
    for (uint8_t i = 0; i < 16; i++) {
        if (data & (1 << i)) {
            bcm2835_gpio_write(SSD1963_LCD_D0 + i, HIGH);
        }
    }
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

// =========================================================================
// Operaciones de dibujo
// =========================================================================
void SSD1963::clear_screen(uint16_t color) {
    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    for (uint32_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        write_data(color);
    }
}

void SSD1963::draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    set_area(x, y, x, y);
    write_command(SSD1963_WRITE_MEMORY_START);
    write_data(color);
}

void SSD1963::draw_block(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    set_area(x, y, x + width - 1, y + height - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    for (uint32_t i = 0; i < (uint32_t)width * height; i++) {
        write_data(color);
    }
}

void SSD1963::fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    draw_block(x, y, width, height, color);
}

void SSD1963::draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    // Superior
    draw_block(x, y, width, 1, color);
    // Inferior
    draw_block(x, y + height - 1, width, 1, color);
    // Izquierda
    draw_block(x, y, 1, height, color);
    // Derecha
    draw_block(x + width - 1, y, 1, height, color);
}

// =========================================================================
// Renderizado de texto
// =========================================================================
void SSD1963::draw_char(uint16_t x, uint16_t y, char chr, uint16_t color, uint16_t bg) {
    if (chr < 0x20 || chr > 0x7E) {
        chr = 0x20; // Reemplazar no imprimibles con espacio
    }
    uint8_t idx = chr - 0x20;

    for (uint8_t col = 0; col < FONT_CHAR_WIDTH; col++) {
        uint8_t line = m_font_8x8[idx][col];
        for (uint8_t row = 0; row < FONT_CHAR_HEIGHT; row++) {
            if (line & (1 << row)) {
                draw_pixel(x + col, y + row, color);
            } else if (bg != 0xFFFF) {
                draw_pixel(x + col, y + row, bg);
            }
        }
    }
}

void SSD1963::draw_string(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg) {
    uint16_t cursor_x = x;
    while (*str) {
        if (*str == '\n') {
            cursor_x = x;
            y += FONT_CHAR_HEIGHT + 2;
        } else {
            draw_char(cursor_x, y, *str, color, bg);
            cursor_x += FONT_CHAR_WIDTH + 1;
        }
        str++;
    }
}

void SSD1963::draw_string_centered(uint16_t center_x, uint16_t y, const char* str,
                                    uint16_t color, uint16_t bg) {
    uint16_t len = 0;
    const char* p = str;
    while (*p) {
        if (*p != '\n') len++;
        p++;
    }
    uint16_t text_width = len * (FONT_CHAR_WIDTH + 1);
    uint16_t start_x = center_x - (text_width / 2);
    draw_string(start_x, y, str, color, bg);
}

// =========================================================================
// Renderizado de texto con escala
// =========================================================================
void SSD1963::draw_char_scaled(uint16_t x, uint16_t y, char chr,
                               uint16_t color, uint16_t bg, uint8_t scale) {
    if (chr < 0x20 || chr > 0x7E) chr = 0x20;
    uint8_t idx = chr - 0x20;
    uint8_t sz = scale;

    for (uint8_t col = 0; col < 8; col++) {
        uint8_t bits = m_font_8x8[idx][col];
        for (uint8_t row = 0; row < 8; row++) {
            uint16_t px = x + col * sz;
            uint16_t py = y + row * sz;
            if (bits & (1 << row)) {
                fill_rect(px, py, sz, sz, color);
            } else if (bg != 0xFFFF) {
                fill_rect(px, py, sz, sz, bg);
            }
        }
    }
}

void SSD1963::draw_string_scaled(uint16_t x, uint16_t y, const char* str,
                                 uint16_t color, uint16_t bg, uint8_t scale) {
    uint16_t cx = x;
    uint8_t step = 8 * scale + scale;
    while (*str) {
        if (*str == '\n') {
            cx = x;
            y += 8 * scale + scale;
        } else {
            draw_char_scaled(cx, y, *str, color, bg, scale);
            cx += step;
        }
        str++;
    }
}

void SSD1963::draw_string_centered_scaled(uint16_t center_x, uint16_t y, const char* str,
                                          uint16_t color, uint16_t bg, uint8_t scale) {
    uint16_t len = 0;
    const char* p = str;
    while (*p) { if (*p != '\n') len++; p++; }
    uint16_t text_w = len * (8 * scale + scale);
    uint16_t sx = (center_x > text_w / 2) ? center_x - text_w / 2 : 0;
    draw_string_scaled(sx, y, str, color, bg, scale);
}

// =========================================================================
// Visualización de imágenes
// =========================================================================
void SSD1963::draw_image_rgb565(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        std::cerr << "[SSD1963] Error: no se pudo abrir " << filepath << std::endl;
        return;
    }

    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    write_command(SSD1963_WRITE_MEMORY_START);

    for (uint32_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        uint8_t high, low;
        if (fread(&high, 1, 1, file) != 1) break;
        if (fread(&low, 1, 1, file) != 1) break;

        uint16_t color = ((uint16_t)high << 8) | low;
        write_data(color);
    }

    fclose(file);
}
