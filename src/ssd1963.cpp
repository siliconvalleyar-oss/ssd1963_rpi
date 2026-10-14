// ssd1963.cpp

#include <unistd.h>
#include <iostream>
#include <vector>
#include <config_hw.hpp>
#include <ssd1963_cmd.hpp>
#include <ssd1963.hpp>
#include <color.hpp>
#include <gpio_mmap.hpp>


SSD1963::SSD1963() {}

void SSD1963::delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

void SSD1963::setup_gpio() {
    // Configurar pines de datos como salida
    for (uint8_t pin = SSD1963_LCD_D0; pin <= SSD1963_LCD_D15; pin++) {
        GPIO_FSEL(pin, GPIO_FSEL_OUT);
    }
    // Configurar pines de control como salida
    GPIO_FSEL(SSD1963_LCD_RS, GPIO_FSEL_OUT);
    GPIO_FSEL(SSD1963_LCD_WR, GPIO_FSEL_OUT);
    GPIO_FSEL(SSD1963_LCD_CS, GPIO_FSEL_OUT);
    GPIO_FSEL(SSD1963_LCD_RESET, GPIO_FSEL_OUT);
    GPIO_FSEL(SSD1963_LCD_BACKLIGHT, GPIO_FSEL_OUT);
    GPIO_FSEL(SSD1963_LCD_RD, GPIO_FSEL_OUT);
    // Estados iniciales: RST en BAJO, RD/WR/CS en ALTO, RS en BAJO (command mode por defecto)
    CS_HIGH();
    WR_HIGH();
    RS_LOW();       // RS en BAJO al inicio (modo comando)
    RESET_LOW();
    GPIO_SET(SSD1963_LCD_RD);
    BACKLIGHT_OFF();
}

void SSD1963::write_command(uint8_t cmd) {
    RS_LOW();
    DELAY_US(1);
    write_data_bus(cmd);
    DELAY_US(1);
    CS_LOW();
    DELAY_US(1);
    WR_LOW();
    DELAY_US(1);
    WR_HIGH();
    DELAY_US(1);
    CS_HIGH();
    DELAY_US(1);
    RS_HIGH();  // Force RS HIGH after command (STM32: RS hold ~95ns after CS↑)
    DELAY_US(1);
}

void SSD1963::write_data(uint16_t data) {
    RS_HIGH();
    DELAY_US(1);
    write_data_bus(data);
    DELAY_US(1);
    CS_LOW();
    DELAY_US(1);
    WR_LOW();
    DELAY_US(1);
    WR_HIGH();
    DELAY_US(1);
    CS_HIGH();
    DELAY_US(1);
    // RS stays HIGH for data bursts
}

void SSD1963::write_data_burst(const uint16_t* data, uint32_t count) {
    // Fast bulk write: CS stays LOW, RS stays HIGH, only WR pulses
    RS_HIGH();
    CS_LOW();
    for (uint32_t i = 0; i < count; i++) {
        write_data_bus(data[i]);
        WR_LOW();
        WR_HIGH();
    }
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



void SSD1963::clear_screen(uint16_t color) {
    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    // Fast fill: reuse single pixel value
    RS_HIGH();
    CS_LOW();
    write_data_bus(color);
    for (uint32_t i = 0; i < (uint32_t)LCD_WIDTH * LCD_HEIGHT; i++) {
        WR_LOW();
        WR_HIGH();
    }
    CS_HIGH();
}

void SSD1963::draw_buffer(const uint16_t* buffer) {
    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    write_data_burst(buffer, (uint32_t)LCD_WIDTH * LCD_HEIGHT);
}

void SSD1963::fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + width > LCD_WIDTH) width = LCD_WIDTH - x;
    if (y + height > LCD_HEIGHT) height = LCD_HEIGHT - y;
    set_area(x, y, x + width - 1, y + height - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    // Fast fill
    RS_HIGH();
    CS_LOW();
    write_data_bus(color);
    for (uint32_t i = 0; i < (uint32_t)width * height; i++) {
        WR_LOW();
        WR_HIGH();
    }
    CS_HIGH();
}

void SSD1963::draw_pixels(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* pixels) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + width > LCD_WIDTH) width = LCD_WIDTH - x;
    if (y + height > LCD_HEIGHT) height = LCD_HEIGHT - y;
    set_area(x, y, x + width - 1, y + height - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    write_data_burst(pixels, (uint32_t)width * height);
}
    write_command(SSD1963_WRITE_MEMORY_START);
    for (uint32_t i = 0; i < (uint32_t)width * height; i++) {
        write_data(pixels[i]);
    }
}


