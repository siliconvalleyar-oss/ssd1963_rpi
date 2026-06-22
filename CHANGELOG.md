# Changelog

All notable changes to this project are documented here.

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
