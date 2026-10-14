#pragma once
#include <cstdint>

// Comandos SSD1963
constexpr uint8_t SSD1963_NOP = 0x00;
constexpr uint8_t SSD1963_SOFT_RESET = 0x01;
constexpr uint8_t SSD1963_GET_PWR_MODE = 0x0A;
constexpr uint8_t SSD1963_GET_ADDR_MODE = 0x0B;
constexpr uint8_t SSD1963_GET_PIXEL_FORMAT = 0x0C;
constexpr uint8_t SSD1963_GET_DISPLAY_MODE = 0x0D;
constexpr uint8_t SSD1963_GET_SIGNAL_MODE = 0x0E;
constexpr uint8_t SSD1963_ENTER_SLEEP_MODE = 0x10;
constexpr uint8_t SSD1963_EXIT_SLEEP_MODE = 0x11;
constexpr uint8_t SSD1963_ENTER_PARTIAL_MODE = 0x12;
constexpr uint8_t SSD1963_ENTER_NORMAL_MODE = 0x13;
constexpr uint8_t SSD1963_EXIT_INVERT_MODE = 0x20;
constexpr uint8_t SSD1963_ENTER_INVERT_MODE = 0x21;
constexpr uint8_t SSD1963_SET_GAMMA = 0x26;
constexpr uint8_t SSD1963_BLANK_DISPLAY = 0x28;
constexpr uint8_t SSD1963_ON_DISPLAY = 0x29;
constexpr uint8_t SSD1963_SET_COLUMN_ADDRESS = 0x2A;
constexpr uint8_t SSD1963_SET_PAGE_ADDRESS = 0x2B;
constexpr uint8_t SSD1963_WRITE_MEMORY_START = 0x2C;
constexpr uint8_t SSD1963_READ_MEMORY_START = 0x2E;
constexpr uint8_t SSD1963_SET_PARTIAL_AREA = 0x30;
constexpr uint8_t SSD1963_SET_SCROLL_AREA = 0x33;
constexpr uint8_t SSD1963_SET_TEAR_OFF = 0x34;
constexpr uint8_t SSD1963_SET_TEAR_ON = 0x35;
constexpr uint8_t SSD1963_SET_ADDR_MODE = 0x36;
constexpr uint8_t SSD1963_SET_SCROLL_START = 0x37;
constexpr uint8_t SSD1963_EXIT_IDLE_MODE = 0x38;
constexpr uint8_t SSD1963_ENTER_IDLE_MODE = 0x39;
constexpr uint8_t SSD1963_SET_PIXEL_FORMAT = 0x3A;
constexpr uint8_t SSD1963_WRITE_MEMORY_CONTINUE = 0x3C;
constexpr uint8_t SSD1963_SET_TEAR_SCANLINE = 0x44;
constexpr uint8_t SSD1963_GET_SCANLINE = 0x45;
constexpr uint8_t SSD1963_SET_BACKLIGHT = 0x51;
constexpr uint8_t SSD1963_GET_BACKLIGHT = 0x52;
constexpr uint8_t SSD1963_SET_DISPLAY_MODE = 0x55;
constexpr uint8_t SSD1963_SET_CABC = 0x5E;
constexpr uint8_t SSD1963_GET_CABC = 0x5F;
constexpr uint8_t SSD1963_SET_PLL = 0xE0;
constexpr uint8_t SSD1963_SET_PLL_MN = 0xE2;
constexpr uint8_t SSD1963_GET_PLL_STATUS = 0xE4;
constexpr uint8_t SSD1963_SET_LCD_MODE = 0xB0;
constexpr uint8_t SSD1963_SET_HOR_PERIOD = 0xB4;
constexpr uint8_t SSD1963_SET_VER_PERIOD = 0xB6;
constexpr uint8_t SSD1963_SET_GPIO_CONF = 0xB8;
constexpr uint8_t SSD1963_SET_GPIO_VALUE = 0xBA;
constexpr uint8_t SSD1963_SET_POST_PROC = 0xBC;
constexpr uint8_t SSD1963_SET_PWM_FC = 0xBE;
constexpr uint8_t SSD1963_SET_DBC_CONF = 0xD0;
constexpr uint8_t SSD1963_SET_PIXEL_DATA_INTERFACE = 0xF0;
constexpr uint8_t SSD1963_SET_LSHIFT_FREQ = 0xE6;

// Modos de interfaz de píxeles
constexpr uint8_t SSD1963_PDI_8BIT = 0x00;
constexpr uint8_t SSD1963_PDI_16BIT565 = 0x03;
constexpr uint8_t SSD1963_PDI_16BIT555 = 0x01;
constexpr uint8_t SSD1963_PDI_18BIT666 = 0x04;
constexpr uint8_t SSD1963_PDI_18BIT565 = 0x05;
constexpr uint8_t SSD1963_PDI_24BIT888 = 0x06;