void SSD1963::init() {

    // Durante toda la inicializacion el bus va lento (~1 us por strobe),
    // como el PMP lento (WAITB=3, WAITM=15, WAITE=3) del PIC32 de referencia.
    slow_bus_ = true;

    // 1. Reset físico del SSD1963.
    // setup_gpio() ya deja RST=LOW, RS=LOW, RD/WR/CS=HIGH.
    // El módulo hace power-up mientras RST está bajo; esperar 2 s.
    delay_ms(SSD1963_RESET_LOW_MS);
    RESET_HIGH();
    
    // CRITICAL: STM32 waits ~1.0s after RST release before any bus activity
    delay_ms(SSD1963_POST_RESET_MS);
    
    delay_ms(10);
    // 2. Soft Reset (ANTES del PLL)
    write_command(SSD1963_SOFT_RESET);
    delay_ms(5);
    // 3. EXIT SLEEP MODE (0x11) - CRITICO: activa el panel y dispara SET_DISPLAY_ON automaticamente
    write_command(SSD1963_EXIT_SLEEP_MODE);
    delay_ms(5);
    // 4. Configurar PLL (para REFclk = 10 MHz → PLLclk = 500MHz, SYSclk = 100MHz)
    write_command(SSD1963_SET_PLL_MN);
    write_data(49);     // PLLclk = REFclk * 50
    write_data(4);      // SYSclk = PLLclk / 5
    write_data(4);      // Dummy (según código STM32)
    // Activar PLL - DATASHEET: esperar 100ms + polling PLL lock
    write_command(SSD1963_SET_PLL);
    write_data(0x01);
    delay_ms(100);
    write_command(SSD1963_SET_PLL);
    write_data(0x03);
    delay_ms(100);

    // Poll PLL status (0xE4) until bit 2 (PLL locked) is set
    for (uint8_t i = 0; i < 50; i++) {
        write_command(SSD1963_GET_PLL_STATUS);
        CS_LOW();
        RS_HIGH();
        write_data_bus(0);
        DELAY_US(1);
        WR_LOW();
        DELAY_US(1);
        WR_HIGH();
        DELAY_US(1);
        uint8_t status = 0;
        for (uint8_t b = 0; b < 8; b++) {
            if (GPIO_LEV(SSD1963_LCD_D0 + b)) status |= (1 << b);
        }
        CS_HIGH();
        if (status & 0x04) break;  // PLL locked
        delay_ms(10);
    }

    // STM32: RS goes HIGH ~50ms before first real command
    RS_HIGH();
    delay_ms(SSD1963_RS_LEAD_MS);
    // 5. Frecuencia LSHIFT (SE ENVIA ANTES DEL MODO LCD, como en el PIC32 de referencia)
    write_command(SSD1963_SET_LSHIFT_FREQ);
    write_data((LCD_FPR >> 16) & 0xFF);
    write_data((LCD_FPR >> 8) & 0xFF);
    write_data(LCD_FPR & 0xFF);
    // 6. Configurar modo LCD - 0x10 para panel 18-bit (común en 480x272)
    write_command(SSD1963_SET_LCD_MODE);
    write_data(0x10);    // A5=0 -> 18-bit panel (probar 0x20 si falla)
    write_data(0x00);    // TFT mode
    write_data((LCD_WIDTH - 1) >> 8);
    write_data((LCD_WIDTH - 1) & 0xFF);
    write_data((LCD_HEIGHT - 1) >> 8);
    write_data((LCD_HEIGHT - 1) & 0xFF);
    write_data(0x00);    // RGB
    // Tiempos horizontales: HT, HPS, HPW, LPS (todos -1)
    const uint16_t HT  = TFT_HSYNC_PERIOD;           // 525
    const uint16_t HPW = TFT_HSYNC_PULSE;            // 41
    const uint16_t HBP = TFT_HSYNC_BACK_PORCH;       // 2
    const uint16_t HFP = TFT_HSYNC_FRONT_PORCH;      // 2
    const uint16_t HPS = HPW + HBP;                  // 43
    write_command(SSD1963_SET_HOR_PERIOD);
    write_data((HT - 1) >> 8);   // 524
    write_data((HT - 1) & 0xFF);
    write_data((HPS - 1) >> 8);  // 42
    write_data((HPS - 1) & 0xFF);
    write_data(HPW - 1);         // 40
    write_data(0x00);            // LPS = 0
    write_data(0x00);
    write_data(0x00);
    // Tiempos verticales: VT, VSP, VPW, FPS
    const uint16_t VT  = TFT_VSYNC_PERIOD;           // 286
    const uint16_t VPW = TFT_VSYNC_PULSE;            // 10
    const uint16_t VBP = TFT_VSYNC_BACK_PORCH;       // 2
    const uint16_t VFP = TFT_VSYNC_FRONT_PORCH;      // 2
    const uint16_t VSP = VPW + VBP;                  // 12
    write_command(SSD1963_SET_VER_PERIOD);
    write_data((VT - 1) >> 8);   // 285
    write_data((VT - 1) & 0xFF);
    write_data((VSP - 1) >> 8);  // 11
    write_data((VSP - 1) & 0xFF);
    write_data(VPW - 1);         // 9
    write_data(0x00);            // FPS = 0
    write_data(0x00);
    // NO ENVIAR SET_PIXEL_FORMAT (0x3A es RESERVADO en SSD1963)
    // El formato se configura con SET_PIXEL_DATA_INTERFACE (0xF0)
    // Interfaz de datos del panel: 16-bit RGB565
    write_command(SSD1963_SET_PIXEL_DATA_INTERFACE);
    write_data(SSD1963_PDI_16BIT565);
    // Address mode: RGB, top-to-bottom, left-to-right
    write_command(SSD1963_SET_ADDR_MODE);
    write_data(0x00);
    // Reset del PANEL fisico via GPIO0 (igual que la referencia PIC32):
    // pulso LOW -> 1ms -> HIGH. Sin esto el panel puede quedar en estado indefinido.
    write_command(SSD1963_SET_GPIO_CONF);
    write_data(0x0F);   // GPIO[3:0] como salidas
    write_data(0x01);   // GPIOs normales, controlados por el host
    write_command(SSD1963_SET_GPIO_VALUE);
    write_data(0x01);   // alto
    write_command(SSD1963_SET_GPIO_VALUE);
    write_data(0x00);   // bajo (reset assertado)
    delay_ms(1);
    write_command(SSD1963_SET_GPIO_VALUE);
    write_data(0x01);   // alto (libera reset)

    // Pantalla en BLANCO (0x28) hasta tener el GRAM limpio, luego ON (0x29)
    write_command(SSD1963_BLANK_DISPLAY);
    write_command(SSD1963_ON_DISPLAY);
    delay_ms(50);

    // Fin de init: el bus vuelve a velocidad normal para dibujar
    slow_bus_ = false;
    // Encender luz de fondo (GPIO)
    BACKLIGHT_ON();
}

 

