# Skill: SSD1963 Raspberry Pi — Código verificado contra datasheet

Conocimiento extraído del análisis del código C++ de este proyecto y su
verificación línea por línea contra `docs/SSD1963.pdf` (Solomon Systech
SSD1963 Rev 1.1, Ene 2010). El código de init está **correcto**; las
fórmulas y tiempos coinciden con el datasheet (detalles abajo).

## Arquitectura del proyecto (4 capas)

```
main.cpp (menú arcade, 7 escenas)
  → GameEngine (loop 30 FPS, BCM system timer, Scene*, FrameBuffer)
    → SSD1963 class (driver 8080 bit-bang vía libbcm2835)
      → GPIO 12-27 (D0-D15) + control (WR=3, RS=4, CS=5, RST=6, BL=0)
```

- `FrameBuffer` (RAM `uint16_t[480*272]`) → `flush()` → `SSD1963::draw_buffer()`.
  Todo el dibujo de escenas pasa por FrameBuffer; el driver solo al final.
- C++11, sin heap en el hot path (pools estáticos de bullets/enemies).
- Timing de frame: `bcm2835_st_read()` (system timer, µs), FRAME_TIME=33ms.

## Verificación contra el datasheet (Rev 1.1)

### PLL (0xE2 set_pll_mn + 0xE0 set_pll) — CORRECTO
- Fórmulas datasheet §9.68: `VCO = REF × (M+1)`, `PLL = VCO / (N+1)`,
  rango obligatorio `250 MHz < VCO < 800 MHz`.
- Código: M=49, N=4 con REF=10 MHz → VCO = 500 MHz ✓, PLL/SYSCLK = 100 MHz ✓.
- Secuencia §9.67: 0xE0←0x01 (enable), esperar ≥100 µs, 0xE0←0x03 (lock).
  Código espera 100 ms (holgado) y luego 50 ms extra. ✓
- ⚠ Antes del PLL el chip corre con el REF clock: "no programar registros
  más rápido que REF/2" (5 M words/s con REF=10 MHz). **Ésta es la razón
  de `slow_bus_ = true` durante `init()`** (~1 µs/strobe). No quitarlo.
- Desviación menor: el ejemplo del datasheet termina la secuencia PLL con
  un soft-reset (0x01) *después* del lock; el código lo hace *antes*.
  Funciona (método del port del PIC32), pero es una diferencia.

### set_lcd_mode (0xB0) — CORRECTO
- 7 parámetros: A1=0x10 (18-bit, FRC/dithering on), A2=0x00 (TFT mode),
  HDP=479 (panel = HDP+1 = 480 ✓), VDP=271 (272 ✓), A7=0x00 (RGB seq).
- Nota: 0x10 ⇒ A[4]=1 y A[3]=0 ⇒ habilita *dithering*, no FRC (el
  comentario del código dice "FRC"; es solo un comentario inexacto,
  la config es válida).

### Períodos H/V (0xB4 / 0xB6) — CORRECTO
- HT = 480+41+2+2 = 525 → se envía HT-1 (datasheet: total = HT+1) ✓
- VT = 272+10+2+2 = 286 → VT-1 ✓
- HPW=41 → se envía HPW-1 (pulse = HPW+1) ✓; VPW=10 → VPW-1 ✓
- HPS = HPW + back porch = 43; VPS = VPW + back porch = 12. El código
  envía HPS-1 / VPS-1 (convención del PIC32 de referencia). El datasheet
  dice "start position = HPS pixels" sin ±1 (ambiguo); la forma del PIC32
  funciona empíricamente — no cambiar sin medir con analizador lógico.

### Pixel clock (0xE6 set_lshift_freq) — CORRECTO
- Fórmula §9.72: `PCLK = PLL × (FPR+1) / 2^20`.
- FPR = 0x0170A2 = 94370 → PCLK = 100 MHz × 94371/1048576 ≈ **9.0 MHz** ✓
- Refresh = 9 MHz / (525×286) ≈ **59.9 Hz** ✓ (ideal para 480×272).

### Formato de píxeles (0xF0) — CORRECTO
- `SSD1963_PDI_16BIT565 = 0x03` = 16-bit 565. Cada escritura al bus = 1 píxel
  completo. **Bug histórico ya corregido**: enviar color + color>>8
  (2 escrituras/píxel) producía rayas/basura.
- `set_address_mode (0x36) = 0x00`: RGB, top→bottom, left→right, sin flips.

