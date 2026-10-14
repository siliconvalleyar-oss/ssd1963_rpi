#pragma once
#include <cstdint>

// =============================================================================
// Merged color palette for SSD1963 TFT (RGB565).
// Consolidated from v7 / v8 / v9 experiment snapshots.
// La versión v9 dejaba la paleta extendida comentada; aquí se unifican todos
// los nombres de color experimentados en los distintos snapshots.
// =============================================================================

// Convert r/g/b (0-255) -> RGB565
#define RGB565CONVERT(red, green, blue) (uint16_t) (((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3))

// ---------------------------------------------------------------------------
// Colores básicos
// ---------------------------------------------------------------------------
#define BLACK               RGB565CONVERT(0,   0,   0)
#define BLUE                RGB565CONVERT(0,   0,   255)
#define RED                 RGB565CONVERT(255, 0,   0)
#define GREEN               RGB565CONVERT(0,   255, 0)
#define CYAN                RGB565CONVERT(0,   255, 255)
#define MAGENTA             RGB565CONVERT(255, 0,   255)
#define YELLOW              RGB565CONVERT(255, 255, 0)
#define WHITE               RGB565CONVERT(255, 255, 255)

// ---------------------------------------------------------------------------
// Paleta extendida (brillantes / tonos medios / grises)
// ---------------------------------------------------------------------------
#define BRIGHTBLUE          RGB565CONVERT(0,   0,   255)
#define BRIGHTGREEN         RGB565CONVERT(0,   255, 0)
#define BRIGHTCYAN          RGB565CONVERT(0,   255, 255)
#define BRIGHTRED           RGB565CONVERT(255, 0,   0)
#define BRIGHTMAGENTA       RGB565CONVERT(255, 0,   255)
#define BRIGHTYELLOW        RGB565CONVERT(255, 255, 0)

#define BROWN               RGB565CONVERT(255, 128, 0)

#define LIGHTGRAY           RGB565CONVERT(128, 128, 128)
#define DARKGRAY            RGB565CONVERT(64,  64,  64)

#define LIGHTBLUE           RGB565CONVERT(128, 128, 255)
#define LIGHTGREEN          RGB565CONVERT(128, 255, 128)
#define LIGHTCYAN           RGB565CONVERT(128, 255, 255)
#define LIGHTRED            RGB565CONVERT(255, 128, 128)
#define LIGHTMAGENTA        RGB565CONVERT(255, 128, 255)

#define SKY                 RGB565CONVERT(200, 255, 255)
#define CLOUD               RGB565CONVERT(254, 254, 254)

#define GRAY0       	    RGB565CONVERT(224, 224, 224)
#define GRAY1        	    RGB565CONVERT(192, 192, 192)
#define GRAY2               RGB565CONVERT(160, 160, 160)
#define GRAY3               RGB565CONVERT(128, 128, 128)
#define GRAY4               RGB565CONVERT(96,  96,  96)
#define GRAY5               RGB565CONVERT(64,  64,  64)
#define GRAY6 	            RGB565CONVERT(32,  32,  32)