void SSD1963::draw_block(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (x + width > LCD_WIDTH) width = LCD_WIDTH - x;
    if (y + height > LCD_HEIGHT) height = LCD_HEIGHT - y;
    set_area(x, y, x + width - 1, y + height - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    // Fast fill
    RS_HIGH();
    CS_LOW();
    write_data_bus(color);
    for (uint32_t i = 0; i < (uint32_t)width * height; i++) {
        WR_LOW();
        WR_HIGH();
    }
    CS_HIGH();
}

void SSD1963::draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    set_area(x, y, x, y);  // Define el área como 1x1 píxel
    write_command(SSD1963_WRITE_MEMORY_START);
    write_data(color);     // Envía el color (pixel)
}


void SSD1963::draw_image_rgb565(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        std::cerr << "No se pudo abrir la imagen: " << filepath << std::endl;
        return;
    }

    // Check for 4-byte header (width, height as uint16_t big-endian)
    uint8_t hdr[4];
    size_t hdr_read = fread(hdr, 1, 4, file);
    uint32_t img_w = LCD_WIDTH, img_h = LCD_HEIGHT;
    bool has_header = false;
    if (hdr_read == 4) {
        uint16_t w = (hdr[0] << 8) | hdr[1];
        uint16_t h = (hdr[2] << 8) | hdr[3];
        if (w <= LCD_WIDTH && h <= LCD_HEIGHT) {
            img_w = w;
            img_h = h;
            has_header = true;
            std::cout << "[IMG] Header detected: " << img_w << "x" << img_h << std::endl;
        } else {
            // Not a valid header, rewind
            fseek(file, 0, SEEK_SET);
        }
    } else {
        fseek(file, 0, SEEK_SET);
    }

    set_area(0, 0, img_w - 1, img_h - 1);
    write_command(SSD1963_WRITE_MEMORY_START);

    uint32_t total_pixels = img_w * img_h;
    std::vector<uint16_t> buffer(total_pixels);
    for (uint32_t i = 0; i < total_pixels; i++) {
        uint8_t high, low;
        if (fread(&high, 1, 1, file) != 1) break;
        if (fread(&low, 1, 1, file) != 1) break;
        buffer[i] = (high << 8) | low;
        if ((i % 32768) == 0) {
            std::cout << "[IMG] Progress: " << (i * 100 / total_pixels) << "%" << std::endl;
        }
    }
    fclose(file);
    
    // Fast burst write
    write_data_burst(buffer.data(), total_pixels);
    std::cout << "[IMG] Done" << std::endl;
}

void SSD1963::write_data_bus(uint16_t data) {
    // Escribe todos los 16 bits directamente (sin glitches intermedios)
    // D0 = bit 0, D1 = bit 1, ..., D15 = bit 15
    for (uint8_t i = 0; i < 16; i++) {
        if ((data >> i) & 1) {
            GPIO_SET(SSD1963_LCD_D0 + i);
        } else {
            GPIO_CLR(SSD1963_LCD_D0 + i);
        }
    }
}

SSD1963::~SSD1963() {
    // GPIO cleanup handled by gpio_mmap destructor
}

