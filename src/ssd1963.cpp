#include <unistd.h>
#include <iostream>
#include <cstring>
#include <config_hw.hpp>
#include <ssd1963_cmd.hpp>
#include <ssd1963.hpp>
#include <color.hpp>
#include <font_8x8.hpp>

// Retardo para timing del bus 8080 (WR pulse width + data hold)
#define BUS_WAIT  do { __asm__ __volatile__( \
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n" \
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n" \
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop" );\
} while(0)

// Retardo 74HC595 timing (~1-2us per toggle) - stable for continuous operation
#define SR_WAIT  do { __asm__ __volatile__( \
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n" \
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n" \
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n" \
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n" \
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop" );\
} while(0)

static void shift_out_16bits(uint16_t data);

SSD1963::SSD1963() : m_shift_count(0) {}

void SSD1963::delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

void SSD1963::setup_gpio() {
    bcm2835_gpio_fsel(SSD1963_SR_SER, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_SR_SRCLK, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_SR_RCLK, BCM2835_GPIO_FSEL_OUTP);

    bcm2835_gpio_fsel(SSD1963_LCD_RS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_WR, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_CS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_RESET, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_BACKLIGHT, BCM2835_GPIO_FSEL_OUTP);

    SR_SER_LOW();
    SR_SRCLK_LOW();
    SR_RCLK_LOW();
    CS_HIGH();
    WR_HIGH();
    RS_HIGH();
    RESET_HIGH();
    BACKLIGHT_OFF();
}

