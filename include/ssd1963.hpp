#pragma once

#include <cstdint>
#include <bcm2835.h>
#include "config_hw.hpp"
#include "ssd1963_commands.hpp"

#define SSD1963_NOP_CMD                      0x00
#define SSD1963_SOFT_RESET_CMD               0x01
#define SSD1963_GET_PWR_MODE_CMD             0x0A
#define SSD1963_GET_ADDR_MODE_CMD            0x0B
#define SSD1963_GET_PIXEL_FORMAT_CMD         0x0C
#define SSD1963_GET_DISPLAY_MODE_CMD         0x0D
#define SSD1963_GET_SIGNAL_MODE_CMD          0x0E
#define SSD1963_ENTER_SLEEP_MODE_CMD         0x10
#define SSD1963_EXIT_SLEEP_MODE_CMD          0x11
#define SSD1963_ENTER_PARTIAL_MODE_CMD       0x12
#define SSD1963_ENTER_NORMAL_MODE_CMD        0x13
#define SSD1963_EXIT_INVERT_MODE_CMD         0x20
#define SSD1963_ENTER_INVERT_MODE_CMD        0x21
#define SSD1963_SET_GAMMA_CMD                0x26
#define SSD1963_BLANK_DISPLAY_CMD            0x28
#define SSD1963_ON_DISPLAY_CMD               0x29
#define SSD1963_SET_COLUMN_ADDRESS_CMD       0x2A
#define SSD1963_SET_PAGE_ADDRESS_CMD         0x2B
#define SSD1963_WRITE_MEMORY_START_CMD       0x2C
#define SSD1963_READ_MEMORY_START_CMD        0x2E
#define SSD1963_SET_PARTIAL_AREA_CMD         0x30
#define SSD1963_SET_SCROLL_AREA_CMD          0x33
#define SSD1963_SET_TEAR_OFF_CMD             0x34
#define SSD1963_SET_TEAR_ON_CMD              0x35
#define SSD1963_SET_ADDR_MODE_CMD            0x36
#define SSD1963_SET_SCROLL_START_CMD         0x37
#define SSD1963_EXIT_IDLE_MODE_CMD           0x38
#define SSD1963_ENTER_IDLE_MODE_CMD          0x39
#define SSD1963_SET_PIXEL_FORMAT_CMD         0x3A
#define SSD1963_WRITE_MEMORY_CONTINUE_CMD    0x3C
#define SSD1963_READ_MEMORY_CONTINUE_CMD     0x3E
#define SSD1963_SET_TEAR_SCANLINE_CMD        0x44
#define SSD1963_GET_SCANLINE_CMD             0x45
#define SSD1963_RD_DDB_START_CMD             0xA1
#define SSD1963_RD_DDB_AUTO_CMD              0xA8
#define SSD1963_SET_PANEL_MODE_CMD           0xB0
#define SSD1963_GET_PANEL_MODE_CMD           0xB1
#define SSD1963_SET_HOR_PERIOD_CMD           0xB4
#define SSD1963_GET_HOR_PERIOD_CMD           0xB5
#define SSD1963_SET_VER_PERIOD_CMD           0xB6
#define SSD1963_GET_VER_PERIOD_CMD           0xB7
#define SSD1963_SET_GPIO_CONF_CMD            0xB8
#define SSD1963_GET_GPIO_CONF_CMD            0xB9
#define SSD1963_SET_GPIO_VAL_CMD             0xBA
#define SSD1963_GET_GPIO_STATUS_CMD          0xBB
#define SSD1963_SET_POST_PROC_CMD            0xBC
#define SSD1963_GET_POST_PROC_CMD            0xBD
#define SSD1963_SET_PWM_CONF_CMD             0xBE
#define SSD1963_GET_PWM_CONF_CMD             0xBF
#define SSD1963_GET_LCD_GEN0_CMD             0xC0
#define SSD1963_SET_LCD_GEN0_CMD             0xC1
#define SSD1963_GET_LCD_GEN1_CMD             0xC2
#define SSD1963_SET_LCD_GEN1_CMD             0xC3
#define SSD1963_GET_LCD_GEN2_CMD             0xC4
#define SSD1963_SET_LCD_GEN2_CMD             0xC5
#define SSD1963_GET_LCD_GEN3_CMD             0xC6
#define SSD1963_SET_LCD_GEN3_CMD             0xC7
#define SSD1963_SET_GPIO0_ROP_CMD            0xC8
#define SSD1963_GET_GPIO0_ROP_CMD            0xC9
#define SSD1963_SET_GPIO1_ROP_CMD            0xCA
#define SSD1963_GET_GPIO1_ROP_CMD            0xCB
#define SSD1963_SET_GPIO2_ROP_CMD            0xCC
#define SSD1963_GET_GPIO2_ROP_CMD            0xCD
#define SSD1963_SET_GPIO3_ROP_CMD            0xCE
#define SSD1963_GET_GPIO3_ROP_CMD            0xCF
#define SSD1963_SET_ABC_DBC_CONF_CMD         0xD0
#define SSD1963_GET_ABC_DBC_CONF_CMD         0xD1
#define SSD1963_SET_DBC_HISTO_PTR_CMD        0xD2
#define SSD1963_GET_DBC_HISTO_PTR_CMD        0xD3
#define SSD1963_SET_DBC_THRES_CMD            0xD4
#define SSD1963_GET_DBC_THRES_CMD            0xD5
#define SSD1963_SET_ABM_TMR_CMD              0xD6
#define SSD1963_GET_ABM_TMR_CMD              0xD7
#define SSD1963_SET_AMB_LVL0_CMD             0xD8
#define SSD1963_GET_AMB_LVL0_CMD             0xD9
#define SSD1963_SET_AMB_LVL1_CMD             0xDA
#define SSD1963_GET_AMB_LVL1_CMD             0xDB
#define SSD1963_SET_AMB_LVL2_CMD             0xDC
#define SSD1963_GET_AMB_LVL2_CMD             0xDD
#define SSD1963_SET_AMB_LVL3_CMD             0xDE
#define SSD1963_GET_AMB_LVL3_CMD             0xDF
#define SSD1963_PLL_START_CMD                0xE0
#define SSD1963_PLL_STOP_CMD                 0xE1
#define SSD1963_SET_PLL_MN_CMD               0xE2
#define SSD1963_GET_PLL_MN_CMD               0xE3
#define SSD1963_GET_PLL_STATUS_CMD           0xE4
#define SSD1963_ENT_DEEP_SLEEP_CMD           0xE5
#define SSD1963_SET_PCLK_CMD                 0xE6
#define SSD1963_GET_PCLK_CMD                 0xE7
#define SSD1963_SET_DATA_INTERFACE_CMD       0xF0
#define SSD1963_GET_DATA_INTERFACE_CMD       0xF1
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

    void wr_strobe();
    void cs_low();
    void cs_high();
    void rs_low();
    void rs_high();
    void reset_low();
    void reset_high();
    void backlight_on_gpio();
    void backlight_off_gpio();
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