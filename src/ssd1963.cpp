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
    for (uint8_t pin = SSD1963_LCD_D0; pin <= SSD1963_LCD_D15; pin++) {
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
    }
    bcm2835_gpio_fsel(SSD1963_LCD_RS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_WR, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_CS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_RESET, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_BACKLIGHT, BCM2835_GPIO_FSEL_OUTP);

    CS_HIGH();
    WR_HIGH();
    RS_HIGH();
    RESET_HIGH();
    BACKLIGHT_OFF();
}

// == Optimized data bus: 2 GPIO calls instead of 32 ==
void SSD1963::write_data_bus(uint16_t data) {
    bcm2835_gpio_clr_multi(DATA_PIN_MASK);
    bcm2835_gpio_set_multi(((uint32_t)data << 12) & DATA_PIN_MASK);
}

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

// == Batch operations: keep RS/CS high, only toggle WR ==

void SSD1963::write_batch_start() {
    RS_HIGH();
    CS_LOW();
}

void SSD1963::write_batch_end() {
    CS_HIGH();
}

void SSD1963::write_batch_strobe() {
    WR_LOW();
    // ~10ns hold via NOP-like barrier (memory-mapped write is fast enough)
    WR_HIGH();
}

void SSD1963::write_batch_data(uint16_t data) {
    write_data_bus(data);
    write_batch_strobe();
}

// == Fixed: writes 1 value per pixel (was 2 from 8-bit legacy) ==
void SSD1963::clear_screen(uint16_t color) {
    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    write_batch_start();
    for (uint32_t i = 0; i < (uint32_t)LCD_WIDTH * LCD_HEIGHT; i++) {
        write_batch_data(color);
    }
    write_batch_end();
}

void SSD1963::draw_block(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    set_area(x, y, x + width - 1, y + height - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    write_batch_start();
    for (uint32_t i = 0; i < (uint32_t)width * height; i++) {
        write_batch_data(color);
    }
    write_batch_end();
}

void SSD1963::draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    set_area(x, y, x, y);
    write_command(SSD1963_WRITE_MEMORY_START);
    write_data(color);
}

void SSD1963::draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    draw_line_h(x, y, w, color);
    draw_line_h(x, y + h - 1, w, color);
    draw_line_v(x, y, h, color);
    draw_line_v(x + w - 1, y, h, color);
}

void SSD1963::draw_line_h(uint16_t x, uint16_t y, uint16_t len, uint16_t color) {
    set_area(x, y, x + len - 1, y);
    write_command(SSD1963_WRITE_MEMORY_START);
    write_batch_start();
    for (uint16_t i = 0; i < len; i++) {
        write_batch_data(color);
    }
    write_batch_end();
}

void SSD1963::draw_line_v(uint16_t x, uint16_t y, uint16_t len, uint16_t color) {
    set_area(x, y, x, y + len - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    write_batch_start();
    for (uint16_t i = 0; i < len; i++) {
        write_batch_data(color);
    }
    write_batch_end();
}

void SSD1963::init() {
    RESET_LOW();
    delay_ms(10);
    RESET_HIGH();
    delay_ms(10);

    write_command(SSD1963_SOFT_RESET);
    delay_ms(10);

    write_command(SSD1963_SET_PLL_MN);
    write_data(49);
    write_data(4);
    write_data(4);

    write_command(SSD1963_SET_PLL);
    write_data(0x01);
    delay_ms(10);
    write_command(SSD1963_SET_PLL);
    write_data(0x03);
    delay_ms(10);

    write_command(SSD1963_SET_LCD_MODE);
    write_data(0x0C);
    write_data(0x00);
    write_data((LCD_WIDTH - 1) >> 8);
    write_data((LCD_WIDTH - 1) & 0xFF);
    write_data((LCD_HEIGHT - 1) >> 8);
    write_data((LCD_HEIGHT - 1) & 0xFF);
    write_data(0x00);

    write_command(SSD1963_SET_PIXEL_DATA_INTERFACE);
    write_data(SSD1963_PDI_16BIT565);

    write_command(SSD1963_SET_LSHIFT_FREQ);
    write_data((LCD_FPR >> 16) & 0xFF);
    write_data((LCD_FPR >> 8) & 0xFF);
    write_data(LCD_FPR & 0xFF);

    write_command(SSD1963_SET_HOR_PERIOD);
    write_data((TFT_HSYNC_PERIOD >> 8) & 0xFF);
    write_data(TFT_HSYNC_PERIOD & 0xFF);
    write_data((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH) >> 8);
    write_data((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH) & 0xFF);
    write_data(TFT_HSYNC_PULSE);
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);

    write_command(SSD1963_SET_VER_PERIOD);
    write_data((TFT_VSYNC_PERIOD >> 8) & 0xFF);
    write_data(TFT_VSYNC_PERIOD & 0xFF);
    write_data((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH) >> 8);
    write_data((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH) & 0xFF);
    write_data(TFT_VSYNC_PULSE);
    write_data(0x00);
    write_data(0x00);

    write_command(SSD1963_ON_DISPLAY);
    delay_ms(50);
    BACKLIGHT_ON();
}

void SSD1963::draw_image_rgb565(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        std::cerr << "Error: cannot open image: " << filepath << std::endl;
        return;
    }

    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    write_batch_start();

    for (uint32_t i = 0; i < (uint32_t)LCD_WIDTH * LCD_HEIGHT; i++) {
        uint8_t high, low;
        if (fread(&high, 1, 1, file) != 1) break;
        if (fread(&low, 1, 1, file) != 1) break;
        write_batch_data(((uint16_t)high << 8) | low);
    }

    write_batch_end();
    fclose(file);
}
