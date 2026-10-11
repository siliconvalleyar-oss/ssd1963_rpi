#include <unistd.h>
#include <iostream>
#include <cstring>
#include <config_hw.hpp>
#include <ssd1963_cmd.hpp>
#include <ssd1963.hpp>
#include <color.hpp>
#include <font_8x8.hpp>

// Retardo NOP para timing del bus 8080 (~43ns con 30 NOPs a 700MHz)
#define BUS_WAIT  do { __asm__ __volatile__( \
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n" \
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n" \
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop" );\
} while(0)

// IMPORTANTE: en ssd1963_cmd.hpp, SSD1963_SET_PWM_CONF vale 0x53 (NO es un
// comando del SSD1963). El comando correcto para el PWM es 0xBE = SSD1963_SET_PWM_FC.

SSD1963::SSD1963() {}

void SSD1963::delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

void SSD1963::setup_gpio() {
    // Estado inicial: se escriben los niveles ANTES de pasar los pines a salida,
    // para que no haya glitches en las lineas de control.
    //   RST = BAJO (el SSD1963 arranca en reset)
    //   RD, WR, CS, RS = ALTO (bus inactivo)
    CS_HIGH();
    WR_HIGH();
    RD_HIGH();
    RS_HIGH();
    RESET_LOW();
    BACKLIGHT_OFF();

    // Bus de datos D0-D15 (GPIO 12-27) como salida
    for (uint8_t pin = SSD1963_LCD_D0; pin <= SSD1963_LCD_D15; pin++) {
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
    }
    // Señales de control
    bcm2835_gpio_fsel(SSD1963_LCD_RS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_WR, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_RD, BCM2835_GPIO_FSEL_OUTP);   // RD siempre en alto
    bcm2835_gpio_fsel(SSD1963_LCD_CS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_RESET, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_BACKLIGHT, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_PW1, BCM2835_GPIO_FSEL_INPT);  // pin PWM del SSD1963 (diagnostico)

    // Reasegurar niveles ya con los pines como salida (RST sigue en BAJO)
    CS_HIGH();
    WR_HIGH();
    RD_HIGH();
    RS_HIGH();
    RESET_LOW();
    BACKLIGHT_OFF();
}

