#!/usr/bin/env python3
"""
Convierte imagen PNG a raw RGB565 (480x272) para mostrar fullscreen en SSD1963.
Uso: python3 convert_png_to_rgb.py <input.png> [output.rgb565]
"""
import sys, os
from PIL import Image

def convert_to_rgb565(filename, output):
    img = Image.open(filename).convert('RGB')
    img = img.resize((480, 272))
    with open(output, 'wb') as f:
        for y in range(img.height):
            for x in range(img.width):
                r, g, b = img.getpixel((x, y))
                rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                f.write(rgb565.to_bytes(2, byteorder='big'))
    print(f"Converted {filename} -> {output} ({img.size[0]}x{img.size[1]})")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    src = sys.argv[1]
    dst = sys.argv[2] if len(sys.argv) > 2 else os.path.splitext(src)[0] + ".rgb565"
    convert_to_rgb565(src, dst)