void SSD1963::init() {
    // === COMPLETE CLEAN SLATE INITIALIZATION ===
    // 1. Clear 74HC595 shift registers FIRST
    printf("[INIT] Clearing 74HC595 shift registers...\n");
    shift_out_16bits(0x0000);
    shift_out_16bits(0x0000);
    delay_ms(10);

    // 2. Hardware reset SSD1963
    printf("[INIT] Hardware reset SSD1963...\n");
    RESET_LOW();
    delay_ms(50);
    RESET_HIGH();
    delay_ms(120);  // wait for internal POR

    // 3. Soft reset
    write_command(SSD1963_SOFT_RESET);
    delay_ms(20);

    // 4. Exit sleep mode
    write_command(SSD1963_EXIT_SLEEP_MODE);
    delay_ms(20);

    // 5. Re-clear shift registers after reset
    shift_out_16bits(0x0000);
    delay_ms(5);

    // --- PLL startup sequence (SINGLE, CORRECT) ---
    // Ref clk = 12MHz (typical crystal)
    // M = 49 → VCO = 12 * 50 = 600MHz (250-800MHz OK)
    // N = 4  → PLL = 600 / 5 = 120MHz
    printf("[INIT] Configuring PLL (M=49, N=4, PLL=120MHz)...\n");
    write_command(SSD1963_SET_PLL_MN);
    write_data(49);   // M = 49 (0x31)
    write_data(4);    // N = 4  (0x04)
    write_data(0x04); // Effectuate

    write_command(SSD1963_SET_PLL);
    write_data(0x01); // Enable PLL
    delay_ms(5);      // Wait 100us min, use 5ms

    write_command(SSD1963_SET_PLL);
    write_data(0x03); // Lock PLL (use PLL as sysclk)
    delay_ms(100);    // Wait for PLL lock

    // --- LCD panel mode ---
#if USE_8BIT_MODE
    write_command(SSD1963_SET_LCD_MODE);
    write_data(0x08);    // TFT mode, 18-bit panel (A[5]=0), 8-bit MCU bus
    write_data(0x00);    // TFT type: 00/01 = TFT mode
    write_data((LCD_WIDTH - 1) >> 8);
    write_data((LCD_WIDTH - 1) & 0xFF);
    write_data((LCD_HEIGHT - 1) >> 8);
    write_data((LCD_HEIGHT - 1) & 0xFF);
    write_data(0x00);    // RGB sequence

    write_command(SSD1963_SET_PIXEL_DATA_INTERFACE);
    write_data(SSD1963_PDI_8BIT);
#else
    // 16-bit MCU bus, 18-bit panel (typical 480x272)
    write_command(SSD1963_SET_LCD_MODE);
    write_data(0x0C);    // A: 18-bit panel, no FRC, LSHIFT falling, LLINE/LFRAME active low
    write_data(0x00);    // B: TFT mode (00/01)
    write_data((LCD_WIDTH - 1) >> 8);   // HDP high = 0x01
    write_data((LCD_WIDTH - 1) & 0xFF); // HDP low  = 0xDF (479)
    write_data((LCD_HEIGHT - 1) >> 8);  // VDP high = 0x01
    write_data((LCD_HEIGHT - 1) & 0xFF); // VDP low  = 0x0F (271)
    write_data(0x00);    // RGB sequence

    write_command(SSD1963_SET_PIXEL_DATA_INTERFACE);
    write_data(SSD1963_PDI_16BIT565);
#endif

    // --- Pixel clock (LSHIFT frequency) ---
    // PCLK = PLL * (LCD_FPR + 1) / 2^20
    // For 480x272@60Hz: ~9MHz pixel clock
    // With PLL=120MHz: LCD_FPR = 0x13332 ≈ 9MHz
    printf("[INIT] Setting LSHIFT freq (LCD_FPR=0x%05X)...\n", LCD_FPR);
    write_command(SSD1963_SET_LSHIFT_FREQ);
    write_data((LCD_FPR >> 16) & 0xFF);
    write_data((LCD_FPR >> 8) & 0xFF);
    write_data(LCD_FPR & 0xFF);

    // --- Horizontal timing ---
    // HT = 525, HPS = 43, HPW = 41, LPS = 0
    printf("[INIT] Setting horizontal timing (HT=%d, HPS=%d, HPW=%d)...\n",
           TFT_HSYNC_PERIOD, TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH, TFT_HSYNC_PULSE);
    write_command(SSD1963_SET_HOR_PERIOD);
    write_data((TFT_HSYNC_PERIOD >> 8) & 0xFF);
    write_data(TFT_HSYNC_PERIOD & 0xFF);
    write_data((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH) >> 8);
    write_data((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH) & 0xFF);
    write_data(TFT_HSYNC_PULSE);
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);

    // --- Vertical timing ---
    // VT = 286, VPS = 12, VPW = 10, FPS = 0
    printf("[INIT] Setting vertical timing (VT=%d, VPS=%d, VPW=%d)...\n",
           TFT_VSYNC_PERIOD, TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH, TFT_VSYNC_PULSE);
    write_command(SSD1963_SET_VER_PERIOD);
    write_data((TFT_VSYNC_PERIOD >> 8) & 0xFF);
    write_data(TFT_VSYNC_PERIOD & 0xFF);
    write_data((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH) >> 8);
    write_data((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH) & 0xFF);
    write_data(TFT_VSYNC_PULSE);
    write_data(0x00);
    write_data(0x00);

    // --- Turn on display ---
    write_command(SSD1963_ON_DISPLAY);
    delay_ms(100);

    // Configure GPIO0 for backlight control (if not using PWM)
    write_command(SSD1963_SET_GPIO_CONF);
    write_data(0x0F); // GPIO0-3 as output, controlled by host
    write_data(0x01); // GPIO0 as output

    printf("[INIT] SSD1963 initialization complete\n");

    // Backlight ON (using SSD1963 GPIO0)
    write_command(SSD1963_SET_GPIO_VALUE);
    write_data(0x01); // GPIO0 = HIGH

    BACKLIGHT_ON(); // Also set Pi GPIO12
}

void SSD1963::shutdown() {
    printf("[SHUTDOWN] Turning off display and clearing shift registers...\n");

    // Turn off display
    write_command(SSD1963_BLANK_DISPLAY);
    delay_ms(10);

    // Turn off backlight via SSD1963 GPIO0
    write_command(SSD1963_SET_GPIO_VALUE);
    write_data(0x00); // GPIO0 = LOW

    // Clear shift registers
    shift_out_16bits(0x0000);
    shift_out_16bits(0x0000);
    delay_ms(10);

    // Hardware reset
    RESET_LOW();
    delay_ms(50);
    RESET_HIGH();
    delay_ms(50);

    // Turn off Pi backlight GPIO
    BACKLIGHT_OFF();
}

