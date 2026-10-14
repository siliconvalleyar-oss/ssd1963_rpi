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

constexpr unsigned int TFTGL_IGNORE_TOUCH = 0x10;

constexpr unsigned int TFTGL_CALIB_MIN_X = 0;
constexpr unsigned int TFTGL_CALIB_MAX_X = 1;
constexpr unsigned int TFTGL_CALIB_MIN_Y = 2;
constexpr unsigned int TFTGL_CALIB_MAX_Y = 3;

#define ADS_CMD_START   0x80
#define ADS_CMD_12BIT   0x00
#define ADS_CMD_8BIT    0x04
#define ADS_CMD_DFR     0x00
#define ADS_CMD_POS_X   (0x5 << 4)
#define ADS_CMD_POS_Y   (0x1 << 4)
#define ADS_CMD_POS_Z1  (0x3 << 4)
#define ADS_CMD_POS_Z2  (0x4 << 4)
#define ADS_CMD_PWR     0x03

// Hardware pinout for ADS7843 touch via SPI0:
//   MOSI=GPIO10, SCLK=GPIO11, MISO=GPIO9, CS(touch)=GPIO7 (CE1), INT=GPIO18
#define TOUCH_CS_PIN    BCM2835_SPI_CS1    // GPIO7 / CE1
#define TOUCH_INT_PIN   18                 // GPIO18 (INT from ADS7843)
#define TOUCH_SPI_DIV   BCM2835_SPI_CLOCK_DIVIDER_4096

class ADS7843_Touch {
public:
    ADS7843_Touch();
    ~ADS7843_Touch();

    unsigned int begin(unsigned int flags = 0);
    void end();

    void set_sensitivity(unsigned int min_pressure);
    void set_calibration(unsigned int which, double raw, unsigned int pixel);

    unsigned int get_touch_raw(unsigned int* x, unsigned int* y, unsigned int* z);
    unsigned int get_touch(unsigned int* x, unsigned int* y);

private:
    static double remap(double value, double inMin, double inMax,
                        double outMin, double outMax);
    unsigned int read_channel_12bit(unsigned int target, unsigned int samples);

    unsigned int m_min_pressure = 100;
    double m_cal[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
    unsigned int m_initialized = 0;
};