void SSD1963::init() {
    // 1. Reset de hardware: RST en BAJO durante SSD1963_RESET_HOLD_MS (2 s),
    //    con RD, WR, CS y RS en ALTO. Luego RST en alto y recien ahi se inicializa.
    //    (Con RST en bajo el chip ignora todo lo que se le escriba, por eso la
    //     limpieza de la pantalla en negro se hace al final de init(), antes del backlight.)
    printf("[INIT] RST en bajo durante %u ms...\n", (unsigned)SSD1963_RESET_HOLD_MS);
    CS_HIGH();
    WR_HIGH();
    RD_HIGH();
    RS_HIGH();
    RESET_LOW();
    delay_ms(SSD1963_RESET_HOLD_MS);
    RESET_HIGH();
    delay_ms(120);   // esperar el POR interno

    // 2. Soft reset (no afecta a los registros del PLL 0xE0-0xE5)
    write_command(SSD1963_SOFT_RESET);
    delay_ms(20);    // el datasheet pide >= 5 ms

    // 3. Salir de sleep
    write_command(SSD1963_EXIT_SLEEP_MODE);
    delay_ms(20);

    // 4. PLL. Cristal de 10 MHz:
    //    VCO = 10 * (M+1) = 300 MHz, PLL = VCO / (N+1) = 100 MHz (maximo permitido: 110 MHz)
    printf("[INIT] PLL (M=%u, N=%u -> 100MHz con cristal de 10MHz)\n",
           (unsigned)SSD1963_PLL_M, (unsigned)SSD1963_PLL_N);
    write_command(SSD1963_SET_PLL_MN);
    write_data(SSD1963_PLL_M);
    write_data(SSD1963_PLL_N);
    write_data(0x04);            // C[2]=1: efectuar M y N

    write_command(SSD1963_SET_PLL);
    write_data(0x01);            // habilitar PLL
    delay_ms(100);               // esperar a que enganche (el bus es solo escritura, no se puede leer 0xE4)

    write_command(SSD1963_SET_PLL);
    write_data(0x03);            // usar el PLL como reloj del sistema
    delay_ms(20);

    // 5. Modo del panel (ancho y polaridades definidos en config_hw.hpp)
    write_command(SSD1963_SET_LCD_MODE);
    write_data(LCD_MODE_CONFIG);
    write_data(0x00);            // TFT, modo Hsync+Vsync
    write_data((LCD_WIDTH - 1) >> 8);
    write_data((LCD_WIDTH - 1) & 0xFF);
    write_data((LCD_HEIGHT - 1) >> 8);
    write_data((LCD_HEIGHT - 1) & 0xFF);
    write_data(0x00);            // secuencia RGB

    // 6. Formato de datos del bus de la Pi: 16-bit RGB565
    write_command(SSD1963_SET_PIXEL_DATA_INTERFACE);
    write_data(SSD1963_PDI_16BIT565);

    // 6b. Address mode (0x36), valor en config_hw.hpp (LCD_ADDR_MODE).
    //     El POR ya es 0x00 y el soft reset lo restaura, asi que esto solo lo deja explicito.
    //     0x00 = RGB, de arriba a abajo, izq->der, sin flips | 0x08 = BGR
    //     0x02 = flip horizontal | 0x01 = flip vertical
    write_command(SSD1963_SET_ADDR_MODE);
    write_data(LCD_ADDR_MODE);

    // 7. Pixel clock: PCLK = PLL * (FPR+1) / 2^20 = 9.00 MHz con PLL de 100 MHz
    write_command(SSD1963_SET_LSHIFT_FREQ);
    write_data((LCD_FPR >> 16) & 0xFF);
    write_data((LCD_FPR >> 8) & 0xFF);
    write_data(LCD_FPR & 0xFF);

    // 8. Timing horizontal. Segun el datasheet los registros guardan (valor - 1):
    //    HT = periodo total - 1, HPW = ancho del pulso - 1.
    //    HPS = pulso + back porch (inicio del sync hasta el primer dato).
    const uint16_t ht  = TFT_HSYNC_PERIOD - 1;
    const uint16_t hps = TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH;
    const uint16_t hpw = TFT_HSYNC_PULSE - 1;
    write_command(SSD1963_SET_HOR_PERIOD);
    write_data((ht >> 8) & 0xFF);
    write_data(ht & 0xFF);
    write_data((hps >> 8) & 0xFF);
    write_data(hps & 0xFF);
    write_data(hpw);
    write_data(0x00);            // LPS high
    write_data(0x00);            // LPS low
    write_data(0x00);            // LPSPP

    // 9. Timing vertical (mismo criterio: valor - 1)
    const uint16_t vt  = TFT_VSYNC_PERIOD - 1;
    const uint16_t vps = TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH;
    const uint16_t vpw = TFT_VSYNC_PULSE - 1;
    write_command(SSD1963_SET_VER_PERIOD);
    write_data((vt >> 8) & 0xFF);
    write_data(vt & 0xFF);
    write_data((vps >> 8) & 0xFF);
    write_data(vps & 0xFF);
    write_data(vpw);
    write_data(0x00);            // FPS high
    write_data(0x00);            // FPS low
    printf("[INIT] HT=%u HPS=%u HPW=%u | VT=%u VPS=%u VPW=%u\n", ht, hps, hpw, vt, vps, vpw);

    // 10. Encender display
    write_command(SSD1963_ON_DISPLAY);
    delay_ms(100);

    // 11. GPIO0-3 del SSD1963 como salidas controladas por el host.
    //     Param 2 B[0]=1: GPIO0 es un GPIO normal (no controla el power del panel con sleep).
    write_command(SSD1963_SET_GPIO_CONF);
    write_data(0x0F);
    write_data(0x01);

    // 12. Backlight por PWM del SSD1963 (comando 0xBE, NO SSD1963_SET_PWM_CONF = 0x53)
    //     P1: PWMF -> frecuencia = PLL / (256 * PWMF) / 256  (PWMF=14 -> ~109 Hz con PLL de 100MHz)
    //     P2: duty = PWM/256 (0xFF = casi maximo, 0x00 = siempre apagado)
    //     P3: C[3]=0 controlado por el host, C[0]=1 PWM habilitado
    //     P4-P6: parametros de DBC (no se usan)
    write_command(SSD1963_SET_PWM_FC);
    write_data(0x0E);
    write_data(0xFF);
    write_data(0x01);
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);

    // 13. GPIO0 del SSD1963 en alto (habilita el panel/driver en muchos modulos)
    write_command(SSD1963_SET_GPIO_VALUE);
    write_data(0x01);

    // 14. Limpiar la pantalla a negro ANTES de encender el backlight
    //     (asi no se ve el contenido basura que tenga la GRAM tras el arranque)
    clear_screen(BLACK);

    // 15. Enable de backlight desde la Pi (BL_EN, GPIO fuera del bus de datos)
    BACKLIGHT_ON();

    printf("[INIT] SSD1963 initialization complete\n");

#ifndef SSD1963_NO_PWM_DEBUG
    // Diagnostico: muestrea el pin PWM del SSD1963 (cableado al GPIO PW1).
    //   ~0 o 10000 altos  -> el PWM no se esta generando (revisar PLL/bus/comando 0xBE)
    //   ~9000-9800 altos  -> el SSD1963 genera el PWM (duty ~94-99%)
    {
        delay_ms(20);
        int highs = 0;
        const int samples = 10000;
        for (int i = 0; i < samples; i++) {
            highs += PW1_READ() ? 1 : 0;
            bcm2835_delayMicroseconds(20);
        }
        printf("[DEBUG] PWM (PW1) en alto: %d/%d muestras\n", highs, samples);
    }
#endif
}

static inline uint32_t data_to_mask(uint16_t data) {
    return ((uint32_t)data) << 12;   // D0=pin12 … D15=pin27
}

void SSD1963::write_data_bus(uint16_t data) {
    uint32_t mask = data_to_mask(data);
    bcm2835_gpio_clr_multi(DATA_PINS_MASK);
    bcm2835_gpio_set_multi(mask);
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
    const uint16_t ROWS_PER_BATCH = 32;
    for (uint16_t y = 0; y < LCD_HEIGHT; y += ROWS_PER_BATCH) {
        uint16_t h = ROWS_PER_BATCH;
        if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
        set_area(0, y, LCD_WIDTH - 1, y + h - 1);
        write_command(SSD1963_WRITE_MEMORY_START);
        fill_burst(this, (uint32_t)LCD_WIDTH * h, color);
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