void SSD1963::shift_out_16bits(uint16_t data) {
#if USE_8BIT_MODE
    // 8-bit mode: only first 74HC595 (D0-D7)
    // Send lower 8 bits only, MSB first (bit 7 -> bit 0)
    for (int8_t i = 7; i >= 0; i--) {
        if (data & (1 << i)) {
            SR_SER_HIGH();
        } else {
            SR_SER_LOW();
        }
        SR_WAIT;
        SR_SRCLK_HIGH();
        SR_WAIT;
        SR_SRCLK_LOW();
    }
#else
    // 16-bit mode: two 74HC595 in series (D0-D15)
    // First 74HC595 Q0-Q7 = D0-D7
    // Second 74HC595 Q0-Q7 = D8-D15
    // Connection: SER -> 1st chip -> Q7' -> SER 2nd chip -> 2nd chip
    for (int8_t i = 15; i >= 0; i--) {
        if (data & (1 << i)) {
            SR_SER_HIGH();
        } else {
            SR_SER_LOW();
        }
        SR_WAIT;
        SR_SRCLK_HIGH();
        SR_WAIT;
        SR_SRCLK_LOW();
    }
#endif
    // Latch the data to outputs
    SR_RCLK_HIGH();
    SR_WAIT;
    SR_WAIT;  // Extra hold for RCLK high
    SR_RCLK_LOW();
    SR_WAIT;  // Extra hold time after latch
}

void SSD1963::write_data_bus(uint16_t data) {
    // Periodic re-sync every 50000 shift-outs to prevent drift
    if (++m_shift_count >= 50000) {
        m_shift_count = 0;
        // Re-sync: clear then set all bits
        shift_out_16bits(0x0000);
        shift_out_16bits(0xFFFF);
        shift_out_16bits(data);
    } else {
        shift_out_16bits(data);
    }
}

void SSD1963::write_command(uint8_t cmd) {
    RS_LOW();
    CS_LOW();
    write_data_bus(cmd);
    WR_LOW();
    BUS_WAIT;
    WR_HIGH();
    BUS_WAIT;
    CS_HIGH();
}

void SSD1963::write_data(uint16_t data) {
    RS_HIGH();
    CS_LOW();
    write_data_bus(data);
    WR_LOW();
    BUS_WAIT;
    WR_HIGH();
    BUS_WAIT;
    CS_HIGH();
}

void SSD1963::write_pixel_burst_start() {
    RS_HIGH();
    CS_LOW();
}

void SSD1963::write_pixel_burst(uint16_t data) {
    write_data_bus(data);
    WR_LOW();
    BUS_WAIT;
    WR_HIGH();
    BUS_WAIT;
}

void SSD1963::write_pixel_burst_end() {
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

    delay_ms(1);
}

// =========================================================================
// Operaciones de dibujo (usando write_data estándar con CS toggling)
// =========================================================================

static void fill_burst(SSD1963* d, uint32_t count, uint16_t color) {
    d->write_pixel_burst_start();
    for (uint32_t i = 0; i < count; i++)
        d->write_pixel_burst(color);
    d->write_pixel_burst_end();
}

static void pixels_burst(SSD1963* d, uint32_t count, const uint16_t* data) {
    d->write_pixel_burst_start();
    for (uint32_t i = 0; i < count; i++)
        d->write_pixel_burst(data[i]);
    d->write_pixel_burst_end();
}

void SSD1963::clear_screen(uint16_t color) {
    const uint16_t ROWS_PER_BATCH = 32;
    for (uint16_t y = 0; y < LCD_HEIGHT; y += ROWS_PER_BATCH) {
        uint16_t h = ROWS_PER_BATCH;
        if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
        set_area(0, y, LCD_WIDTH - 1, y + h - 1);
        write_command(SSD1963_WRITE_MEMORY_START);
        fill_burst(this, (uint32_t)LCD_WIDTH * h, color);
        delay_ms(1);
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
    fill_burst(this, (uint32_t)width * height, color);
}

void SSD1963::fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    draw_block(x, y, width, height, color);
}

void SSD1963::draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    draw_block(x, y, width, 1, color);
    draw_block(x, y + height - 1, width, 1, color);
    draw_block(x, y, 1, height, color);
    draw_block(x + width - 1, y, 1, height, color);
}

