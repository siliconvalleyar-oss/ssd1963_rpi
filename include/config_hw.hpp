#pragma once
#include <cstdint>

// Definición de pines - Bus paralelo directo (D0-D15 en GPIO 12-27)
// Configura USE_8BIT_MODE para probar con 1 solo 74HC595 (8 bits D0-D7)
#define USE_8BIT_MODE  0   // 0 = 16 bits (bus directo D0-D15), 1 = 8 bits (no implementado)

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
constexpr uint8_t SSD1963_LCD_TE      = 1;   // Tearing Effect (INPUT from SSD1963)
constexpr uint8_t SSD1963_LCD_PW1     = 7;   // PW1/PWM backlight (INPUT from SSD1963)
constexpr uint8_t SSD1963_LCD_BACKLIGHT = 0;  // Backlight enable (OUTPUT - software controlled via Pi GPIO 0)

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
// Reference: 10MHz xtal, PLL=100MHz (M=29, N=2) → FPR = 0x0170A2 for ~9.0MHz
constexpr uint32_t LCD_FPR = 0x0170A2;

// PLL (cristal del módulo = 10 MHz)
//   VCO = 10MHz * (M+1)   (debe estar entre 250 y 800 MHz)
//   PLL = VCO / (N+1)      (máximo 110 MHz según tabla 13-6)
constexpr uint8_t  SSD1963_PLL_M = 29;
constexpr uint8_t  SSD1963_PLL_N = 2;

// SET_LCD_MODE (0xB0) parameter A - panel configuration (from working reference)
// 0x00 = 18-bit panel, no FRC, LSHIFT falling, sync active low
#define LCD_MODE_CONFIG 0x00
