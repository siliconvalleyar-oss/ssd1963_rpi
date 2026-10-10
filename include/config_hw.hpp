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
constexpr uint8_t SSD1963_LCD_BACKLIGHT = 12; // Backlight control (era GPIO 0, ahora GPIO 12 libre tras usar 74HC595)


// Dimensiones del display
//constexpr unsigned int LCD_WIDTH  = 480;
//constexpr unsigned int LCD_HEIGHT = 272;



static constexpr uint16_t LCD_WIDTH = 480;
static constexpr uint16_t LCD_HEIGHT = 272;


// Sincronización horizontal
constexpr uint16_t TFT_HSYNC_PULSE        = 41;
constexpr uint16_t TFT_HSYNC_BACK_PORCH   = 2;
constexpr uint16_t TFT_HSYNC_FRONT_PORCH  = 2;
constexpr uint16_t TFT_HSYNC_PERIOD = LCD_WIDTH + TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH + TFT_HSYNC_FRONT_PORCH;

// Sincronización vertical
constexpr uint16_t TFT_VSYNC_PULSE        = 10;
constexpr uint16_t TFT_VSYNC_BACK_PORCH   = 2;
constexpr uint16_t TFT_VSYNC_FRONT_PORCH  = 2;
constexpr uint16_t TFT_VSYNC_PERIOD = LCD_HEIGHT + TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH + TFT_VSYNC_FRONT_PORCH;

// Frecuencia de píxel: PCLK = SYSCLK × (FPR + 1) / 2^20
// Oscilador 12MHz → PLL = 12MHz × 50 / 5 = 120MHz
// Para 480×272@60Hz con PLL=120MHz: FPR = 0x13332 (≈9MHz)
// (mismo valor que referencia PIC32 para idéntico panel)
constexpr uint32_t LCD_FPR = 0x13332;
