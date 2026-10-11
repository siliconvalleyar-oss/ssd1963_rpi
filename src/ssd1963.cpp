#include <unistd.h>
#include <iostream>
#include <cstring>
#include <config_hw.hpp>
#include <ssd1963_cmd.hpp>
#include <ssd1963.hpp>
#include <color.hpp>
#include <font_8x8.hpp>

// Retardo para timing del bus 8080 (WR pulse width + data hold)
#define BUS_WAIT  do { bcm2835_delayMicroseconds(1); } while(0)

SSD1963::SSD1963() {}

void SSD1963::delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

void SSD1963::setup_gpio() {
    // Data bus D0-D15 (GPIO 12-27)
    for (uint8_t pin = SSD1963_LCD_D0; pin <= SSD1963_LCD_D15; pin++) {
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
    }
    // Control signals
    bcm2835_gpio_fsel(SSD1963_LCD_RS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_WR, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_RD, BCM2835_GPIO_FSEL_OUTP);   // RD pin (GPIO2)
    bcm2835_gpio_fsel(SSD1963_LCD_CS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_RESET, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_BACKLIGHT, BCM2835_GPIO_FSEL_OUTP);
    // Status inputs
    bcm2835_gpio_fsel(SSD1963_LCD_TE, BCM2835_GPIO_FSEL_INPT);   // TE pin (GPIO1)
    bcm2835_gpio_fsel(SSD1963_LCD_PW1, BCM2835_GPIO_FSEL_INPT);  // PW1 pin (GPIO7)

    // Initialize control signals to inactive state
    CS_HIGH();
    WR_HIGH();
    RD_HIGH();   // RD inactive HIGH (read disabled)
    RS_HIGH();
    RESET_LOW(); // Start with reset LOW
    BACKLIGHT_ON();  // Backlight ON at startup
}

