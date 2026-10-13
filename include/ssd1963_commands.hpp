#pragma once

#include <cstdint>

// SSD1963 command opcodes
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
constexpr uint8_t SSD1963_READ_MEMORY_CONTINUE = 0x3E;
constexpr uint8_t SSD1963_SET_TEAR_SCANLINE = 0x44;
constexpr uint8_t SSD1963_GET_SCANLINE = 0x45;
constexpr uint8_t SSD1963_RD_DDB_START = 0xA1;
constexpr uint8_t SSD1963_RD_DDB_AUTO = 0xA8;
constexpr uint8_t SSD1963_SET_PANEL_MODE = 0xB0;
constexpr uint8_t SSD1963_GET_PANEL_MODE = 0xB1;
constexpr uint8_t SSD1963_SET_HOR_PERIOD = 0xB4;
constexpr uint8_t SSD1963_GET_HOR_PERIOD = 0xB5;
constexpr uint8_t SSD1963_SET_VER_PERIOD = 0xB6;
constexpr uint8_t SSD1963_GET_VER_PERIOD = 0xB7;
constexpr uint8_t SSD1963_SET_GPIO_CONF = 0xB8;
constexpr uint8_t SSD1963_GET_GPIO_CONF = 0xB9;
constexpr uint8_t SSD1963_SET_GPIO_VAL = 0xBA;
constexpr uint8_t SSD1963_GET_GPIO_STATUS = 0xBB;
constexpr uint8_t SSD1963_SET_POST_PROC = 0xBC;
constexpr uint8_t SSD1963_GET_POST_PROC = 0xBD;
constexpr uint8_t SSD1963_SET_PWM_CONF = 0xBE;
constexpr uint8_t SSD1963_GET_PWM_CONF = 0xBF;
constexpr uint8_t SSD1963_GET_LCD_GEN0 = 0xC0;
constexpr uint8_t SSD1963_SET_LCD_GEN0 = 0xC1;
constexpr uint8_t SSD1963_GET_LCD_GEN1 = 0xC2;
constexpr uint8_t SSD1963_SET_LCD_GEN1 = 0xC3;
constexpr uint8_t SSD1963_GET_LCD_GEN2 = 0xC4;
constexpr uint8_t SSD1963_SET_LCD_GEN2 = 0xC5;
constexpr uint8_t SSD1963_GET_LCD_GEN3 = 0xC6;
constexpr uint8_t SSD1963_SET_LCD_GEN3 = 0xC7;
constexpr uint8_t SSD1963_SET_GPIO0_ROP = 0xC8;
constexpr uint8_t SSD1963_GET_GPIO0_ROP = 0xC9;
constexpr uint8_t SSD1963_SET_GPIO1_ROP = 0xCA;
constexpr uint8_t SSD1963_GET_GPIO1_ROP = 0xCB;
constexpr uint8_t SSD1963_SET_GPIO2_ROP = 0xCC;
constexpr uint8_t SSD1963_GET_GPIO2_ROP = 0xCD;
constexpr uint8_t SSD1963_SET_GPIO3_ROP = 0xCE;
constexpr uint8_t SSD1963_GET_GPIO3_ROP = 0xCF;
constexpr uint8_t SSD1963_SET_ABC_DBC_CONF = 0xD0;
constexpr uint8_t SSD1963_GET_ABC_DBC_CONF = 0xD1;
constexpr uint8_t SSD1963_SET_DBC_HISTO_PTR = 0xD2;
constexpr uint8_t SSD1963_GET_DBC_HISTO_PTR = 0xD3;
constexpr uint8_t SSD1963_SET_DBC_THRES = 0xD4;
constexpr uint8_t SSD1963_GET_DBC_THRES = 0xD5;
constexpr uint8_t SSD1963_SET_ABM_TMR = 0xD6;
constexpr uint8_t SSD1963_GET_ABM_TMR = 0xD7;
constexpr uint8_t SSD1963_SET_AMB_LVL0 = 0xD8;
constexpr uint8_t SSD1963_GET_AMB_LVL0 = 0xD9;
constexpr uint8_t SSD1963_SET_AMB_LVL1 = 0xDA;
constexpr uint8_t SSD1963_GET_AMB_LVL1 = 0xDB;
constexpr uint8_t SSD1963_SET_AMB_LVL2 = 0xDC;
constexpr uint8_t SSD1963_GET_AMB_LVL2 = 0xDD;
constexpr uint8_t SSD1963_SET_AMB_LVL3 = 0xDE;
constexpr uint8_t SSD1963_GET_AMB_LVL3 = 0xDF;
constexpr uint8_t SSD1963_PLL_START = 0xE0;
constexpr uint8_t SSD1963_PLL_STOP = 0xE1;
constexpr uint8_t SSD1963_SET_PLL_MN = 0xE2;
constexpr uint8_t SSD1963_GET_PLL_MN = 0xE3;
constexpr uint8_t SSD1963_GET_PLL_STATUS = 0xE4;
constexpr uint8_t SSD1963_ENT_DEEP_SLEEP = 0xE5;
constexpr uint8_t SSD1963_SET_PCLK = 0xE6;
constexpr uint8_t SSD1963_GET_PCLK = 0xE7;
constexpr uint8_t SSD1963_SET_DATA_INTERFACE = 0xF0;
constexpr uint8_t SSD1963_GET_DATA_INTERFACE = 0xF1;

// Pixel Data Interface modes
constexpr uint8_t SSD1963_PDI_8BIT = 0x00;
constexpr uint8_t SSD1963_PDI_12BIT = 0x01;
constexpr uint8_t SSD1963_PDI_16BIT = 0x02;
constexpr uint8_t SSD1963_PDI_16BIT565 = 0x03;
constexpr uint8_t SSD1963_PDI_18BIT = 0x04;
constexpr uint8_t SSD1963_PDI_24BIT = 0x05;
constexpr uint8_t SSD1963_PDI_9BIT = 0x06;

// Aliases used in existing code
constexpr uint8_t SSD1963_SET_LCD_MODE = SSD1963_SET_PANEL_MODE;
constexpr uint8_t SSD1963_SET_LSHIFT_FREQ = SSD1963_SET_PCLK;
constexpr uint8_t SSD1963_SET_PIXEL_DATA_INTERFACE = SSD1963_SET_DATA_INTERFACE;
constexpr uint8_t SSD1963_SET_PLL = SSD1963_PLL_START;
constexpr uint8_t SSD1963_SET_BACKLIGHT = 0x51;
constexpr uint8_t SSD1963_GET_BACKLIGHT = 0x52;
constexpr uint8_t SSD1963_SET_DISPLAY_MODE = 0x55;
constexpr uint8_t SSD1963_SET_CABC = 0x5E;
constexpr uint8_t SSD1963_GET_CABC = 0x5F;
constexpr uint8_t SSD1963_SET_PWM_FC = SSD1963_SET_PWM_CONF;
constexpr uint8_t SSD1963_SET_DBC_CONF = SSD1963_SET_ABC_DBC_CONF;

// Pixel Data Interface mode aliases
constexpr uint8_t SSD1963_PDI_16BIT555 = SSD1963_PDI_12BIT;
constexpr uint8_t SSD1963_PDI_18BIT666 = SSD1963_PDI_18BIT;
constexpr uint8_t SSD1963_PDI_18BIT565 = SSD1963_PDI_18BIT;
constexpr uint8_t SSD1963_PDI_24BIT888 = SSD1963_PDI_24BIT;

//}//SSD1963_DRV