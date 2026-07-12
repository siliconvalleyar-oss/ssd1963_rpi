# Diagramas del Proyecto

> Generados desde `diagrams/*.drawio`. Editar los `.drawio` y regenerar este documento.

---

## 1. Arquitectura del Software (`architecture.drawio`)

```
┌─────────────────────────────────────────────────────────────────────────┐
│  APPLICATION LAYER                                                      │
│  ┌────────────────────────────────────────────────────────────────┐     │
│  │  main.cpp — Entry point, engine bootstrap                      │     │
│  ├────────────────────────────────────────────────────────────────┤     │
│  │  Scenes: Menu, ColorBlocks, ImageViewer, Paint, Patterns       │     │
│  └────────────────────────────────────────────────────────────────┘     │
└──────────────────────────────┬──────────────────────────────────────────┘
                               │  (direct call)
┌──────────────────────────────▼──────────────────────────────────────────┐
│  ENGINE LAYER                                                           │
│  ┌────────────────────────────────────────────────────────────────┐     │
│  │  GameEngine — Main loop, state machine, input processing       │     │
│  ├────────────────────────────────────────────────────────────────┤     │
│  │  Scene (abstract base) — on_enter/on_exit/update/draw/...     │     │
│  ├────────────────────────────────────────────────────────────────┤     │
│  │  MenuScene — Menu rendering, navigation, scroll                │     │
│  └────────────────────────────────────────────────────────────────┘     │
└──────────────────────────────┬──────────────────────────────────────────┘
                               │  uses
┌──────────────────────────────▼──────────────────────────────────────────┐
│  DRIVER LAYER (SSD1963)                                                 │
│  ┌────────────────────────────────────────────────────────────────┐     │
│  │  init() — PLL, LCD mode, timings                              │     │
│  ├────────────────────────────────────────────────────────────────┤     │
│  │  draw_block, draw_pixel, draw_rect, fill_rect, clear_screen    │     │
│  ├────────────────────────────────────────────────────────────────┤     │
│  │  draw_char, draw_string, draw_string_centered (font 8x8)      │     │
│  ├────────────────────────────────────────────────────────────────┤     │
│  │  draw_image_rgb565 — file → display                            │     │
│  ├────────────────────────────────────────────────────────────────┤     │
│  │  write_command / write_data / write_data_bus                   │     │
│  │  8080 parallel protocol (RS, WR, CS)                           │     │
│  └────────────────────────────────────────────────────────────────┘     │
└──────────────────────────────┬──────────────────────────────────────────┘
                               │  GPIO calls
┌──────────────────────────────▼──────────────────────────────────────────┐
│  HARDWARE ABSTRACTION (libbcm2835)                                      │
│  ┌────────────────────────────────────────────────────────────────┐     │
│  │  GPIO memory-mapped I/O — bcm2835_gpio_fsel, gpio_write       │     │
│  ├────────────────────────────────────────────────────────────────┤     │
│  │  bcm2835_delay, bcm2835_st_read (timing)                      │     │
│  └────────────────────────────────────────────────────────────────┘     │
└──────────────────────────────┬──────────────────────────────────────────┘
                               │  controls
┌──────────────────────────────▼──────────────────────────────────────────┐
│  PHYSICAL HARDWARE                                                      │
│  ┌────────────────────────────────────────────────────────────────┐     │
│  │  Raspberry Pi (Zero 2W / Pi 4)                                │     │
│  ├────────────────────────────────────────────────────────────────┤     │
│  │  SSD1963 TFT LCD — 480x272, RGB565, 16-bit parallel            │     │
│  └────────────────────────────────────────────────────────────────┘     │
└─────────────────────────────────────────────────────────────────────────┘

  CONFIGURATION (compile-time includes ── dashed)
  ┌─────────────────────────────────────────────┐
  │  config_hw.hpp — GPIO pins, timings, dims   │
  │  ssd1963_cmd.hpp — ~100 command opcodes      │
  │  color.hpp — RGB565 macro, 8 colors         │
  │  assets/*.rgb565 — Imágenes pre-convertidas  │
  │  Makefile — Build system + remote targets    │
  └─────────────────────────────────────────────┘

Legend:
  Solid   = direct call / use
  Dashed  = compile-time include
```

---

## 2. Flujo del Programa (`flow.drawio`)

```
                    ┌───────────────┐
                    │   main()      │
                    └───────┬───────┘
                            │
                    ┌───────▼───────┐
                    │  engine.init()│
                    │  bcm2835_init │
                    │  setup_gpio   │
                    │  display.init │
                    └───────┬───────┘
                            │
                    ┌───────▼───────┐
                    │ Create Scenes │
                    │ Menu, Blocks  │
                    │ ImageViewer.. │
                    └───────┬───────┘
                            │
                    ┌───────▼───────┐
                    │ set_scene(    │
                    │   &main_menu) │
                    │ on_enter()    │
                    └───────┬───────┘
                            │
                    ┌───────▼───────┐
                    │  engine.run() │
                    │  MAIN LOOP    │
                    └───┬───────┬───┘
              ┌─────────┤       ├──────────┐
              │         │       │          │
      ┌───────▼───┐ ┌──▼────┐ ┌─▼────────┐ │
      │process_   │ │update │ │  draw()   │ │
      │input()    │ │ (dt)  │ │          │ │
      │read_btn()→│ │       │ │          │ │
      │handle_btn │ │       │ │          │ │
      └───────┬───┘ └──┬────┘ └─────┬────┘ │
              │         │           │      │
              └─────────┴───────────┘      │
                                    │      │
                            ┌───────▼────┐ │
                            │ Frame done │◄┘
                            │ ~33ms/30fps│
                            └──┬──────┬──┘
                    loop back ◄┘      └► ┌───────────────┐
                                         │ engine.quit() │
                                         │ bcm2835_close │
                                         └───────────────┘

  ── Scene Switch (from handle_button) ──

                    ┌──────────────────┐
                    │  Scene Switch?   │◄────── handle_button
                    │  CHANGE action   │        (from main loop)
                    └────────┬─────────┘
                             │
                    ┌────────▼─────────┐
                    │ current_scene.   │
                    │   on_exit()      │
                    │ set_scene(target)│
                    │ new_scene.       │
                    │   on_enter()     │
                    └────────┬─────────┘
                             │
                    loop back┼─────────────────► main loop

  ── Menu Action ──

                    ┌──────────────────┐
                    │ Menu: SELECT     │◄────── MenuScene.handle_button
                    │ set_scene(target)│
                    └────────┬─────────┘
                             │
                             └─────────────────► scene_on_exit

Timing Notes:
  - Frame timing: ~33ms (30 FPS)
  - GPIO buttons checked every frame
  - Auto-cycle: 3s per scene (Screensaver mode)
```
