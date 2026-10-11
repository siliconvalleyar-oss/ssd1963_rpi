#pragma once
#include <cstdint>

// Definición de pines - Bus paralelo directo (D0-D15 en GPIO 12-27)
constexpr uint8_t SSD1963_LCD_D0  = 12;
constexpr uint8_t SSD1963_LCD_D1  = 13;
constexpr uint8_t SSD1963_LCD_D2  = 14;
constexpr uint8_t SSD1963_LCD_D3  = 15;
constexpr uint8_t SSD1963_LCD_D4  = 16;
constexpr uint8_t SSD1963_LCD_D5  = 17;
constexpr uint8_t SSD1963_LCD_D6  = 18;
constexpr uint8_t SSD1963_LCD_D7  = 19;
constexpr uint8_t SSD1963_LCD_D8  = 20;
constexpr uint8_t SSD1963_LCD_D9  = 21;
constexpr uint8_t SSD1963_LCD_D10 = 22;
constexpr uint8_t SSD1963_LCD_D11 = 23;
constexpr uint8_t SSD1963_LCD_D12 = 24;
constexpr uint8_t SSD1963_LCD_D13 = 25;
constexpr uint8_t SSD1963_LCD_D14 = 26;
constexpr uint8_t SSD1963_LCD_D15 = 27;
constexpr uint8_t SSD1963_LCD_WR    = 3;   // Write strobe
constexpr uint8_t SSD1963_LCD_RD    = 2;   // Read strobe (GPIO2): se mantiene en alto
constexpr uint8_t SSD1963_LCD_RS    = 4;   // Data/Command selection
constexpr uint8_t SSD1963_LCD_CS    = 5;   // Chip select
constexpr uint8_t SSD1963_LCD_RESET = 6;   // Reset

// PW1: pin PWM del SSD1963 leído como ENTRADA por la Pi (solo para diagnóstico)
constexpr uint8_t SSD1963_LCD_PW1     = 7;

// Backlight enable (SALIDA de la Pi hacia el módulo).
// IMPORTANTE: NO puede estar entre GPIO 12 y 27, porque ese rango es el bus de
// datos y write_data_bus() lo sobreescribe en cada escritura.
// (Antes estaba en GPIO13 = D1, y por eso el backlight se apagaba al dibujar.)
// Se eliminó SSD1963_LCD_TE: estaba en GPIO12 = D0 y no se usa.
// >>> Hay que recablear BL_EN al GPIO 7 (o cambiar este numero a otro GPIO libre: 7-11). <<<
constexpr uint8_t SSD1963_LCD_BACKLIGHT = 0;

// Tiempo que RST se mantiene en BAJO al arrancar (ms), antes de inicializar el SSD1963
constexpr uint32_t SSD1963_RESET_HOLD_MS = 2000;

// Dimensiones del display
static constexpr uint16_t LCD_WIDTH = 480;
static constexpr uint16_t LCD_HEIGHT = 272;

// Filas por ráfaga al volcar la pantalla (clear_screen y draw_buffer).
//   272 = una sola ráfaga por cuadro (más rápido).
//   Si aparece corrupción, bajar a 32 o a 1 (1 = una fila por ráfaga, comportamiento anterior).
constexpr uint16_t SSD1963_BURST_ROWS = LCD_HEIGHT;

// Sincronización horizontal (PIC32 reference: H pulse=41, back=2, front=2)
constexpr uint16_t TFT_HSYNC_PULSE        = 41;  // SSD_HOR_PULSE_WIDTH
constexpr uint16_t TFT_HSYNC_BACK_PORCH   = 2;   // SSD_HOR_BACK_PORCH
constexpr uint16_t TFT_HSYNC_FRONT_PORCH  = 2;   // SSD_HOR_FRONT_PORCH
constexpr uint16_t TFT_HSYNC_PERIOD = LCD_WIDTH + TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH + TFT_HSYNC_FRONT_PORCH;

// Sincronización vertical (PIC32 reference: V pulse=10, back=2, front=2)
constexpr uint16_t TFT_VSYNC_PULSE        = 10;  // SSD_VER_PULSE_WIDTH
constexpr uint16_t TFT_VSYNC_BACK_PORCH   = 2;   // SSD_VER_BACK_PORCH
constexpr uint16_t TFT_VSYNC_FRONT_PORCH  = 2;   // SSD_VER_FRONT_PORCH
constexpr uint16_t TFT_VSYNC_PERIOD = LCD_HEIGHT + TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH + TFT_VSYNC_FRONT_PORCH;

// ---------------------------------------------------------------------------
// PLL (cristal del módulo = 10 MHz)
//   VCO = 10MHz * (M+1)   (debe estar entre 250 y 800 MHz)
//   PLL = VCO / (N+1)     (N tiene solo 4 bits: 0..15)
// El system clock del SSD1963 NO puede pasar de 110 MHz (datasheet, tabla 13-x, fMCLK).
// M=29, N=2  ->  VCO = 300 MHz, PLL = 100 MHz  (es el ejemplo del propio datasheet, sec. 9.68)
// ---------------------------------------------------------------------------
constexpr uint8_t SSD1963_PLL_M = 29;
constexpr uint8_t SSD1963_PLL_N = 2;

// Frecuencia de píxel: PCLK = PLL * (FPR + 1) / 2^20
// Con PLL = 100 MHz: FPR = 0x0170A2 (94370) -> 9.00 MHz (~57.7 Hz para 532x293 totales)
constexpr uint32_t LCD_FPR = 0x0170A2;

// SET_LCD_MODE (0xB0) parametro A - configuración del panel
// A[5]: ancho de datos del panel (0=18-bit, 1=24-bit)
// A[4]: mejora de color (FRC/dithering)
// A[3]: FRC (ignorado si A[4]=0)
// A[2]: polaridad LSHIFT (0 = dato capturado en flanco de bajada, 1 = subida)
// A[1]: polaridad LLINE  (0 = activo en bajo)
// A[0]: polaridad LFRAME (0 = activo en bajo)
// 0x10 = panel 18-bit con FRC (A[4]=1, A[5]=0)
#define LCD_MODE_CONFIG 0x10

// SET_ADDR_MODE (0x36): 0x00 = RGB, sin flips (es el valor POR)
//   0x08 = BGR (si rojo y azul salen intercambiados)
//   0x02 = flip horizontal | 0x01 = flip vertical
#define LCD_ADDR_MODE 0x00
