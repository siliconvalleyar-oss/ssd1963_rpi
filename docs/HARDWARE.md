# Hardware Setup

## Required Components

| Component         | Description                                     |
|-------------------|-------------------------------------------------|
| Raspberry Pi      | Zero 2W or Pi 4 (tested); any model with GPIO   |
| TFT LCD           | SSD1963 controller, 480x272, 16-bit parallel    |
| Power supply      | 5V/2.5A+ for Pi + display                       |
| Jumper wires      | 21 female-to-female (M-F) or male-to-female     |
| Level shifter     | Optional: 3.3V↔5V if display requires 5V logic  |

## Pinout: Raspberry Pi ↔ SSD1963

### Data Bus (16-bit)

| SSD1963 Pin | BCM GPIO | Pi Header Pin |
|-------------|----------|---------------|
| D0          | 12       | GPIO 12       |
| D1          | 13       | GPIO 13       |
| D2          | 14       | GPIO 14       |
| D3          | 15       | GPIO 15       |
| D4          | 16       | GPIO 16       |
| D5          | 17       | GPIO 17       |
| D6          | 18       | GPIO 18       |
| D7          | 19       | GPIO 19       |
| D8          | 20       | GPIO 20       |
| D9          | 21       | GPIO 21       |
| D10         | 22       | GPIO 22       |
| D11         | 23       | GPIO 23       |
| D12         | 24       | GPIO 24       |
| D13         | 25       | GPIO 25       |
| D14         | 26       | GPIO 26       |
| D15         | 27       | GPIO 27       |

### Control Signals

| SSD1963 Pin | Function        | BCM GPIO | Pi Header Pin |
|-------------|-----------------|----------|---------------|
| WR          | Write strobe    | 3        | GPIO 3        |
| RS (DC)     | Data/Command    | 4        | GPIO 4        |
| CS          | Chip Select     | 5        | GPIO 5        |
| RESET       | Hardware Reset  | 6        | GPIO 6        |
| BACKLIGHT   | Backlight PWM   | 0        | GPIO 0        |

### Power

| SSD1963 Pin | Connect To                 |
|-------------|----------------------------|
| VCC         | 3.3V or 5V (check module) |
| GND         | Pi GND                     |
| BL_EN       | GPIO 0 (or 3.3V for always on) |

## Wiring Diagram

```
Raspberry Pi                    SSD1963 TFT
================                ===========
GPIO 12  ────────────────────── D0
GPIO 13  ────────────────────── D1
GPIO 14  ────────────────────── D2
GPIO 15  ────────────────────── D3
GPIO 16  ────────────────────── D4
GPIO 17  ────────────────────── D5
GPIO 18  ────────────────────── D6
GPIO 19  ────────────────────── D7
GPIO 20  ────────────────────── D8
GPIO 21  ────────────────────── D9
GPIO 22  ────────────────────── D10
GPIO 23  ────────────────────── D11
GPIO 24  ────────────────────── D12
GPIO 25  ────────────────────── D13
GPIO 26  ────────────────────── D14
GPIO 27  ────────────────────── D15

GPIO 3   ────────────────────── WR
GPIO 4   ────────────────────── RS (DC)
GPIO 5   ────────────────────── CS
GPIO 6   ────────────────────── RESET

GPIO 0   ────────────────────── BACKLIGHT (or BL_EN)

3.3V     ────────────────────── VCC
GND      ────────────────────── GND
```

## Voltage Level Considerations

- **Raspberry Pi GPIOs**: 3.3V logic levels.
- **SSD1963 modules**: Many operate at 3.3V, but some require 5V.
- If your module is 5V, use a bi-directional level shifter for all 21 lines, or use a module with 3.3V compatibility.
- Check your specific TFT module datasheet before connecting.

## Connection Checklist

Before powering on:

- [ ] All 16 data lines (D0-D15) connected to correct GPIOs
- [ ] WR connected to GPIO 3
- [ ] RS (DC) connected to GPIO 4
- [ ] CS connected to GPIO 5
- [ ] RESET connected to GPIO 6
- [ ] Backlight enable connected to GPIO 0 (or 3.3V)
- [ ] Power and GND correctly wired
- [ ] No crossed wires (verify pin-by-pin)
- [ ] Level shifters in place if needed

## Raspberry Pi Pin Reference

### Raspberry Pi 4

```
        3.3V (1) (2) 5V
   GPIO 2 (3) (4) 5V
   GPIO 3 (5) (6) GND
   GPIO 4 (7) (8) GPIO 14
        GND (9) (10) GPIO 15
  GPIO 17 (11) (12) GPIO 18
  GPIO 27 (13) (14) GND
  GPIO 22 (15) (16) GPIO 23
        3.3V (17) (18) GPIO 24
  GPIO 10 (19) (20) GND
   GPIO 9 (21) (22) GPIO 25
  GPIO 11 (23) (24) GPIO 8
        GND (25) (26) GPIO 7
   GPIO 0 (27) (28) GPIO 1
   GPIO 5 (29) (30) GND
   GPIO 6 (31) (32) GPIO 12
  GPIO 13 (33) (34) GND
  GPIO 19 (35) (36) GPIO 16
  GPIO 26 (37) (38) GPIO 20
        GND (39) (40) GPIO 21
```

### Raspberry Pi Zero 2W

Same 40-pin GPIO layout as Pi 4, same pin mapping.

## Troubleshooting Hardware

| Symptom                | Likely Cause                        | Fix                              |
|------------------------|-------------------------------------|----------------------------------|
| No display output      | Backlight off or not connected      | Check GPIO 0, set BACKLIGHT HIGH |
| White screen           | Reset held low or not initialized   | Check RESET pin connection       |
| Garbage on screen      | Data lines swapped or wrong timing  | Verify D0-D15 wiring order       |
| Flickering display     | Power insufficient                  | Use 2.5A+ power supply           |
| Wrong colors           | RGB byte order swapped              | Check endianness in code         |
| Display stays blank    | CS/WR not toggling                  | Verify control signal wiring     |
