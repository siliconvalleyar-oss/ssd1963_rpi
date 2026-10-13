#include "ssd1963.hpp"
#include <cstdio>
#include <unistd.h>

SSD1963::SSD1963() {}

SSD1963::~SSD1963() {
    bcm2835_close();
}

void SSD1963::setup_gpio() {
    if (!bcm2835_init()) {
        return;
    }

    const uint8_t data_pins[16] = {
        SSD1963_LCD_D0, SSD1963_LCD_D1, SSD1963_LCD_D2, SSD1963_LCD_D3,
        SSD1963_LCD_D4, SSD1963_LCD_D5, SSD1963_LCD_D6, SSD1963_LCD_D7,
        SSD1963_LCD_D8, SSD1963_LCD_D9, SSD1963_LCD_D10, SSD1963_LCD_D11,
        SSD1963_LCD_D12, SSD1963_LCD_D13, SSD1963_LCD_D14, SSD1963_LCD_D15
    };

    for (int i = 0; i < 16; i++) {
        bcm2835_gpio_fsel(data_pins[i], BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_write(data_pins[i], LOW);
    }

    bcm2835_gpio_fsel(SSD1963_LCD_WR, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_RS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_CS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_RESET, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_BACKLIGHT, BCM2835_GPIO_FSEL_OUTP);

    WR_HIGH();
    RS_HIGH();
    CS_HIGH();
    RESET_HIGH();
    BACKLIGHT_OFF();
}

void SSD1963::init() {
    hardware_reset();
    soft_reset();
    init_pll();
    init_lcd_mode();
    init_pixel_interface();
    init_pixel_clock();
    init_timing();
    display_on();
    backlight_on();
}

void SSD1963::hardware_reset() {
    RESET_LOW();
    delay_ms(10);
    RESET_HIGH();
    delay_ms(120);
}

void SSD1963::soft_reset() {
    write_command(SSD1963_SOFT_RESET);
    delay_ms(5);
}

void SSD1963::init_pll() {
    write_command(SSD1963_SET_PLL_MN);
    write_data(0x31);
    write_data(0x02);
    write_data(0x04);

    write_command(SSD1963_PLL_START);
    write_data(0x01);
    delay_ms(1);
    write_command(SSD1963_PLL_START);
    write_data(0x03);
    delay_ms(5);
}

void SSD1963::init_lcd_mode() {
    write_command(SSD1963_SET_PANEL_MODE);
    write_data(0x20);
    write_data(0x00);
    write_data((LCD_WIDTH - 1) >> 8);
    write_data((LCD_WIDTH - 1) & 0xFF);
    write_data((LCD_HEIGHT - 1) >> 8);
    write_data((LCD_HEIGHT - 1) & 0xFF);
    write_data(0x00);
}

void SSD1963::init_pixel_interface() {
    write_command(SSD1963_SET_DATA_INTERFACE);
    write_data(SSD1963_PDI_16BIT565);
}

void SSD1963::init_pixel_clock() {
    write_command(SSD1963_SET_PCLK);
    write_data((LCD_FPR >> 16) & 0xFF);
    write_data((LCD_FPR >> 8) & 0xFF);
    write_data(LCD_FPR & 0xFF);
}

void SSD1963::init_timing() {
    uint16_t h_period = TFT_HSYNC_PERIOD;
    uint16_t h_pulse = TFT_HSYNC_PULSE;
    uint16_t h_back = TFT_HSYNC_BACK_PORCH;
    uint16_t h_front = TFT_HSYNC_FRONT_PORCH;

    write_command(SSD1963_SET_HOR_PERIOD);
    write_data((h_period - 1) >> 8);
    write_data((h_period - 1) & 0xFF);
    write_data((h_pulse - 1) >> 8);
    write_data((h_pulse - 1) & 0xFF);
    write_data((h_back - 1) >> 8);
    write_data((h_back - 1) & 0xFF);
    write_data((h_front - 1) >> 8);
    write_data((h_front - 1) & 0xFF);
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);

    uint16_t v_period = TFT_VSYNC_PERIOD;
    uint16_t v_pulse = TFT_VSYNC_PULSE;
    uint16_t v_back = TFT_VSYNC_BACK_PORCH;
    uint16_t v_front = TFT_VSYNC_FRONT_PORCH;

    write_command(SSD1963_SET_VER_PERIOD);
    write_data((v_period - 1) >> 8);
    write_data((v_period - 1) & 0xFF);
    write_data((v_pulse - 1) >> 8);
    write_data((v_pulse - 1) & 0xFF);
    write_data((v_back - 1) >> 8);
    write_data((v_back - 1) & 0xFF);
    write_data((v_front - 1) >> 8);
    write_data((v_front - 1) & 0xFF);
    write_data(0x00);
    write_data(0x00);
}