void SSD1963::init() {
    // === COMPLETE CLEAN SLATE INITIALIZATION ===
    
    // 1. Set control signals to inactive state FIRST
    printf("[INIT] Setting control signals: RD=HIGH, WR=HIGH, CS=HIGH...\n");
    RD_HIGH();
    WR_HIGH();
    CS_HIGH();
    
    // 2. Hardware reset sequence: RST LOW initially
    printf("[INIT] Hardware reset SSD1963 (RST LOW 2000ms)...\n");
    RESET_LOW();
    
    // 3. Wait 2 seconds with RST LOW (no commands - SSD1963 ignores all during reset)
    printf("[INIT] Waiting 2 seconds (RST LOW)...\n");
    delay_ms(2000);  // 2 seconds with RST LOW
    
    // 4. Release reset
    printf("[INIT] Releasing reset (RST HIGH)...\n");
    RESET_HIGH();
    delay_ms(200);   // Wait for internal POR after reset release
    
    // 5. Soft reset (does not affect PLL registers 0xE0-0xE5)
    write_command(SSD1963_SOFT_RESET);
    delay_ms(20);   // datasheet requires >= 5ms

    // 6. Exit sleep mode (also triggers display on 0x29)
    write_command(SSD1963_EXIT_SLEEP_MODE);
    delay_ms(20);   // datasheet requires >= 5ms

    // --- PLL startup sequence (10MHz crystal, PLL=100MHz) ---
    // Ref clk = 10MHz (physical crystal on SSD1963 modules)
    // PLL = 10MHz * (M+1) / (N+1)
    // For 100MHz: M=29, N=2 -> 10MHz * 30 / 3 = 100MHz
    // Reference uses: N=2 (0x02), M=29 (0x1D), effectuate=0x04
    printf("[INIT] Configuring PLL (M=29, N=2, PLL=100MHz)...\n");
    write_command(SSD1963_SET_PLL_MN);
    write_data(SSD1963_PLL_N);   // N = 2  (0x02)
    write_data(SSD1963_PLL_M);   // M = 29 (0x1D)
    write_data(0x04); // C[2]=1: effectuate M and N

    write_command(SSD1963_SET_PLL);
    write_data(0x01); // Enable PLL
    delay_ms(10);     // Wait for PLL to stabilize

    write_command(SSD1963_SET_PLL);
    write_data(0x03); // Lock PLL (use PLL as sysclk)
    delay_ms(12);     // Wait for PLL lock

    // --- LCD panel mode (matching working reference) ---
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
        // 16-bit MCU bus, 24-bit panel mode (matching reference)
        write_command(SSD1963_SET_LCD_MODE);
        write_data(LCD_MODE_CONFIG);  // 0x20 = 24-bit panel, LSHIFT falling, sync active low
        write_data(0x00);    // TFT type: 00/01 = TFT mode
        write_data((LCD_WIDTH - 1) >> 8);   // HDP high = 0x01
        write_data((LCD_WIDTH - 1) & 0xFF); // HDP low  = 0xDF (479)
        write_data((LCD_HEIGHT - 1) >> 8);  // VDP high = 0x01
        write_data((LCD_HEIGHT - 1) & 0xFF); // VDP low  = 0x0F (271)
        write_data(0x00);    // RGB sequence

        write_command(SSD1963_SET_PIXEL_DATA_INTERFACE);
        write_data(SSD1963_PDI_16BIT565);  // 0x03 = 16-bit 565 format
    #endif

    // 6b. Address mode: RGB, top->bottom, left->right, sin flips
    write_command(SSD1963_SET_ADDR_MODE);   // 0x36
    write_data(0x00);  // RGB, top->bottom, left->right, sin flips
    // Si R y B salen intercambiados, cambiar 0x00 por 0x08 (BGR).

    // --- Pixel clock (LSHIFT frequency) ---
    // PCLK = PLL * (LCD_FPR + 1) / 2^20
    // Reference: PLL=150MHz, FPR=0x00FFBE -> PCLK ≈ 9.3MHz
    printf("[INIT] Setting LSHIFT freq (LCD_FPR=0x%05X)...\n", LCD_FPR);
    write_command(SSD1963_SET_LSHIFT_FREQ);
    write_data((LCD_FPR >> 16) & 0xFF);
    write_data((LCD_FPR >> 8) & 0xFF);
    write_data(LCD_FPR & 0xFF);

    // --- Horizontal timing (matching reference) ---
    // HT = 480 + 43 + 8 + 1 = 532 (but HT register stores HT-1 = 531 = 0x0213)
    // HPS = 43 (back porch)
    // HPW = 1-1 = 0
    // LPS = 0
    printf("[INIT] Setting horizontal timing (HT=%d, HPS=%d, HPW=%d)...\n",
           TFT_HSYNC_PERIOD, TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH, TFT_HSYNC_PULSE);
    write_command(SSD1963_SET_HOR_PERIOD);
    // HT-1
    write_data(((TFT_HSYNC_PERIOD - 1) >> 8) & 0xFF);
    write_data((TFT_HSYNC_PERIOD - 1) & 0xFF);
    // HPS = pulse + back porch
    write_data((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH) >> 8);
    write_data((TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH) & 0xFF);
    // HPW = pulse - 1
    write_data(TFT_HSYNC_PULSE - 1);
    write_data(0x00);   // LPS high
    write_data(0x00);   // LPS low
    write_data(0x00);   // LPSPP

    // --- Vertical timing ---
    // VT = 272 + 12 + 8 + 1 = 293 (VT-1 = 292 = 0x0124)
    // VPS = 12 (back porch)
    // VPW = 1-1 = 0
    // FPS = 0
    printf("[INIT] Setting vertical timing (VT=%d, VPS=%d, VPW=%d)...\n",
           TFT_VSYNC_PERIOD, TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH, TFT_VSYNC_PULSE);
    write_command(SSD1963_SET_VER_PERIOD);
    // VT-1
    write_data(((TFT_VSYNC_PERIOD - 1) >> 8) & 0xFF);
    write_data((TFT_VSYNC_PERIOD - 1) & 0xFF);
    // VPS = pulse + back porch
    write_data((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH) >> 8);
    write_data((TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH) & 0xFF);
    // VPW = pulse - 1
    write_data(TFT_VSYNC_PULSE - 1);
    write_data(0x00);   // FPS high
    write_data(0x00);   // FPS low

    // --- SET_ADDR_MODE (0x36) - required for correct color/orientation ---
    write_command(SSD1963_SET_ADDR_MODE);  // 0x36
    write_data(0x00);  // RGB, top->bottom, left->right, no flip
    // Si R y B salen intercambiados, cambiar a 0x08 (BGR)

    // --- Turn on display ---
    write_command(SSD1963_ON_DISPLAY);
    delay_ms(100);

    // Configure PWM for backlight (matching reference)
    write_command(SSD1963_SET_PWM_FC);  // 0xBE
    write_data(0x05);     // PWMF = 5
    write_data(0xFE);     // PWM duty = 254/256 (max)
    write_data(0x01);     // C[3]=0 (host), C[0]=1 (enable)
    write_data(0x00);     // D (DBC manual brightness)
    write_data(0x00);     // E (DBC min brightness)
    write_data(0x00);     // F (brightness prescaler)

    // Disable DBC (Dynamic Backlight Control)
    write_command(SSD1963_SET_DBC_CONF);  // 0xD0
    write_data(0x00);     // DBC disabled

    // Configure GPIO0 for backlight enable
    write_command(SSD1963_SET_GPIO_CONF);  // 0xB8
    write_data(0x03);     // GPIO0-1 as output, controlled by host
    write_data(0x01);     // GPIO0 as normal GPIO

    // Set GPIO0 HIGH for panel enable
    write_command(SSD1963_SET_GPIO_VALUE);  // 0xBA
    write_data(0x01);     // GPIO0 = HIGH

    // Backlight ON via Pi GPIO
    BACKLIGHT_ON();

    // Now clear screen to black (GRAM is accessible after init)
    printf("[INIT] Clearing screen to black...\n");
    clear_screen(BLACK);

    printf("[INIT] SSD1963 initialization complete\n");
}

static inline uint32_t data_to_mask(uint16_t data) {
    return ((uint32_t)data) << 12;   // D0=pin12 … D15=pin27
}

void SSD1963::write_data_bus(uint16_t data) {
    static uint32_t g_bus_state = 0;
    uint32_t new_state = ((uint32_t)data) << 12;
    uint32_t to_set = new_state & ~g_bus_state;
    uint32_t to_clr = g_bus_state & ~new_state;
    if (to_clr) bcm2835_gpio_clr_multi(to_clr);
    if (to_set) bcm2835_gpio_set_multi(to_set);
    g_bus_state = new_state;
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
    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    fill_burst(this, (uint32_t)LCD_WIDTH * LCD_HEIGHT, color);
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
    // Single batch for full framebuffer - no per-row overhead
    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    pixels_burst(this, (uint32_t)LCD_WIDTH * LCD_HEIGHT, data);
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
