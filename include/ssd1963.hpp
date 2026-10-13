#pragma once

#include <cstdint>
#include <bcm2835.h>
#include "config_hw.hpp"
#include "ssd1963_commands.hpp"
#include "color.hpp"

class SSD1963 {
public:
    SSD1963();
    ~SSD1963();

    void setup_gpio();
    void init();

    void clear_screen(uint16_t color);
    void draw_block(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
    void draw_pixel(uint16_t x, uint16_t y, uint16_t color);
    void draw_image_rgb565(const char* filepath);
    void draw_buffer(const uint16_t* buffer);
    void fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
    void draw_pixels(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* pixels);

    void delay_ms(uint32_t ms);

private:
    void write_data_bus(uint16_t data);
    void write_command(uint8_t cmd);
    void write_data(uint16_t data);
    void set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    void hardware_reset();
    void soft_reset();
    void init_pll();
    void init_lcd_mode();
    void init_pixel_interface();
    void init_pixel_clock();
    void init_timing();
    void display_on();
    void backlight_on();
};

#define RS_LOW()    bcm2835_gpio_write(SSD1963_LCD_RS, LOW)
#define RS_HIGH()   bcm2835_gpio_write(SSD1963_LCD_RS, HIGH)
#define WR_LOW()    bcm2835_gpio_write(SSD1963_LCD_WR, LOW)
#define WR_HIGH()   bcm2835_gpio_write(SSD1963_LCD_WR, HIGH)
#define CS_LOW()    bcm2835_gpio_write(SSD1963_LCD_CS, LOW)
#define CS_HIGH()   bcm2835_gpio_write(SSD1963_LCD_CS, HIGH)
#define RESET_LOW() bcm2835_gpio_write(SSD1963_LCD_RESET, LOW)
#define RESET_HIGH() bcm2835_gpio_write(SSD1963_LCD_RESET, HIGH)
#define BACKLIGHT_ON() bcm2835_gpio_write(SSD1963_LCD_BACKLIGHT, HIGH)
#define BACKLIGHT_OFF() bcm2835_gpio_write(SSD1963_LCD_BACKLIGHT, LOW)