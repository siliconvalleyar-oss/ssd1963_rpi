# SSD1963 Hardware Wiring Guide

## Pin Mapping (Raspberry Pi → SSD1963)

| Raspberry Pi GPIO | Function | SSD1963 Pin       | Notes                |
|-------------------|----------|-------------------|----------------------|
| 0                 | BACKLIGHT| BL_EN             | PWM-capable          |
| 3                 | WR       | WR# (pin 120)     | Write strobe         |
| 4                 | RS       | RS (pin 123)      | 0=command, 1=data    |
| 5                 | CS       | CS# (pin 123)     | Chip select          |
| 6                 | RESET    | RST# (pin 128)    | Hardware reset       |
| 7                 | BTN_UP   | —                 | External pull-up     |
| 8                 | BTN_DOWN | —                 | External pull-up     |
| 9                 | BTN_SEL  | —                 | External pull-up     |
| 10                | BTN_BACK | —                 | External pull-up     |
| 12                | D0       | DB0               | Data bus LSB         |
| 13                | D1       | DB1               |                      |
| 14                | D2       | DB2               |                      |
| 15                | D3       | DB3               |                      |
| 16                | D4       | DB4               |                      |
| 17                | D5       | DB5               |                      |
| 18                | D6       | DB6               |                      |
| 19                | D7       | DB7               |                      |
| 20                | D8       | DB8               |                      |
| 21                | D9       | DB9               |                      |
| 22                | D10      | DB10              |                      |
| 23                | D11      | DB11              |                      |
| 24                | D12      | DB12              |                      |
| 25                | D13      | DB13              |                      |
| 26                | D14      | DB14              |                      |
| 27                | D15      | DB15              | Data bus MSB         |

## Power

| Pin     | Voltage | Source           |
|---------|---------|------------------|
| VCC     | 3.3V    | Pi pin 1 or 17   |
| GND     | 0V      | Pi pin 6, 9, 14  |
| BL_EN   | 3.3V    | GPIO 0 (or 3.3V) |
| VCC_5V  | 5V      | (if needed)      |

## Interface Mode

The SSD1963 is configured in **8080-series 16-bit parallel mode**:

- Pin 128 (SMD config): HIGH = 8080 mode, LOW = 6800 mode
- Data bus: 16-bit (D0-D15)
- Control: RS (command/data), WR (write strobe), CS (chip select)
- RD pin can be left floating (no reads performed)

## Button Wiring

```
GPIO 7 ──┬── 10kΩ ── 3.3V
         └── Button ── GND

GPIO 8 ──┬── 10kΩ ── 3.3V
         └── Button ── GND

GPIO 9 ──┬── 10kΩ ── 3.3V
         └── Button ── GND

GPIO 10 ─┬── 10kΩ ── 3.3V
         └── Button ── GND
```

Buttons connect GPIO to GND when pressed (active low). Internal pull-ups are enabled in software.

## Timing Diagram (8080 Write Cycle)

```
CS#    ──\__________________________/──
RS     ────────\___________/───────────
DATA   ────────<── valid ──>───────────
WR#    ────────\___/───────────────────
```

Minimum WR low pulse: ~10ns (GPIO speed is sufficient)

## Display Dimensions

- Active area: 480 × 272 pixels
- Pixel format: RGB565 (16-bit, 5R+6G+5B)
- Typical pixel clock: 2–10 MHz

## Level Shifting

All GPIO signals from Raspberry Pi are 3.3V. The SSD1963 is 3.3V tolerant on control/data lines. No level shifting required if the display module is 3.3V compatible.

## Troubleshooting

| Symptom                 | Likely Cause                  | Fix                               |
|-------------------------|-------------------------------|-----------------------------------|
| Blank screen            | Backlight off                 | Check BL_EN pin/GPIO 0            |
| No image, backlight on  | Reset stuck                   | Check RST# pin (GPIO 6)           |
| Corrupted colors        | Data bus wiring               | Verify D0-D15 connections         |
| Flickering              | Timing wrong                  | Adjust LCD_FPR in config_hw.hpp   |
| No response to buttons  | Pull-up resistors missing     | Add 10kΩ to 3.3V or use internal PUD |
| Ghosting/smearing       | Pixel clock too fast          | Decrease LCD_FPR                  |
