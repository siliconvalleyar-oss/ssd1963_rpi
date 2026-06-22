// ssd1963.hpp
#pragma once
#include <bcm2835.h>
#include <cstdint>


//SSD1963_PIN_RST pin128_ssd1963_smd  CONF 1=8080 0:6800
//SSD1963_PIN_RD pin121_ssd1963_smd  Enable signal (RD)6800 mode: E (enable signal)
//8080 mode: RD# (read strobe signal)
//SSD1963_PIN_RW  pin120_ssd1963_smd  R/W
//SSD1963_PIN_CS  pin123_ssd1963_smd Chip Select
//SSD1963_PIN_RS  pin123_ssd1963_smd Data command




// Macros de control
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


class SSD1963 {
public:

    SSD1963();
    void init();
    void setup_gpio();
    void clear_screen(uint16_t color);
    void draw_block(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
    void delay_ms(uint32_t ms);
	void draw_pixel(uint16_t x, uint16_t y, uint16_t color);
    void draw_image_rgb565(const char* filepath) ;
    
private:
    
    void write_data_bus(uint16_t data);
    void write_command(uint8_t cmd);
    void write_data(uint16_t data);
    void set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void define_constants();

};

