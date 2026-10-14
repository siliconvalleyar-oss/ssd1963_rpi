#pragma once

#include <cstdint>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdexcept>

// BCM2835/2836/2837 GPIO register offsets (from peripheral base)
#define GPIO_BASE_OFFSET 0x200000

// GPIO register offsets
#define GPFSEL0  0x00  // Function select 0
#define GPFSEL1  0x04  // Function select 1
#define GPFSEL2  0x08  // Function select 2
#define GPFSEL3  0x0C  // Function select 3
#define GPFSEL4  0x10  // Function select 4
#define GPFSEL5  0x14  // Function select 5

#define GPSET0   0x1C  // Pin output set 0
#define GPSET1   0x20  // Pin output set 1

#define GPCLR0   0x28  // Pin output clear 0
#define GPCLR1   0x2C  // Pin output clear 1

#define GPLEV0   0x34  // Pin level 0
#define GPLEV1   0x38  // Pin level 1

#define GPEDS0   0x40  // Pin event detect status 0
#define GPEDS1   0x44  // Pin event detect status 1

#define GPREN0   0x4C  // Pin rising edge detect enable 0
#define GPREN1   0x50  // Pin rising edge detect enable 1

#define GPFEN0   0x58  // Pin falling edge detect enable 0
#define GPFEN1   0x5C  // Pin falling edge detect enable 1

#define GPHEN0   0x64  // Pin high detect enable 0
#define GPHEN1   0x68  // Pin high detect enable 1

#define GPLEN0   0x70  // Pin low detect enable 0
#define GPLEN1   0x74  // Pin low detect enable 1

#define GPAREN0  0x7C  // Pin async rising edge detect 0
#define GPAREN1  0x80  // Pin async rising edge detect 1

#define GPAFEN0  0x88  // Pin async falling edge detect 0
#define GPAFEN1  0x8C  // Pin async falling edge detect 1

#define GPPUD    0x94  // Pin pull-up/down enable
#define GPPUDCLK0 0x98 // Pin pull-up/down clock 0
#define GPPUDCLK1 0x9C // Pin pull-up/down clock 1

// Function select values
#define FSEL_IN   0b000
#define FSEL_OUT  0b001
#define FSEL_ALT5 0b010
#define FSEL_ALT4 0b011
#define FSEL_ALT0 0b100
#define FSEL_ALT1 0b101
#define FSEL_ALT2 0b110
#define FSEL_ALT3 0b111

class GpioMmap {
public:
    GpioMmap() : mem_fd_(-1), gpio_map_(nullptr), gpio_((volatile uint32_t*)MAP_FAILED) {
        // Open /dev/gpiomem (no root needed if user in gpio group)
        mem_fd_ = open("/dev/gpiomem", O_RDWR | O_SYNC);
        if (mem_fd_ < 0) {
            // Fallback to /dev/mem (needs root)
            mem_fd_ = open("/dev/mem", O_RDWR | O_SYNC);
            if (mem_fd_ < 0) {
                throw std::runtime_error("Failed to open /dev/gpiomem or /dev/mem");
            }
        }

        // Map GPIO registers
        gpio_map_ = mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd_, GPIO_BASE_OFFSET);
        if (gpio_map_ == MAP_FAILED) {
            close(mem_fd_);
            throw std::runtime_error("mmap failed");
        }

        gpio_ = static_cast<volatile uint32_t*>(gpio_map_);
    }

    ~GpioMmap() {
        if (gpio_map_ && gpio_map_ != MAP_FAILED) {
            munmap(gpio_map_, 4096);
        }
        if (mem_fd_ >= 0) {
            close(mem_fd_);
        }
    }

    // Delete copy/move
    GpioMmap(const GpioMmap&) = delete;
    GpioMmap& operator=(const GpioMmap&) = delete;

    // Set pin function (0=input, 1=output, 4=alt0, etc.)
    void fsel(uint8_t pin, uint8_t func) {
        uint32_t reg = GPFSEL0 + (pin / 10) * 4;
        uint32_t shift = (pin % 10) * 3;
        uint32_t mask = 0b111 << shift;
        gpio_[reg / 4] = (gpio_[reg / 4] & ~mask) | ((func & 0b111) << shift);
    }

    // Set pin HIGH
    void set(uint8_t pin) {
        if (pin < 32) gpio_[GPSET0 / 4] = 1 << pin;
        else gpio_[GPSET1 / 4] = 1 << (pin - 32);
    }

    // Set pin LOW
    void clr(uint8_t pin) {
        if (pin < 32) gpio_[GPCLR0 / 4] = 1 << pin;
        else gpio_[GPCLR1 / 4] = 1 << (pin - 32);
    }

    // Read pin level
    uint8_t lev(uint8_t pin) const {
        if (pin < 32) return (gpio_[GPLEV0 / 4] >> pin) & 1;
        return (gpio_[GPLEV1 / 4] >> (pin - 32)) & 1;
    }

    // Fast busy-wait delay (nanoseconds)
    static void delay_ns(uint32_t ns) {
        // Approximate: 1 CPU cycle ~ 1ns on 1GHz Pi
        // Use volatile loop to prevent optimization
        volatile uint32_t count = ns / 2;  // rough calibration
        while (count--) __asm__ volatile("nop");
    }

    // Microsecond delay using nanosleep
    static void delay_us(uint32_t us) {
        usleep(us);
    }

    // Millisecond delay
    static void delay_ms(uint32_t ms) {
        usleep(ms * 1000);
    }

private:
    int mem_fd_;
    void* gpio_map_;
    volatile uint32_t* gpio_;
};

// Global instance (initialized on first use)
inline GpioMmap& gpio() {
    static GpioMmap instance;
    return instance;
}

// Convenience macros matching bcm2835 API
#define GPIO_FSEL_IN  FSEL_IN
#define GPIO_FSEL_OUT FSEL_OUT

#define GPIO_SET(pin)    gpio().set(pin)
#define GPIO_CLR(pin)    gpio().clr(pin)
#define GPIO_LEV(pin)    gpio().lev(pin)
#define GPIO_FSEL(pin, f) gpio().fsel(pin, f)

#define DELAY_NS(ns)  GpioMmap::delay_ns(ns)
#define DELAY_US(us)  GpioMmap::delay_us(us)
#define DELAY_MS(ms)  GpioMmap::delay_ms(ms)