// tftgl_ads7843.hpp
//
// Puerto C++ (autónomo, sin EGL) del driver de touch ADS7843 en
// tmp/tftgl_ads7843.h. Lee X/Y/Z vía SPI (bcm2835_spi_*) con
// recalibración, estilo de la clase SSD1963 del proyecto.
//
// El driver original (C) dependía de tftgl.h + EGL. Aquí se encapsula en una
// clase usando solo libbcm2835, consistente con src/ssd1963.cpp.

#pragma once
#include <bcm2835.h>
#include <cstdint>

namespace TouchStatus {
    constexpr unsigned int OK          = 1;
    constexpr unsigned int ERROR       = 0;
    constexpr unsigned int SPI_ERROR   = 3;
    constexpr unsigned int GOT_TOUCH  = 1;
    constexpr unsigned int NO_TOUCH   = 0;
}

// Flags de inicialización
constexpr unsigned int TFTGL_IGNORE_TOUCH = 0x10;

// Puntos de calibración (índices del arreglo calibrado)
constexpr unsigned int TFTGL_CALIB_MIN_X = 0;
constexpr unsigned int TFTGL_CALIB_MAX_X = 1;
constexpr unsigned int TFTGL_CALIB_MIN_Y = 2;
constexpr unsigned int TFTGL_CALIB_MAX_Y = 3;

// Comandos ADS7843 (bit 7 = START, bit 6 = 12/8 bit, bit 5 = DF, bits 4..2 = canal/pwr)
#define ADS_CMD_START   0x80
#define ADS_CMD_12BIT   0x00
#define ADS_CMD_8BIT    0x04
#define ADS_CMD_DFR     0x00
#define ADS_CMD_POS_X   (0x5 << 4)
#define ADS_CMD_POS_Y   (0x1 << 4)
#define ADS_CMD_POS_Z1  (0x3 << 4)
#define ADS_CMD_POS_Z2  (0x4 << 4)
#define ADS_CMD_PWR     0x03

#define TOUCH_CS_PIN    BCM2835_SPI_CS0
#define TOUCH_SPI_DIV   BCM2835_SPI_CLOCK_DIVIDER_4096

class ADS7843_Touch {
public:
    ADS7843_Touch();
    ~ADS7843_Touch();

    // Inicializa el bus SPI del touch. flags & TFTGL_IGNORE_TOUCH desactiva.
    unsigned int begin(unsigned int flags = 0);
    void end();

    void set_sensitivity(unsigned int min_pressure);
    void set_calibration(unsigned int which, double raw, unsigned int pixel);

    // Lectura raw de X/Y/Z (12-bit). Devuelve OK.
    unsigned int get_touch_raw(unsigned int* x, unsigned int* y, unsigned int* z);

    // Lectura con calibración. Devuelve GOT_TOUCH / NO_TOUCH.
    unsigned int get_touch(unsigned int* x, unsigned int* y);

private:
    static double remap(double value, double inMin, double inMax,
                        double outMin, double outMax);
    unsigned int read_channel_12bit(unsigned int target, unsigned int samples);

    unsigned int m_min_pressure = 100;
    // m_cal[which][0] = raw, m_cal[which][1] = pixel
    double m_cal[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
    unsigned int m_initialized = 0;
};
