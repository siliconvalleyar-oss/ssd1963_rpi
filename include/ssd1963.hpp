// ssd1963.hpp
#pragma once
#include <cstdint>
#include <gpio_mmap.hpp>


//SSD1963_PIN_RST pin128_ssd1963_smd  CONF 1=8080 0:6800
//SSD1963_PIN_RD pin121_ssd1963_smd  Enable signal (RD)6800 mode: E (enable signal)
//8080 mode: RD# (read strobe signal)
//SSD1963_PIN_RW  pin120_ssd1963_smd  R/W
//SSD1963_PIN_CS  pin123_ssd1963_smd Chip Select
//SSD1963_PIN_RS  pin123_ssd1963_smd Data command





// Macros de control
#define RS_LOW()    GPIO_CLR(SSD1963_LCD_RS)
#define RS_HIGH()   GPIO_SET(SSD1963_LCD_RS)
#define WR_LOW()    GPIO_CLR(SSD1963_LCD_WR)
#define WR_HIGH()   GPIO_SET(SSD1963_LCD_WR)
#define CS_LOW()    GPIO_CLR(SSD1963_LCD_CS)
#define CS_HIGH()   GPIO_SET(SSD1963_LCD_CS)
#define RESET_LOW() GPIO_CLR(SSD1963_LCD_RESET)
#define RESET_HIGH() GPIO_SET(SSD1963_LCD_RESET)
#define BACKLIGHT_ON()  GPIO_SET(SSD1963_LCD_BACKLIGHT)
#define BACKLIGHT_OFF() GPIO_CLR(SSD1963_LCD_BACKLIGHT)


class SSD1963 {
public:

    SSD1963();
    ~SSD1963();
    void init();
    void setup_gpio();
    void clear_screen(uint16_t color);
    void draw_block(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
    void delay_ms(uint32_t ms);
	void draw_pixel(uint16_t x, uint16_t y, uint16_t color);
    void draw_image_rgb565(const char* filepath) ;
    void draw_buffer(const uint16_t* buffer);
    void fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
    void draw_pixels(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* pixels);
    
private:

    // true = espera ~1 us tras cada strobe WR (durante init, como el PMP lento del PIC32)
    bool slow_bus_ = false;

    void write_data_bus(uint16_t data);
    void write_command(uint8_t cmd);
    void write_data(uint16_t data);
    void write_data_burst(const uint16_t* data, uint32_t count);  // Fast bulk write
    void set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void define_constants();

};