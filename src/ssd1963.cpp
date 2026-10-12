// ssd1963.cpp

#include <unistd.h>
#include <iostream>
#include <time.h>
#include <config_hw.hpp>
#include <ssd1963_cmd.hpp>
#include <ssd1963.hpp>
#include <color.hpp>
#include <font_8x8.hpp>

// Helper para delays en nanosegundos (para SSD1963_MCU_WAVEFORM)
static inline void ns_delay(long ns) {
    struct timespec ts = {0, ns};
    clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, nullptr);
}


SSD1963::SSD1963() : last_op_type_(0) {}

void SSD1963::delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

void SSD1963::setup_gpio() {
    // Configurar pines de datos como salida
    for (uint8_t pin = SSD1963_LCD_D0; pin <= SSD1963_LCD_D15; pin++) {
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
    }
    // Configurar pines de control como salida
    bcm2835_gpio_fsel(SSD1963_LCD_RS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_WR, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_CS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_RESET, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_BACKLIGHT, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(SSD1963_LCD_RD, BCM2835_GPIO_FSEL_OUTP);
    // Estados iniciales: RST arranca en BAJO, RD/WR/CS1 en ALTO
    // (el modulo hace su power-up mientras RST esta bajo; ~2 s despues
    //  se libera y recien ahi comienza nuestra 2a inicializacion)
    CS_HIGH();
    WR_HIGH();
    RS_HIGH();
    RESET_LOW();
    bcm2835_gpio_write(SSD1963_LCD_RD, HIGH);
    BACKLIGHT_OFF();
}

void  SSD1963::write_command(uint8_t cmd) {
#if SSD1963_MCU_WAVEFORM
    // === MCU WAVEFORM: COMANDO ===
    // 1. RS baja 40 ns ANTES de CS (pre-pulso)
    RS_LOW();
    ns_delay(40);

    // 2. CS baja
    CS_LOW();
    ns_delay(30);

    // 3. RS sube 30ns y baja junto con CS (pulso extra)
    RS_HIGH();
    ns_delay(30);
    RS_LOW();

    // 4. Datos en bus + WR pulso
    write_data_bus(cmd);
    WR_LOW();
    ns_delay(65);  // WR en bajo ~65 ns

    // 5. WR sube 10 ns ANTES de CS
    WR_HIGH();
    ns_delay(10);

    // 6. CS sube
    CS_HIGH();

    // 7. RS se queda LOW 95 ns DESPUES de CS↑, luego sube
    ns_delay(95);
    RS_HIGH();

    if (slow_bus_) usleep(1);
#else
    RS_LOW();
    CS_LOW();
    write_data_bus(cmd);
    WR_LOW();
    WR_HIGH();
    CS_HIGH();
    if (slow_bus_) usleep(1);
#endif
}

void SSD1963::write_data(uint16_t data) {
#if SSD1963_MCU_WAVEFORM
    // Determinar si es primer dato tras comando
    bool is_first_data = (last_op_type_ == 0);
    last_op_type_ = 1;

    if (is_first_data) {
        // === PRIMER DATO TRAS COMANDO ===
        // RS pulsa: HIGH 40ns -> LOW 30ns -> HIGH (con CS↓)
        RS_HIGH();
        ns_delay(40);
        RS_LOW();
        ns_delay(30);
    } else {
        // === RESTO DE DATOS ===
        // RS se queda HIGH
        RS_HIGH();
    }

    CS_LOW();

    if (is_first_data) {
        // RS sube con CS↓
        ns_delay(30);
        RS_HIGH();
    }

    write_data_bus(data);
    WR_LOW();
    ns_delay(65);
    WR_HIGH();
    ns_delay(10);
    CS_HIGH();

    if (is_first_data) {
        // RS se queda HIGH
    }

    if (slow_bus_) usleep(1);
#else
    RS_HIGH();
    CS_LOW();
    write_data_bus(data);
    WR_LOW();
    WR_HIGH();
    CS_HIGH();
    if (slow_bus_) usleep(1);
#endif
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
    // Interfaz 16-bit RGB565 (PDI 0x03): CADA escritura al bus es UN píxel completo.
    // Antes se enviaba color + color>>8 (2 escrituras por píxel): desplazaba/duplicaba
    // los datos -> rayas y "basura de bits" en pantalla.
    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    for (uint32_t i = 0; i < (uint32_t)LCD_WIDTH * LCD_HEIGHT; i++) {
        write_data(color);
    }
}


void SSD1963::init() {

    // Durante toda la inicializacion el bus va lento (~1 us por strobe),
    // como el PMP lento (WAITB=3, WAITM=15, WAITE=3) del PIC32 de referencia.
    slow_bus_ = true;

    // 1. Reset físico del SSD1963.
    // El módulo arranca con RST en BAJO (RD/WR/CS en alto) y ~2 s despues se libera
    // a alto; mantenerlo bajo ese tiempo antes de inicializar.
    RESET_LOW();
    delay_ms(2000);
    RESET_HIGH();
    delay_ms(10);
    // 2. Soft Reset (ANTES del PLL)
    write_command(SSD1963_SOFT_RESET);
    delay_ms(10);
    // 3. Configurar PLL (para REFclk = 10 MHz → PLLclk = 500MHz, SYSclk = 100MHz)
    write_command(SSD1963_SET_PLL_MN);
    write_data(49);     // PLLclk = REFclk * 50
    write_data(4);      // SYSclk = PLLclk / 5
    write_data(4);      // Dummy (según código STM32)
    // Activar PLL
    write_command(SSD1963_SET_PLL);
    write_data(0x01);
    delay_ms(10);  // largo retardo
    write_command(SSD1963_SET_PLL);
    write_data(0x03);
    delay_ms(10);
    // 6. Frecuencia LSHIFT (SE ENVIA ANTES DEL MODO LCD, como en el PIC32 de referencia)
    write_command(SSD1963_SET_LSHIFT_FREQ);
    write_data((LCD_FPR >> 16) & 0xFF);
    write_data((LCD_FPR >> 8) & 0xFF);
    write_data(LCD_FPR & 0xFF);
    // 7. Configurar modo LCD
    write_command(SSD1963_SET_LCD_MODE);
    write_data(0x10);    // LCD_MODE_CONFIG: mismo valor que la referencia PIC32
    write_data(0x00);    // igual que TechToys (byte 2 = 0x00)
    write_data((LCD_WIDTH - 1) >> 8);
    write_data((LCD_WIDTH - 1) & 0xFF);
    write_data((LCD_HEIGHT - 1) >> 8);
    write_data((LCD_HEIGHT - 1) & 0xFF);
    write_data(0x00);    // RGB
    // Tiempos horizontales (frmulas del PIC32 en la referencia: HT-1, HPS-1, HPW-1)
    const uint16_t HT  = TFT_HSYNC_PERIOD;
    const uint16_t HPS = TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH;
    write_command(SSD1963_SET_HOR_PERIOD);
    write_data(((HT - 1) >> 8) & 0xFF);
    write_data((HT - 1) & 0xFF);
    write_data(((HPS - 1) >> 8) & 0xFF);
    write_data((HPS - 1) & 0xFF);
    write_data(TFT_HSYNC_PULSE - 1);
    write_data(0x00); // LPS = 0
    write_data(0x00); // Opcional
    write_data(0x00); // Opcional
    // Tiempos verticales (VT-1, VSP-1, VPW-1)
    const uint16_t VT  = TFT_VSYNC_PERIOD;
    const uint16_t VSP = TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH;
    write_command(SSD1963_SET_VER_PERIOD);
    write_data(((VT - 1) >> 8) & 0xFF);
    write_data((VT - 1) & 0xFF);
    write_data(((VSP - 1) >> 8) & 0xFF);
    write_data((VSP - 1) & 0xFF);
    write_data(TFT_VSYNC_PULSE - 1);
    write_data(0x00);
    write_data(0x00);
    // Formato de píxel: 16bpp (0x55), igual que la referencia PIC32
    write_command(SSD1963_SET_PIXEL_FORMAT);
    write_data(0x55);
    // Interfaz de datos del panel: 16-bit RGB565
    write_command(SSD1963_SET_PIXEL_DATA_INTERFACE);
    write_data(SSD1963_PDI_16BIT565);
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
    set_area(x, y, x + width - 1, y + height - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    for (uint32_t i = 0; i < width * height; i++) {
        write_data(color);
    }
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

    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    write_command(SSD1963_WRITE_MEMORY_START);

    for (uint32_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        uint8_t high, low;
        if (fread(&high, 1, 1, file) != 1) break;
        if (fread(&low, 1, 1, file) != 1) break;

        uint16_t color = (high << 8) | low;
        write_data(color);
    }

    fclose(file);
}

void SSD1963::write_data_bus(uint16_t data) {
    // Limpia todos los pines de datos
    for (uint8_t pin = SSD1963_LCD_D0; pin <= SSD1963_LCD_D15; pin++) {
        bcm2835_gpio_write(pin, LOW);
    }
    // Escribe los bits correspondientes a HIGH
    for (uint8_t i = 0; i < 16; i++) {
        if (data & (1 << i)) {
            bcm2835_gpio_write(SSD1963_LCD_D0 + i, HIGH);
        }
    }
}

void SSD1963::write_pixel_burst_start() {
    RS_HIGH();
    CS_LOW();
}

void SSD1963::write_pixel_burst(uint16_t data) {
    write_data_bus(data);
    WR_LOW();
    WR_HIGH();
    if (slow_bus_) usleep(1);
}

void SSD1963::write_pixel_burst_end() {
    CS_HIGH();
}

void SSD1963::fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    draw_block(x, y, width, height, color);
}

void SSD1963::draw_pixels(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                          const uint16_t* data) {
    set_area(x, y, x + w - 1, y + h - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    write_pixel_burst_start();
    for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
        write_pixel_burst(data[i]);
    }
    write_pixel_burst_end();
}

void SSD1963::draw_buffer(const uint16_t* data) {
    set_area(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    write_command(SSD1963_WRITE_MEMORY_START);
    write_pixel_burst_start();
    for (uint32_t i = 0; i < (uint32_t)LCD_WIDTH * LCD_HEIGHT; i++) {
        write_pixel_burst(data[i]);
    }
    write_pixel_burst_end();
}