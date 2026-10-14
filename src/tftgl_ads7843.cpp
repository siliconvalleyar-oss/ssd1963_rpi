// tftgl_ads7843.cpp
// Implementación C++ del driver de touch ADS7843 (puerto de tmp/tftgl_ads7843.h).

#include <tftgl_ads7843.hpp>
#include <bcm2835.h>
#include <cstdint>

static double remap(double value, double inMin, double inMax,
                    double outMin, double outMax) {
    return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

double ADS7843_Touch::remap(double value, double inMin, double inMax,
                            double outMin, double outMax) {
    return ::remap(value, inMin, inMax, outMin, outMax);
}

ADS7843_Touch::ADS7843_Touch() {}

ADS7843_Touch::~ADS7843_Touch() {
    end();
}

unsigned int ADS7843_Touch::begin(unsigned int flags) {
    if (flags & TFTGL_IGNORE_TOUCH) {
        m_initialized = 1;
        return TouchStatus::OK;
    }
    // 2048 ~= 122 KHz; nada más puede leer de forma inestable.
    if (!bcm2835_spi_begin()) {
        m_initialized = 0;
        return TouchStatus::SPI_ERROR;
    }
    bcm2835_spi_chipSelect(TOUCH_CS_PIN);
    m_initialized = 1;
    return TouchStatus::OK;
}

void ADS7843_Touch::end() {
    if (m_initialized) {
        bcm2835_spi_end();
    }
    m_initialized = 0;
}

void ADS7843_Touch::set_sensitivity(unsigned int min_pressure) {
    m_min_pressure = min_pressure;
}

void ADS7843_Touch::set_calibration(unsigned int which, double raw, unsigned int pixel) {
    if (which > TFTGL_CALIB_MAX_Y) return;
    m_cal[which][0] = raw;
    m_cal[which][1] = static_cast<double>(pixel);
}

unsigned int ADS7843_Touch::read_channel_12bit(unsigned int target, unsigned int samples) {
    unsigned char out[3];
    unsigned char buf[3];
    buf[0] = static_cast<unsigned char>(ADS_CMD_START | ADS_CMD_12BIT | ADS_CMD_DFR | target | ADS_CMD_PWR);

    unsigned int acc = 0;
    for (unsigned int i = 0; i < samples; i++) {
        bcm2835_spi_transfernb(buf, out, 3);
        acc += ((out[1] << 8 | out[2]) >> 3);
    }
    return acc / samples;
}

unsigned int ADS7843_Touch::get_touch_raw(unsigned int* x, unsigned int* y, unsigned int* z) {
    bcm2835_spi_setClockDivider(TOUCH_SPI_DIV);
    bcm2835_spi_chipSelect(TOUCH_CS_PIN);

    if (z != nullptr) *z = read_channel_12bit(ADS_CMD_POS_Z1, 8);
    if (x != nullptr) *x = read_channel_12bit(ADS_CMD_POS_X, 8);
    if (y != nullptr) *y = read_channel_12bit(ADS_CMD_POS_Y, 8);
    return TouchStatus::OK;
}

unsigned int ADS7843_Touch::get_touch(unsigned int* x, unsigned int* y) {
    bcm2835_spi_setClockDivider(TOUCH_SPI_DIV);
    bcm2835_spi_chipSelect(TOUCH_CS_PIN);

    unsigned int press = read_channel_12bit(ADS_CMD_POS_Z1, 1);
    if (press > m_min_pressure) {
        if (x != nullptr) {
            double rawx = static_cast<double>(read_channel_12bit(ADS_CMD_POS_X, 8));
            double weight = remap(rawx, m_cal[0][0], m_cal[1][0], 0.0, 1.0);
            *x = static_cast<unsigned int>(remap(weight, 0.0, 1.0, m_cal[0][1], m_cal[1][1]));
        }
        if (y != nullptr) {
            double rawy = static_cast<double>(read_channel_12bit(ADS_CMD_POS_Y, 8));
            double weight = remap(rawy, m_cal[2][0], m_cal[3][0], 0.0, 1.0);
            *y = static_cast<unsigned int>(remap(weight, 0.0, 1.0, m_cal[2][1], m_cal[3][1]));
        }
        return TouchStatus::GOT_TOUCH;
    }
    return TouchStatus::NO_TOUCH;
}
