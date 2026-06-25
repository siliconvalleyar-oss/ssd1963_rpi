#!/usr/bin/env python3
"""
Convierte imagen PNG a .rgb565 unificado con header (w+h BE) + pixels BE.

Formato .rgb565:
  - 2 bytes: width  (uint16 big-endian)
  - 2 bytes: height (uint16 big-endian)
  - N bytes: RGB565 pixel data (2 bytes per pixel, big-endian)
  - Transparencia: píxeles con alpha < 128 se codifican como 0x0001

Uso:
  python3 convert_png_to_rgb.py <input.png> [output.rgb565]
"""
import sys, os
from PIL import Image, ImageFile
ImageFile.LOAD_TRUNCATED_IMAGES = True

ALPHA_THRESH = 128
KEY_COLOR    = 0x0001

def rgba_to_rgb565(r, g, b, a=255):
    if a < ALPHA_THRESH:
        return KEY_COLOR
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def convert_to_rgb565(filename, output):
    img = Image.open(filename)
    if img.mode == 'RGBA':
        px = img.load()
        w, h = img.size
        with open(output, 'wb') as f:
            f.write(w.to_bytes(2, 'big'))
            f.write(h.to_bytes(2, 'big'))
            for y in range(h):
                for x in range(w):
                    r, g, b, a = px[x, y]
                    f.write(rgba_to_rgb565(r, g, b, a).to_bytes(2, 'big'))
    else:
        img_rgb = img.convert('RGB')
        w, h = img_rgb.size
        with open(output, 'wb') as f:
            f.write(w.to_bytes(2, 'big'))
            f.write(h.to_bytes(2, 'big'))
            for y in range(h):
                for x in range(w):
                    r, g, b = img_rgb.getpixel((x, y))
                    rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                    f.write(rgb565.to_bytes(2, 'big'))
    print(f"Converted {filename} -> {output} ({w}x{h})")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    src = sys.argv[1]
    dst = sys.argv[2] if len(sys.argv) > 2 else os.path.splitext(src)[0] + ".rgb565"
    convert_to_rgb565(src, dst)