void SSD1963::display_on() {
    write_command(SSD1963_ON_DISPLAY);
}

void SSD1963::backlight_on() {
    BACKLIGHT_ON();
}

void SSD1963::write_data_bus(uint16_t data) {
    const uint8_t pins[16] = {
        SSD1963_LCD_D0, SSD1963_LCD_D1, SSD1963_LCD_D2, SSD1963_LCD_D3,
        SSD1963_LCD_D4, SSD1963_LCD_D5, SSD1963_LCD_D6, SSD1963_LCD_D7,
        SSD1963_LCD_D8, SSD1963_LCD_D9, SSD1963_LCD_D10, SSD1963_LCD_D11,
        SSD1963_LCD_D12, SSD1963_LCD_D13, SSD1963_LCD_D14, SSD1963_LCD_D15
    };

    for (int i = 0; i < 16; i++) {
        bcm2835_gpio_write(pins[i], (data >> i) & 0x01);
    }
}

void SSD1963::write_command(uint8_t cmd) {
    CS_LOW();
    RS_LOW();
    write_data_bus(cmd);
    WR_LOW();
    WR_HIGH();
    CS_HIGH();
}

void SSD1963::write_data(uint16_t data) {
    CS_LOW();
    RS_HIGH();
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

    write_command(SSD1963_WRITE_MEMORY_START);
}

void SSD1963::clear_screen(uint16_t color) {
    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    uint32_t total_pixels = LCD_WIDTH * LCD_HEIGHT;
    for (uint32_t i = 0; i < total_pixels; i++) {
        write_data(color);
    }
}

void SSD1963::draw_block(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + width > LCD_WIDTH) width = LCD_WIDTH - x;
    if (y + height > LCD_HEIGHT) height = LCD_HEIGHT - y;

    set_area(x, y, x + width - 1, y + height - 1);
    uint32_t total = width * height;
    for (uint32_t i = 0; i < total; i++) {
        write_data(color);
    }
}

void SSD1963::draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    set_area(x, y, x, y);
    write_data(color);
}

void SSD1963::draw_image_rgb565(const char* filepath) {
    FILE* fp = fopen(filepath, "rb");
    if (!fp) return;

    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    uint16_t pixel;
    size_t pixels_read = 0;
    const size_t total_pixels = LCD_WIDTH * LCD_HEIGHT;

    while (pixels_read < total_pixels && fread(&pixel, 2, 1, fp) == 1) {
        write_data(__builtin_bswap16(pixel));
        pixels_read++;
    }

    fclose(fp);
}

void SSD1963::delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

void SSD1963::draw_buffer(const uint16_t* buffer) {
    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    uint32_t total_pixels = LCD_WIDTH * LCD_HEIGHT;
    for (uint32_t i = 0; i < total_pixels; i++) {
        write_data(buffer[i]);
    }
}

void SSD1963::fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    draw_block(x, y, width, height, color);
}

void SSD1963::draw_pixels(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* pixels) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + width > LCD_WIDTH) width = LCD_WIDTH - x;
    if (y + height > LCD_HEIGHT) height = LCD_HEIGHT - y;

    set_area(x, y, x + width - 1, y + height - 1);
    uint32_t total = width * height;
    for (uint32_t i = 0; i < total; i++) {
        write_data(pixels[i]);
    }
}