// =========================================================================
// Renderizado de texto optimizado
// =========================================================================

void SSD1963::draw_char(uint16_t x, uint16_t y, char chr, uint16_t color, uint16_t bg) {
    if (chr < 0x20 || chr > 0x7E) chr = 0x20;
    uint8_t idx = chr - 0x20;

    if (bg == 0xFFFF) {
        for (uint8_t row = 0; row < FONT_CHAR_HEIGHT; row++) {
            uint8_t bits = FONT_8X8[idx][row];
            for (uint8_t col = 0; col < FONT_CHAR_WIDTH; col++) {
                if (bits & (1 << (7 - col)))
                    draw_pixel(x + col, y + row, color);
            }
        }
    } else {
        uint16_t buf[FONT_CHAR_WIDTH * FONT_CHAR_HEIGHT];
        for (uint8_t row = 0; row < FONT_CHAR_HEIGHT; row++) {
            uint8_t bits = FONT_8X8[idx][row];
            for (uint8_t col = 0; col < FONT_CHAR_WIDTH; col++) {
                buf[row * FONT_CHAR_WIDTH + col] =
                    (bits & (1 << (7 - col))) ? color : bg;
            }
        }
        draw_pixels(x, y, FONT_CHAR_WIDTH, FONT_CHAR_HEIGHT, buf);
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

void SSD1963::draw_char_scaled(uint16_t x, uint16_t y, char chr,
                               uint16_t color, uint16_t bg, uint8_t scale) {
    if (chr < 0x20 || chr > 0x7E) chr = 0x20;
    uint8_t idx = chr - 0x20;
    uint8_t sz = scale;

    for (uint8_t row = 0; row < 8; row++) {
        uint8_t bits = FONT_8X8[idx][row];
        for (uint8_t col = 0; col < 8; col++) {
            uint16_t px = x + col * sz;
            uint16_t py = y + row * sz;
            if (bits & (1 << (7 - col))) {
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

void SSD1963::draw_pixels(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                          const uint16_t* data) {
    set_area(x, y, x + w - 1, y + h - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    pixels_burst(this, (uint32_t)w * h, data);
}

void SSD1963::draw_buffer(const uint16_t* data) {
    // Dividir en filas individuales para evitar ráfaga larga que corrompe el display
    for (uint16_t y = 0; y < LCD_HEIGHT; y++) {
        set_area(0, y, LCD_WIDTH - 1, y);
        write_command(SSD1963_WRITE_MEMORY_START);
        pixels_burst(this, LCD_WIDTH, data + (uint32_t)y * LCD_WIDTH);
    }
}

void SSD1963::draw_image_rgb565(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        std::cerr << "[SSD1963] Error: no se pudo abrir " << filepath << std::endl;
        return;
    }

    uint8_t hdr[4];
    if (fread(hdr, 1, 4, file) != 4) {
        std::cerr << "[SSD1963] Error leyendo header de " << filepath << std::endl;
        fclose(file);
        return;
    }
    uint16_t w = (hdr[0] << 8) | hdr[1];
    uint16_t h = (hdr[2] << 8) | hdr[3];

    if (w != LCD_WIDTH || h != LCD_HEIGHT) {
        std::cerr << "[SSD1963] Advertencia: " << filepath
                  << " es " << w << "x" << h
                  << ", se esperaba " << LCD_WIDTH << "x" << LCD_HEIGHT << std::endl;
    }

    uint16_t draw_w = (w > LCD_WIDTH) ? LCD_WIDTH : w;
    uint16_t draw_h = (h > LCD_HEIGHT) ? LCD_HEIGHT : h;

    set_area(0, 0, draw_w - 1, draw_h - 1);
    write_command(SSD1963_WRITE_MEMORY_START);

    write_pixel_burst_start();
    for (uint32_t i = 0; i < (uint32_t)draw_w * draw_h; i++) {
        uint8_t hi, lo;
        if (fread(&hi, 1, 1, file) != 1 || fread(&lo, 1, 1, file) != 1) break;
        uint16_t color = (hi << 8) | lo;
        write_pixel_burst(color);
    }
    write_pixel_burst_end();

    fclose(file);
}
