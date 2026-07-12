#pragma once
#include <cstdint>

struct RGB565 {
    uint16_t raw;

    constexpr RGB565() : raw(0) {}
    constexpr RGB565(uint16_t v) : raw(v) {}

    static constexpr RGB565 from_rgb(uint8_t r, uint8_t g, uint8_t b) {
        return RGB565(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
    }

    uint8_t r() const { return (raw >> 8) & 0xF8; }
    uint8_t g() const { return (raw >> 3) & 0xFC; }
    uint8_t b() const { return (raw << 3) & 0xF8; }

    operator uint16_t() const { return raw; }
};

#define RGB565CONVERT(red, green, blue) RGB565::from_rgb(red, green, blue).raw

// Colores básicos en formato RGB565
constexpr RGB565 BLACK   = RGB565(0x0000);
constexpr RGB565 BLUE    = RGB565(0x001F);
constexpr RGB565 RED     = RGB565(0xF800);
constexpr RGB565 GREEN   = RGB565(0x07E0);
constexpr RGB565 CYAN    = RGB565(0x07FF);
constexpr RGB565 MAGENTA = RGB565(0xF81F);
constexpr RGB565 YELLOW  = RGB565(0xFFE0);
constexpr RGB565 WHITE   = RGB565(0xFFFF);


/*
#define BLACK               RGB565CONVERT(0,    0,      0)
#define BRIGHTBLUE          RGB565CONVERT(0,    0,      255)
#define BRIGHTGREEN         RGB565CONVERT(0,    255,    0)
#define BRIGHTCYAN          RGB565CONVERT(0,    255,    255)
#define BRIGHTRED           RGB565CONVERT(255,  0,      0)
#define BRIGHTMAGENTA       RGB565CONVERT(255,  0,      255)
#define BRIGHTYELLOW        RGB565CONVERT(255,  255,    0)
#define BLUE                RGB565CONVERT(0,    0,      128)
#define GREEN               RGB565CONVERT(0,    128,    0)
#define CYAN                RGB565CONVERT(0,    128,    128)
#define RED                 RGB565CONVERT(128,  0,      0)
#define MAGENTA             RGB565CONVERT(128,  0,      128)
#define BROWN               RGB565CONVERT(255,  128,    0)
#define LIGHTGRAY           RGB565CONVERT(128,  128,    128)
#define DARKGRAY            RGB565CONVERT(64,   64,     64)
#define LIGHTBLUE           RGB565CONVERT(128,  128,    255)
#define LIGHTGREEN          RGB565CONVERT(128,  255,    128)
#define LIGHTCYAN           RGB565CONVERT(128,  255,    255)
#define LIGHTRED            RGB565CONVERT(255,  128,    128)
#define LIGHTMAGENTA        RGB565CONVERT(255,  128,    255)
#define YELLOW              RGB565CONVERT(255,  255,    128)
#define WHITE               RGB565CONVERT(255,  255,    255)
#define SKY               RGB565CONVERT(200,  255,    255)
#define CLOUD               RGB565CONVERT(254,254,254)
                            
#define GRAY0       	    RGB565CONVERT(224,  224,    224)
#define GRAY1         	    RGB565CONVERT(192,  192,    192)   
#define GRAY2               RGB565CONVERT(160,  160,    160)   
#define GRAY3               RGB565CONVERT(128,  128,    128)
#define GRAY4               RGB565CONVERT(96,   96,     96)
#define GRAY5               RGB565CONVERT(64,   64,     64)
#define GRAY6	            RGB565CONVERT(32,   32,     32)

*/
