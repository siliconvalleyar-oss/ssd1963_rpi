# Changelog

All notable changes to this project are documented here.

## v5.4.1 — 2026-07-12

### Fixed
- SSD1963 driver timing: added `BUS_WAIT` to `write_pixel_burst()` for stable WR pulses.
- Fragmented `clear_screen()` into 32-row batches with inter-batch delays to prevent display corruption on long bursts.
- Added explicit `EXIT_SLEEP_MODE` after soft reset in `init()`.
- Increased post-`ON_DISPLAY` delay to 100 ms before backlight ON.
- Added 1 ms stabilization delay in `set_area()` before GRAM writes.
- Fixed LCD_FPR reference in docs to match `config_hw.hpp` (`0x13332`, ~9 MHz).

### Changed
- Improved initialization robustness for 480x272 RGB565 panels.

## v5.4.0 — 2026-07-10

### Added
- Game engine loop (~30 FPS) with `FrameBuffer` (RAM-backed) and scene lifecycle.
- `MenuScene` with auto-cycling selection.
- `SpaceShooterScene` mini arcade game (player, bullets, enemies, collisions, HUD).
- `SpriteViewerScene` for browsing `.rgb565` sprite assets.
- `PatternScene` with color bars, gradient, checkerboard, grid, and rainbow.
- `FontDemoScene` showcasing scalable 8x8 bitmap font.
- `ImageViewerScene` and `ColorBlocksScene`.

### Fixed
- Split display flush into per-row bursts to avoid corruption on long transfers.

## v5.3.x — 2026-06-22

### Added
- `FrameBuffer` class, scene abstraction (`Scene` base class).
- Sprite loading with transparency key color (`0x0001`).
- Optimized sprite rendering with segment-based horizontal clipping.

### Fixed
- Reverted PLL charge pump and LCD mode parameters to match PIC32 reference.
- Restored original PCLK `0x13332` for 12 MHz oscillator → 120 MHz PLL.
- Re-added `EXIT_SLEEP_MODE` and stabilized 8080 bus timing.

## v5.0.x — 2026-06-15

### Added
- Multi-scene architecture (`GameEngine`, `Scene`, `FrameBuffer`).
- Direct color test sequence in `main.cpp`.
- Bitmap font rendering (8x8) with scalable variants.
- Remote build and run via `make remote-build` / `make remote-run`.

### Changed
- Migrated all drawing operations to use `FrameBuffer` + single flush per frame.
- Reorganized `include/` and `src/engine/` directories.

## v4.0.0 — 2026-06-12

### Added
- Full documentation suite: ARCHITECTURE, API, HARDWARE, DEVELOPMENT, RULES, DIAGRAMS.
- Draw.io architecture and flow diagrams.
- `script_tools/compilacion_remota.sh` for automated SSH deployment.

### Changed
- Adopted semantic versioning and tag workflow defined in `docs/RULES.md`.
- Established `main` branch as stable release branch.

## v3.0.x — 2026-06-11

### Added
- `Sprite` class with `.rgb565` file loading.
- `sprite_list.hpp` helper for populating sprite viewer scenes.
- Space shooter assets (`assets/naves/`).

### Fixed
- Corrected GPIO pin mapping and `DATA_PINS_MASK` generation.

## v2.0.0 — 2026-06-10

### Added
- 8080 parallel bus protocol implementation (`write_command`, `write_data`, `write_data_bus`).
- Burst write mode (`write_pixel_burst_start/end`) for faster fills.
- `draw_pixels()`, `draw_buffer()`, `draw_image_rgb565()`.
- 8x8 bitmap font (`font_8x8.hpp`) and text rendering primitives.

### Changed
- Consolidated all SSD1963 commands into `ssd1963_cmd.hpp`.
- Introduced `RGB565` struct and color constants in `color.hpp`.

## v1.0.3 — 2026-06-10

### Added
- Comprehensive project documentation: README, Makefile help, skill definition, install script.
- Remote build and run Makefile targets for SSH-based cross-development.
- Image conversion pipeline documentation.

### Changed
- Improved Makefile with `remote-build` and `remote-run` targets.
- Restructured project layout for clarity.

## v1.0.2 — 2026-06-10

### Added
- Full project file structure with source, headers, assets, and tools.
- SSD1963 driver with 8080 parallel bus GPIO bit-banging.
- Color block demo (7 colors) and full-screen photo display.
- PNG to RGB565 conversion script (Python/PIL).

### Changed
- Consolidated all project files into organized directory structure.

## v1.0.1 — 2026-06-10

### Added
- Initial SSD1963 driver for Raspberry Pi.
- Basic display initialization and pixel drawing.
- GPIO bit-bang implementation via libbcm2835.
- Display of RGB565 image from raw binary file.