### GPIO / reset del panel (0xB8 / 0xBA) — CORRECTO
- `set_gpio_conf(0x0F, 0x01)`: GPIO[3:0] salidas host; **B0=1 ⇒ GPIO0 es
  GPIO normal** (no "panel power" de sleep mode) — así el pulsado manual
  de reset no choca con enter/exit_sleep_mode.
- Secuencia: GPIO0 alto → bajo → 1 ms → alto (reset del panel físico).

### Tiempos de soft reset / sleep — CORRECTO (con holgura)
- 0x01 soft_reset: datasheet pide esperar **≥5 ms**; código espera 20 ms ✓
- 0x11 exit_sleep: **≥5 ms**; código espera 20 ms. Además 0x11 auto-
  dispara 0x29 (display on) — por eso el 0x29 final es redundante e inofensivo.

### Bus 8080 (Tabla 13-6) — CORRECTO
- tPWLW (WR low) mín 12 ns, tDSW 4 ns. El bit-bang bcm2835 toma ~100 ns
  por escritura; el modo `SSD1963_MCU_WAVEFORM` usa 65 ns — ambos válidos.
- `write_data_bus()`: D0=GPIO12…D15=GPIO27, `data<<12` + set/clr_multi
  con caché `g_bus_state` (evita RMW innecesario). Mapeo contiguous ✓.

## Formato de archivos .rgb565 (¡dos variantes!)

El converter `script_tools/convert_png_to_rgb.py` produce:
```
[2B width BE][2B height BE][pixels RGB565 BE]  + alpha<128 → key 0x0001
```
- `FrameBuffer::draw_image_rgb565()` y `Sprite::load()` **leen el header**
  de 4 bytes. ✓ Son los usados por el engine.
- ⚠ `SSD1963::draw_image_rgb565()` (versión directa del driver) **NO lee
  header**: asume 480×272×2 bytes exactos. Con el formato actual del
  converter (que sí lleva header) desplazaría la imagen 2 píxeles.
  Es código legado sin uso actual; si se usa, pasarle solo el raw o
  arreglarlo para leer el header.
- Key color de transparencia: `0x0001` (= SPRITE_KEY_COLOR). Nunca es un
  color real RGB565 válido en la práctica.

## Reglas al modificar el driver/init

1. No tocar el orden de init sin consultar el datasheet §9 y §11
   (power-up: reset → soft reset → exit sleep → PLL → LCD mode →
   LSHIFT → períodos → panel reset GPIO → 0x29).
2. Mantener `slow_bus_=true` durante init (límite REF/2 antes del PLL).
3. Respeta los delays: soft reset/exit sleep ≥5 ms; PLL lock ≥100 µs.
4. Cualquier cambio de timing fino (HPS/VPS/±1, forma de onda RS) solo
   con evidencia de analizador lógico (ver `SSD1963_MCU_WAVEFORM` en
   `config_hw.hpp` y `script_tools/analizar_captura.py`).
5. Al añadir .cpp al build: agregarlo a `SRCS` del Makefile (usa
   `notdir`, todos los .cpp de src/ y src/engine/ van a obj/ plano).

## Comandos útiles

```bash
make                       # build local (requiere libbcm2835)
make remote-build          # compila en la Pi por SSH
make remote-run            # compila + ejecuta en la Pi (sudo)
python3 script_tools/convert_png_to_rgb.py in.png out.rgb565
pdftotext -layout docs/SSD1963.pdf /tmp/ds.txt   # texto del datasheet
```

## Datasheet: índice de secciones usadas

- §7.2 System Clock Generation (PLL, REF/2 programming limit)
- §9.2 soft_reset (0x01, espera 5 ms) — §9.8 exit_sleep (0x11)
- §9.16-9.18 set_column/page_address (0x2A/0x2B) + write_memory_start (0x2C)
- §9.24 set_address_mode (0x36)
- §9.33 set_lcd_mode (0xB0, HDP/VDP = valor+1)
- §9.35/9.37 set_hori/vert_period (0xB4/0xB6, HT/VT/HPW/VPW = valor+1)
- §9.39/9.41 set_gpio_conf/value (0xB8/0xBA)
- §9.67/9.68 set_pll/set_pll_mn (0xE0/0xE2, VCO 250-800 MHz)
- §9.72 set_lshift_freq (0xE6, PCLK = PLL×(FPR+1)/2^20)
- §9.74 set_pixel_data_interface (0xF0, 0x03 = 16-bit 565)
- §13.2.2 Table 13-6 timing 8080 (tPWLW 12 ns, tDSW 4 ns)
