#include <unistd.h>
#include <iostream>
#include <cstring>
#include <config_hw.hpp>
#include <ssd1963_cmd.hpp>
#include <ssd1963.hpp>
#include <color.hpp>
#include <font_8x8.hpp>

// Retardo para timing del bus 8080 (WR pulse width + data hold) - ~50ns
#define BUS_WAIT  do { __asm__ __volatile__( \
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n" \
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n" \
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop" );\
} while(0)

// Retardo 74HC595 timing (~1us per toggle) - stable for shift register
#define SR_WAIT  do { bcm2835_delayMicroseconds(1); } while(0)


SSD1963::SSD1963() {}

void SSD1963::delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

void SSD1963::setup_gpio() {
    bcm2835_gpio_fsel(SSD1963_SR_SER, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_SR_SRCLK, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_SR_RCLK, BCM2835_GPIO_FSEL_OUTP);

    bcm2835_gpio_fsel(SSD1963_LCD_RS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_WR, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_RD, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_CS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_RESET, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_BACKLIGHT, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_TE, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(SSD1963_LCD_PW1, BCM2835_GPIO_FSEL_INPT);

    SR_SER_LOW();
    SR_SRCLK_LOW();
    SR_RCLK_LOW();
    CS_HIGH();
    WR_HIGH();
    RD_HIGH();   // RD inactivo en HIGH (lectura deshabilitada)
    RS_HIGH();
    RESET_HIGH();
    BACKLIGHT_OFF();
}

void SSD1963::init() {
    // === COMPLETE CLEAN SLATE INITIALIZATION ===
    
    // 1. Set control signals to inactive state FIRST
    printf("[INIT] Setting control signals: RD=HIGH, WR=HIGH, CS=HIGH...\n");
    RD_HIGH();
    WR_HIGH();
    CS_HIGH();
    
    // 2. BACKLIGHT ON at startup (Pi GPIO for BL_EN)
    printf("[INIT] Backlight ON...\n");
    BACKLIGHT_ON();
    
    // 2. Hardware reset sequence: RST LOW -> wait 500ms -> RST HIGH
    printf("[INIT] Hardware reset SSD1963 (RST LOW 500ms)...\n");
    RESET_LOW();
    delay_ms(SSD1963_RESET_LOW_MS);
    RESET_HIGH();
    
    // CRITICAL: STM32 waits ~1.0s after RST release before any bus activity
    delay_ms(SSD1963_POST_RESET_MS);
    
    delay_ms(10);
    
    // 3. Clear screen for 2 seconds (black)
    printf("[INIT] Clearing screen for 2 seconds...\n");
    clear_screen(BLACK);
    delay_ms(2000);  // 2 seconds black screen
    
    // 4. Clear 74HC595 shift registers
    printf("[INIT] Clearing 74HC595 shift registers...\n");
    shift_out_16bits(0x0000);
    shift_out_16bits(0x0000);
    delay_ms(10);
    
    // 5. Soft reset (does not affect PLL registers 0xE0-0xE5)
    write_command(SSD1963_SOFT_RESET);
    delay_ms(20);   // datasheet requires >= 5ms

    // 6. Exit sleep mode (also triggers display on 0x29)
    write_command(SSD1963_EXIT_SLEEP_MODE);
    delay_ms(20);   // datasheet requires >= 5ms

    // 7. Re-clear shift registers after reset
    shift_out_16bits(0x0000);
    delay_ms(5);

    // --- PLL startup sequence (matching working reference) ---
    // Ref clk = 10MHz (crystal on SSD1963 modules)
    // M = 2 -> VCO = 10 * (45+1) = 460MHz (250-800MHz OK)
    // N = 45 -> PLL = 460 / (2+1) = 153MHz
    // Reference uses: N=45 (0x2D), M=2 (0x02), effectuate=0x04
    printf("[INIT] Configuring PLL (M=2, N=45, PLL=153MHz)...\n");
    write_command(SSD1963_SET_PLL_MN);
    write_data(45);   // N = 45 (0x2D)
    write_data(2);    // M = 2  (0x02)
    write_data(0x04); // C[2]=1: effectuate M and N

    write_command(SSD1963_SET_PLL);
    write_data(0x01); // Enable PLL
    delay_ms(100);    // Datasheet: 100ms para PLL lock

    write_command(SSD1963_SET_PLL);
    write_data(0x03); // Lock PLL (use PLL as sysclk)
    delay_ms(100);    // Datasheet: 100ms para PLL lock

    // Poll PLL status (0xE4) until bit 2 (PLL locked) is set
    for (uint8_t i = 0; i < 50; i++) {
        write_command(SSD1963_GET_PLL_STATUS);
        CS_LOW();
        RS_HIGH();
        write_data_bus(0);
        usleep(1);
        WR_LOW();
        usleep(1);
        WR_HIGH();
        usleep(1);
        uint8_t status = 0;
        for (int b = 0; b < 8; b++) {
            if (bcm2835_gpio_lev(SSD1963_LCD_D0 + b)) status |= (1 << b);
        }
        CS_HIGH();
        if (status & 0x04) break;  // PLL locked
        delay_ms(10);
    }

    // STM32: RS goes HIGH ~50ms before first real command
    RS_HIGH();
    delay_ms(SSD1963_RS_LEAD_MS);

    // --- LCD panel mode (18-bit panel for 480x272) ---
    write_command(SSD1963_SET_LCD_MODE);
    write_data(0x10);    // A5=0 -> 18-bit panel (probar 0x20 si falla)
    write_data(0x00);    // TFT mode
    write_data((LCD_WIDTH - 1) >> 8);
    write_data((LCD_WIDTH - 1) & 0xFF);
    write_data((LCD_HEIGHT - 1) >> 8);
    write_data((LCD_HEIGHT - 1) & 0xFF);
    write_data(0x00);    // RGB sequence

    // NO ENVIAR SET_PIXEL_FORMAT (0x3A es RESERVADO en SSD1963)
    // El formato se configura con SET_PIXEL_DATA_INTERFACE (0xF0)
    // Interfaz de datos del panel: 16-bit RGB565
    write_command(SSD1963_SET_PIXEL_DATA_INTERFACE);
    write_data(SSD1963_PDI_16BIT565);
    #endif

    // --- Pixel clock (LSHIFT frequency) ---
    // PCLK = PLL * (LCD_FPR + 1) / 2^20
    // Reference: PLL=150MHz, FPR=0x00FFBE → PCLK ≈ 9.3MHz
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
    // --- Horizontal timing: HT, HPS, HPW, LPS (todos -1) ---
    const uint16_t HT  = TFT_HSYNC_PERIOD;
    const uint16_t HPW = TFT_HSYNC_PULSE;
    const uint16_t HBP = TFT_HSYNC_BACK_PORCH;
    const uint16_t HFP = TFT_HSYNC_FRONT_PORCH;
    const uint16_t HPS = HPW + HBP;
    write_command(SSD1963_SET_HOR_PERIOD);
    write_data((HT - 1) >> 8);
    write_data((HT - 1) & 0xFF);
    write_data((HPS - 1) >> 8);
    write_data((HPS - 1) & 0xFF);
    write_data(HPW - 1);
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);

    // --- Vertical timing ---
    // VT = 272 + 12 + 8 + 1 = 293 (VT-1 = 292 = 0x0124)
    // VPS = 12 (back porch)
    // VPW = 1-1 = 0
    // FPS = 0
    printf("[INIT] Setting vertical timing (VT=%d, VPS=%d, VPW=%d)...\n",
           TFT_VSYNC_PERIOD, TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH, TFT_VSYNC_PULSE);
    // --- Vertical timing: VT, VSP, VPW, FPS ---
    const uint16_t VT  = TFT_VSYNC_PERIOD;
    const uint16_t VPW = TFT_VSYNC_PULSE;
    const uint16_t VBP = TFT_VSYNC_BACK_PORCH;
    const uint16_t VFP = TFT_VSYNC_FRONT_PORCH;
    const uint16_t VSP = VPW + VBP;
    write_command(SSD1963_SET_VER_PERIOD);
    write_data((VT - 1) >> 8);
    write_data((VT - 1) & 0xFF);
    write_data((VSP - 1) >> 8);
    write_data((VSP - 1) & 0xFF);
    write_data(VPW - 1);
    write_data(0x00);
    write_data(0x00);

    // --- Turn on display ---
    write_command(SSD1963_ON_DISPLAY);
    delay_ms(100);

    // Configure PWM for backlight (matching reference)
    // SET_PWM_FC (0xBE): PWMF=5 (~380Hz @ 150MHz), PWM=0xFE (max), C[3]=0/host, C[0]=1/enable
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

    printf("[INIT] SSD1963 initialization complete\n");
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
    // MSB FIRST: bit 15 -> bit 0 (worked for RED before)
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
    shift_out_16bits(data);
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
    // Frame-end re-sync
    shift_out_16bits(0x0000);
    shift_out_16bits(0x0000);
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
    // Area-end re-sync
    shift_out_16bits(0x0000);
    shift_out_16bits(0x0000);
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
    // Frame-end re-sync: clear shift register to prevent drift accumulation
    shift_out_16bits(0x0000);
    shift_out_16bits(0x0000);
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
