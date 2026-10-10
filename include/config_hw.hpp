#pragma once
#include <cstdint>


// Definición de pines - 74HC595 Shift Registers
// Configura USE_8BIT_MODE para probar con 1 solo 74HC595 (8 bits D0-D7)
#define USE_8BIT_MODE  0   // 0 = 16 bits (2x 74HC595), 1 = 8 bits (1x 74HC595)

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


// Sincronización horizontal - From working reference (tmp/4.3inch SSD1963 STM32)
constexpr uint16_t TFT_HSYNC_PULSE        = 1;   // SSD_HOR_PULSE_WIDTH
constexpr uint16_t TFT_HSYNC_BACK_PORCH   = 43;  // SSD_HOR_BACK_PORCH
constexpr uint16_t TFT_HSYNC_FRONT_PORCH  = 8;   // SSD_HOR_FRONT_PORCH
constexpr uint16_t TFT_HSYNC_PERIOD = LCD_WIDTH + TFT_HSYNC_PULSE + TFT_HSYNC_BACK_PORCH + TFT_HSYNC_FRONT_PORCH;

// Sincronización vertical - From working reference (tmp/4.3inch SSD1963 STM32)
constexpr uint16_t TFT_VSYNC_PULSE        = 1;   // SSD_VER_PULSE_WIDTH
constexpr uint16_t TFT_VSYNC_BACK_PORCH   = 12;  // SSD_VER_BACK_PORCH
constexpr uint16_t TFT_VSYNC_FRONT_PORCH  = 8;   // SSD_VER_FRONT_PORCH
constexpr uint16_t TFT_VSYNC_PERIOD = LCD_HEIGHT + TFT_VSYNC_PULSE + TFT_VSYNC_BACK_PORCH + TFT_VSYNC_FRONT_PORCH;

// Frecuencia de píxel: PCLK = PLL * (FPR + 1) / 2^20
// Reference: 10MHz xtal, PLL=150MHz (M=2, N=45) → FPR = 0x00FFBE for ~9.3MHz
// Our 12MHz osc: M=49, N=4 → PLL=120MHz → adjust FPR accordingly
constexpr uint32_t LCD_FPR = 0x00FFBE;

// SET_LCD_MODE (0xB0) parameter A - panel configuration (from working reference)
// 0x20 = 24-bit panel, no FRC, LSHIFT falling, sync active low
#define LCD_MODE_CONFIG 0x20
