/**
 * @file    ssd1963.hpp
 * @brief   Driver para el controlador SSD1963 TFT LCD en Raspberry Pi.
 * @details Proporciona una interfaz completa para inicializar y dibujar
 *          en displays SSD1963 usando GPIO bit-banging (bus paralelo 8080)
 *          a través de la librería libbcm2835.
 *
 * @note    Soporta resolución 480x272 en formato RGB565 a 16-bit.
 * @note    Requiere libbcm2835 instalada (http://www.airspayce.com/mikem/bcm2835/)
 */
#pragma once

#include <bcm2835.h>
#include <cstdint>
#include <cstdio>

// =========================================================================
// Macros de control GPIO
// =========================================================================
#define RS_LOW()        bcm2835_gpio_write(SSD1963_LCD_RS, LOW)
#define RS_HIGH()       bcm2835_gpio_write(SSD1963_LCD_RS, HIGH)
#define WR_LOW()        bcm2835_gpio_write(SSD1963_LCD_WR, LOW)
#define WR_HIGH()       bcm2835_gpio_write(SSD1963_LCD_WR, HIGH)
#define CS_LOW()        bcm2835_gpio_write(SSD1963_LCD_CS, LOW)
#define CS_HIGH()       bcm2835_gpio_write(SSD1963_LCD_CS, HIGH)
#define RESET_LOW()     bcm2835_gpio_write(SSD1963_LCD_RESET, LOW)
#define RESET_HIGH()    bcm2835_gpio_write(SSD1963_LCD_RESET, HIGH)
#define BACKLIGHT_ON()  bcm2835_gpio_write(SSD1963_LCD_BACKLIGHT, HIGH)
#define BACKLIGHT_OFF() bcm2835_gpio_write(SSD1963_LCD_BACKLIGHT, LOW)

// Máscara para todos los pines de datos D0-D15 (12..27)
#define DATA_PINS_MASK  (uint32_t)( \
    (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15) | \
    (1 << 16) | (1 << 17) | (1 << 18) | (1 << 19) | \
    (1 << 20) | (1 << 21) | (1 << 22) | (1 << 23) | \
    (1 << 24) | (1 << 25) | (1 << 26) | (1 << 27) )

/*! \def FONT_CHAR_WIDTH
 *  \brief Ancho en píxeles de cada carácter bitmap. */
/*! \def FONT_CHAR_HEIGHT
 *  \brief Alto en píxeles de cada carácter bitmap. */
#define FONT_CHAR_WIDTH     8
#define FONT_CHAR_HEIGHT    8

/**
 * @class   SSD1963
 * @brief   Controlador de alto nivel para el display SSD1963.
 *
 * Proporciona métodos para inicializar el display, dibujar formas
 * geométricas, renderizar texto con fuente bitmap y mostrar imágenes
 * desde archivos RGB565 raw.
 *
 * ## Uso básico
 * @code
 *     SSD1963 display;
 *     bcm2835_init();
 *     display.setup_gpio();
 *     display.init();
 *     display.clear_screen(BLACK);
 *     display.draw_string(10, 10, "Hola Mundo", WHITE, BLACK);
 * @endcode
 */
class SSD1963 {
public:
    SSD1963();

    // ---------------------------------------------------------------
    // Inicialización y configuración
    // ---------------------------------------------------------------

    /** @brief Configura todos los pines GPIO como salida. */
    void setup_gpio();

    /** @brief Secuencia completa de inicialización del SSD1963.
     *
     *  Incluye: reset físico, soft reset, configuración PLL,
     *  modo LCD, temporización horizontal/vertical, y encendido. */
    void init();

    /** @brief Delay de bloqueo en milisegundos.
     *  @param ms Tiempo de espera. */
    void delay_ms(uint32_t ms);

    // ---------------------------------------------------------------
    // Operaciones de dibujo
    // ---------------------------------------------------------------

    /** @brief Limpia toda la pantalla con un color sólido.
     *  @param color Color RGB565. */
    void clear_screen(uint16_t color);

    /** @brief Dibuja un píxel individual.
     *  @param x Coordenada X.
     *  @param y Coordenada Y.
     *  @param color Color RGB565. */
    void draw_pixel(uint16_t x, uint16_t y, uint16_t color);

