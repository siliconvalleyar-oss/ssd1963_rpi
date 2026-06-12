# Driver Optimization Guide

## The Problem

The original SSD1963 driver wrote 16-bit data to the parallel bus using individual GPIO calls:

```cpp
// OLD: 32 GPIO calls per pixel
void write_data_bus(uint16_t data) {
    for (int i = 0; i < 16; i++) {
        bcm2835_gpio_write(D0 + i, (data >> i) & 1);
    }
}
```

For a 480×272 display, that's **480 × 272 × 32 = 4,177,920 GPIO calls** per screen fill.

## The Solution: Bitmask Operations

Using `bcm2835_gpio_clr_multi()` and `bcm2835_gpio_set_multi()`, we reduced this to **2 GPIO calls** per pixel:

```cpp
// NEW: 2 GPIO calls per pixel
#define DATA_PIN_MASK 0x0FFF0000  // GPIO 12-27

void write_data_bus(uint16_t data) {
    bcm2835_gpio_clr_multi(DATA_PIN_MASK);          // Clear all 16 data pins
    bcm2835_gpio_set_multi(((uint32_t)data << 12) & DATA_PIN_MASK);  // Set data bits
}
```

Total GPIO calls for a full screen: **480 × 272 × 2 = 261,120** — a **16× reduction**.

## Batch Writes

For filling large areas, we further optimize by keeping RS high and CS low across multiple writes:

```
Standard write (per pixel):
  RS_HIGH, CS_LOW, set_data, WR_LOW, WR_HIGH, CS_HIGH  → 6 operations

Batch write (per pixel):
  set_data, WR_LOW, WR_HIGH                              → 3 operations
```

`write_batch_start()` sets RS and CS once. `write_batch_end()` releases CS.

## WR Strobe

The WR strobe uses individual GPIO writes (not mask operations) because it's a single pin (GPIO 3):

```cpp
WR_LOW();   // bcm2835_gpio_write(GPIO_3, LOW)
WR_HIGH();  // bcm2835_gpio_write(GPIO_3, HIGH)
```

No explicit delay is needed — the memory-mapped GPIO write itself takes ~10ns, which meets the SSD1963's minimum WR low pulse requirement.

## Pixel Count Optimization

### `clear_screen()` Bug Fix

The original implementation wrote 2 values per pixel (high byte + low byte), a leftover from 8-bit bus mode:

```cpp
// BUG: writes 2× pixels (8-bit legacy)
for (i = 0; i < WIDTH * HEIGHT; i++) {
    write_data(color);      // 16-bit write (but called twice)
    write_data(color >> 8); // second write shifts pixel data
}
```

**Fix:** Single 16-bit write per pixel.

```cpp
// FIX: 1 write per pixel
for (i = 0; i < WIDTH * HEIGHT; i++) {
    write_batch_data(color);
}
```

Performance gain: **2× fewer writes** for block fills.

## Performance Benchmarks (estimated)

| Operation              | Original   | Optimized | Gain   |
|------------------------|------------|-----------|--------|
| Full screen fill       | ~800 ms    | ~50 ms    | 16×    |
| 100×100 block fill     | ~35 ms     | ~2 ms     | 17×    |
| Draw 100px horizontal line | ~3 ms | ~0.2 ms   | 15×    |
| Image display (RGB565) | ~900 ms    | ~60 ms    | 15×    |

*Actual performance depends on Pi model, CPU frequency, and memory speed.*

## Additional Optimizations

### Line Drawing

Horizontal and vertical lines use `set_area()` + batch writes instead of per-pixel calls:

```cpp
void draw_line_h(uint16_t x, uint16_t y, uint16_t len, uint16_t color) {
    set_area(x, y, x + len - 1, y);  // single row
    write_command(WRITE_MEMORY_START);
    write_batch_start();
    for (int i = 0; i < len; i++) write_batch_data(color);
    write_batch_end();
}
```

This is dramatically faster than calling `draw_pixel()` in a loop.

### Image Display

`draw_image_rgb565()` streams raw data from file into the display using batch writes, avoiding any per-pixel overhead beyond the file read and bus write.

## Further Improvements (Future Work)

1. **DMA transfer** — Use BCM2835's DMA engine to transfer pixel data without CPU involvement
2. **Double buffering** — Allocate a framebuffer in main memory for atomic screen updates
3. **SPI flash** — Store images in external SPI flash for faster access
4. **FPGA/CPLD** — Offload the parallel bus protocol to hardware for true 60 FPS
5. **Multi-threading** — Separate rendering from GPIO communication
