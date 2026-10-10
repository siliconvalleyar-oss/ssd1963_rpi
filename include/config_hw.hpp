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
constexpr uint8_t SSD1963_LCD_RD    = 2;   // Read strobe (GPIO2)
constexpr uint8_t SSD1963_LCD_RS    = 4;   // Data/Command selection
constexpr uint8_t SSD1963_LCD_CS    = 5;   // Chip select
constexpr uint8_t SSD1963_LCD_RESET = 6;   // Reset
constexpr uint8_t SSD1963_LCD_TE      = 0;   // Tearing Effect (INPUT from SSD1963)
constexpr uint8_t SSD1963_LCD_PW1     = 13;  // PW1/PWM backlight (INPUT from SSD1963)
constexpr uint8_t SSD1963_LCD_BACKLIGHT = 1;  // Backlight enable (OUTPUT - software controlled via Pi GPIO 1)

// Dimensiones del display
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
constexpr uint32_t LCD_FPR = 0x00FFBE;

// SET_LCD_MODE (0xB0) parameter A - panel configuration (from working reference)
// 0x20 = 24-bit panel, no FRC, LSHIFT falling, sync active low
#define LCD_MODE_CONFIG 0x20
