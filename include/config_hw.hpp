#pragma once
#include <cstdint>


// Definición de pines - 74HC595 Shift Registers
// Configura USE_8BIT_MODE para probar con 1 solo 74HC595 (8 bits D0-D7)
#define USE_8BIT_MODE  0   // 0 = 16 bits (2x 74HC595), 1 = 8 bits (1x 74HC595)

constexpr uint8_t SSD1963_SR_SER   = 1;  // Serial data input (pin 14 of first 74HC595)
constexpr uint8_t SSD1963_SR_SRCLK = 19; // Shift register clock (pin 11 of both 74HC595)
constexpr uint8_t SSD1963_SR_RCLK  = 20; // Storage register clock / latch (pin 12 of both 74HC595)

constexpr uint8_t SSD1963_LCD_WR    = 3;  // Write strobe
constexpr uint8_t SSD1963_LCD_RS    = 4;  // Data/Command selection
constexpr uint8_t SSD1963_LCD_CS    = 5;  // Chip select
constexpr uint8_t SSD1963_LCD_RESET = 6;  // Reset
constexpr uint8_t SSD1963_LCD_TE      = 13; // Tearing Effect (INPUT from SSD1963)
constexpr uint8_t SSD1963_LCD_PW1     = 0;  // PW1/PWM backlight (INPUT from SSD1963) - was 13, now free pin 0
constexpr uint8_t SSD1963_LCD_BACKLIGHT = 12; // Backlight control (GPIO 12 - user requested)


// Dimensiones del display
//constexpr unsigned int LCD_WIDTH  = 480;
//constexpr unsigned int LCD_HEIGHT = 272;



static constexpr uint16_t LCD_WIDTH = 480;
static constexpr uint16_t LCD_HEIGHT = 272;


// Sincronización horizontal - Based on stm32plus SSD1963_480x272PanelTraits
constexpr uint16_t TFT_HSYNC_PULSE        = 2;   // H_SYNC_PULSE_WIDTH
constexpr uint16_t TFT_HSYNC_BACK_PORCH   = 43;  // H_BACK_PORCH
constexpr uint16_t TFT_HSYNC_FRONT_PORCH  = 8;   // H_FRONT_PORCH
constexpr uint16_t TFT_HSYNC_PERIOD = LCD_WIDTH + TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH + TFT_HSYNC_FRONT_PORCH;

// Sincronización vertical - Based on stm32plus SSD1963_480x272PanelTraits
constexpr uint16_t TFT_VSYNC_PULSE        = 10;  // V_SYNC_PULSE_WIDTH
constexpr uint16_t TFT_VSYNC_BACK_PORCH   = 12;  // V_BACK_PORCH
constexpr uint16_t TFT_VSYNC_FRONT_PORCH  = 4;   // V_FRONT_PORCH
constexpr uint16_t TFT_VSYNC_PERIOD = LCD_HEIGHT + TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH + TFT_VSYNC_FRONT_PORCH;

// Frecuencia de píxel: PCLK = SYSCLK × (FPR + 1) / 2^20
// Oscilador 12MHz → PLL = 12MHz × 50 / 5 = 120MHz
// Para 480×272@60Hz con PLL=120MHz: FPR = 0x13332 (≈9MHz)
// (mismo valor que referencia PIC32 para idéntico panel)
constexpr uint32_t LCD_FPR = 0x13332;

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
#define LCD_MODE_CONFIG 0x0C  // Default: 18-bit panel, standard timing