    /** @brief Rellena un área rectangular con un color sólido.
     *  @param x      X inicial.
     *  @param y      Y inicial.
     *  @param width  Ancho del rectángulo.
     *  @param height Alto del rectángulo.
     *  @param color  Color RGB565. */
    void draw_block(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

    /** @brief Dibuja el contorno de un rectángulo (1px de grosor).
     *  @param x      X inicial.
     *  @param y      Y inicial.
     *  @param width  Ancho.
     *  @param height Alto.
     *  @param color  Color del borde. */
    void draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

    /** @brief Rellena un rectángulo (alias de draw_block, semántica más clara).
     *  @param x      X inicial.
     *  @param y      Y inicial.
     *  @param width  Ancho.
     *  @param height Alto.
     *  @param color  Color de relleno. */
    void fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

    // ---------------------------------------------------------------
    // Renderizado de texto
    // ---------------------------------------------------------------

    /** @brief Dibuja un carácter ASCII (fuente bitmap 8x8).
     *  @param x     Posición X.
     *  @param y     Posición Y.
     *  @param chr   Carácter a dibujar.
     *  @param color Color del texto.
     *  @param bg    Color de fondo (0xFFFF para transparente). */
    void draw_char(uint16_t x, uint16_t y, char chr, uint16_t color, uint16_t bg);

    /** @brief Dibuja una cadena de texto.
     *  @param x     Posición X inicial.
     *  @param y     Posición Y.
     *  @param str   Cadena a dibujar.
     *  @param color Color del texto.
     *  @param bg    Color de fondo. */
    void draw_string(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg);

    /** @brief Dibuja texto centrado horizontalmente.
     *  @param center_x Centro X de la pantalla.
     *  @param y         Posición Y.
     *  @param str       Cadena a dibujar.
     *  @param color     Color del texto.
     *  @param bg        Color de fondo. */
    void draw_string_centered(uint16_t center_x, uint16_t y, const char* str,
                              uint16_t color, uint16_t bg);

    /** @brief Dibuja un carácter ASCII con escala (usando fill_rect).
     *  @param x      Posición X.
     *  @param y      Posición Y.
     *  @param chr    Carácter a dibujar.
     *  @param color  Color del texto.
     *  @param bg     Color de fondo (0xFFFF = transparente).
     *  @param scale  Factor de escala (1 = 8x8, 2 = 16x16, 3 = 24x24...). */
    void draw_char_scaled(uint16_t x, uint16_t y, char chr,
                          uint16_t color, uint16_t bg, uint8_t scale);

    /** @brief Dibuja cadena con factor de escala.
     *  @param x      Posición X.
     *  @param y      Posición Y.
     *  @param str    Cadena a dibujar.
     *  @param color  Color del texto.
     *  @param bg     Color de fondo.
     *  @param scale  Factor de escala. */
    void draw_string_scaled(uint16_t x, uint16_t y, const char* str,
                            uint16_t color, uint16_t bg, uint8_t scale);

    /** @brief Dibuja cadena escalada centrada horizontalmente.
     *  @param center_x Centro X.
     *  @param y         Posición Y.
     *  @param str       Cadena.
     *  @param color     Color.
     *  @param bg        Fondo.
     *  @param scale     Escala. */
    void draw_string_centered_scaled(uint16_t center_x, uint16_t y, const char* str,
                                     uint16_t color, uint16_t bg, uint8_t scale);

    // ---------------------------------------------------------------
    // Visualización de imágenes
    // ---------------------------------------------------------------

    /** @brief Escribe un bloque de píxeles RGB565 en el display.
     *
     *  Usa set_area + escritura masiva. Mucho más rápido que draw_pixel por píxel.
     *
     *  @param x    X inicial.
     *  @param y    Y inicial.
     *  @param w    Ancho en píxeles.
     *  @param h    Alto en píxeles.
     *  @param data Arreglo de colores RGB565 (w × h píxeles, row-major). */
    void draw_pixels(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                     const uint16_t* data);

    /** @brief Carga y muestra una imagen RGB565 raw desde archivo.
     *
     *  El archivo debe tener exactamente LCD_WIDTH * LCD_HEIGHT * 2 bytes
     *  en formato big-endian RGB565.
     *
     *  @param filepath Ruta al archivo .rgb565. */
    void draw_image_rgb565(const char* filepath);

private:
    // ---------------------------------------------------------------
    // Primitivas del bus 8080
    // ---------------------------------------------------------------

    /** @brief Escribe 16 bits en el bus de datos GPIO usando multi-write. */
    void write_data_bus(uint16_t data);

    /** @brief Ciclo de escritura de comando (RS=LOW). */
    void write_command(uint8_t cmd);

    /** @brief Ciclo de escritura de datos (RS=HIGH, CS toggles). */
    void write_data(uint16_t data);

    /** @brief Convierte 16-bit data a máscara GPIO (D0-D15). */
    static uint32_t data_to_mask(uint16_t data);

    /** @brief Define el área activa de escritura en GRAM. */
    void set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    // ---------------------------------------------------------------
    // Fuente bitmap 8x8 (ASCII 0x20-0x7F)
    // ---------------------------------------------------------------

    /** @brief Tabla de fuente bitmap 8x8 para caracteres ASCII imprimibles. */
    static const uint8_t m_font_8x8[95][8];
};
