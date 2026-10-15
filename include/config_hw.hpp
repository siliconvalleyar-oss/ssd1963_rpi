#pragma once
#include <cstdint>


// Definición de pines
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
constexpr uint8_t SSD1963_LCD_WR    = 3;  // Write strobe
constexpr uint8_t SSD1963_LCD_RS    = 4;  // Data/Command selection
constexpr uint8_t SSD1963_LCD_CS    = 5;  // Chip select
constexpr uint8_t SSD1963_LCD_RESET = 6;  // Reset
constexpr uint8_t SSD1963_LCD_BACKLIGHT = 0; // Backlight control ( accomp: PW1/GPIO0 del SSD1963 )

constexpr uint8_t SSD1963_LCD_TE    = 7;
constexpr uint8_t SSD1963_LCD_RD    = 2;

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

// =========================================================================
// Espejo horizontal (bit A[1] del comando 0x36 SET ADDRESS MODE)
// El panel TFT montado muestra la imagen invertida de derecha a izquierda
// (FPC del panel cableada al reves); activar corrige el espejo sin tocar
// el frame buffer. Ver docs/SSD1963.PDF, figura 9-15 "A[1] Flip Horizontal".
// =========================================================================
constexpr bool SSD1963_MIRROR_X = true;