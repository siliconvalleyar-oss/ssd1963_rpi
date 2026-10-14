#pragma once
#include <cstdint>


// Definición de pines - 74HC595 Shift Registers
// Configura USE_8BIT_MODE para probar con 1 solo 74HC595 (8 bits D0-D7)
#define USE_8BIT_MODE  0   // 1 = 8 bits (1x 74HC595), 0 = 16 bits (2x 74HC595)

constexpr uint8_t SSD1963_SR_SER   = 1;  // Serial data input (pin 14 of first 74HC595)
constexpr uint8_t SSD1963_SR_SRCLK = 19; // Shift register clock (pin 11 of both 74HC595)
constexpr uint8_t SSD1963_SR_RCLK  = 20; // Storage register clock / latch (pin 12 of both 74HC595)

constexpr uint8_t SSD1963_LCD_WR    = 3;  // Write strobe
constexpr uint8_t SSD1963_LCD_RD    = 2;  // Read strobe (GPIO2 - para leer status/registros)
constexpr uint8_t SSD1963_LCD_RS    = 4;  // Data/Command selection
constexpr uint8_t SSD1963_LCD_CS    = 5;  // Chip select
constexpr uint8_t SSD1963_LCD_RESET = 6;  // Reset
constexpr uint8_t SSD1963_LCD_TE      = 12; // Tearing Effect (INPUT from SSD1963)
constexpr uint8_t SSD1963_LCD_PW1     = 13; // PW1/PWM backlight (INPUT from SSD1963)
constexpr uint8_t SSD1963_LCD_BACKLIGHT = 13; // Backlight enable (OUTPUT - software controlled via Pi GPIO 13)


// Dimensiones del display
//constexpr unsigned int LCD_WIDTH  = 480;
//constexpr unsigned int LCD_HEIGHT = 272;


static constexpr uint16_t LCD_WIDTH = 480;
static constexpr uint16_t LCD_HEIGHT = 272;


// Sincronización horizontal - Based on stm32plus SSD1963_480x272PanelTraits
constexpr uint16_t TFT_HSYNC_PULSE        = 41;
constexpr uint16_t TFT_HSYNC_BACK_PORCH   = 2;
constexpr uint16_t TFT_HSYNC_FRONT_PORCH  = 2;
constexpr uint16_t TFT_HSYNC_PERIOD = LCD_WIDTH + TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH + TFT_HSYNC_FRONT_PORCH;

// Sincronización vertical - Based on stm32plus SSD1963_480x272PanelTraits
constexpr uint16_t TFT_VSYNC_PULSE        = 10;
constexpr uint16_t TFT_VSYNC_BACK_PORCH   = 2;
constexpr uint16_t TFT_VSYNC_FRONT_PORCH  = 2;
constexpr uint16_t TFT_VSYNC_PERIOD = LCD_HEIGHT + TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH + TFT_VSYNC_FRONT_PORCH;

// Frecuencia de píxel: PCLK = SYSCLK * (FPR+1) / 2^20
// 0x0170A2 con SYSCLK = 100 MHz -> PCLK = 9.00 MHz (igual que la referencia PIC32
// para el panel TY430TFT480272)
constexpr uint32_t LCD_FPR = 0x0170A2; // 9.0 MHz para SYSCLK = 100 MHz

// =========================================================================
// OPCION: Replicar forma de onda RS exacta del micro que funciona
// =========================================================================
// El analizador logico mostro que el micro que funciona genera:
// COMANDO:
//   1. RS baja 40 ns ANTES de CS (pre-pulso)
//   2. WR pulsa ~65 ns
//   3. WR sube 10 ns ANTES de CS
//   4. RS se queda LOW 95 ns DESPUES de CS↑, luego sube a HIGH
// PRIMER DATO tras comando:
//   RS pulsa: HIGH 40 ns -> LOW 30 ns -> HIGH (con CS↓)
// RESTO DE DATOS:
//   RS se queda HIGH fijo
// Activar para replicar exactamente esta forma de onda (requiere clock_gettime para ns).
constexpr bool SSD1963_MCU_WAVEFORM = false;

// Timings de arranque STM32 (observados en captura de analizador logico)
constexpr uint16_t SSD1963_RESET_LOW_MS = 2000;      // RST en bajo durante power-up
constexpr uint16_t SSD1963_POST_RESET_MS = 1000;     // Espera tras liberar RST (STM32: ~1.0s)
constexpr uint16_t SSD1963_RS_LEAD_MS = 50;          // RS HIGH antes del 1er comando (STM32: ~50ms)

// SET_LCD_MODE (0xB0) parameter A - panel configuration
// A[5]: TFT panel data width (0=18-bit, 1=24-bit)
// A[4]: Color depth enhancement (0=disable, 1=enable FRC/dithering)
// A[3]: TFT FRC enable (0=dithering, 1=FRC) - ignored if A[5]=1
// A[2]: LSHIFT polarity (0=falling edge, 1=rising edge)
// A[1]: LLINE polarity (0=active low, 1=active high)
// A[0]: LFRAME polarity (0=active low, 1=active high)
//
// Common values for 480x272 panels:
// 0x0C = 0b00001100 = 18-bit panel, no FRC, LSHIFT falling, sync active low
// 0x2C = 0b00101100 = 24-bit panel, no FRC, LSHIFT falling, sync active low
// 0x0E = 0b00001110 = 18-bit panel, FRC enabled, LSHIFT falling, sync active low
// 0x08 = 0b00001000 = 18-bit panel, 8-bit MCU bus
#define LCD_MODE_CONFIG 0x10  // 18-bit panel with FRC (0x10 = A5=0, A4=1, A3=1)