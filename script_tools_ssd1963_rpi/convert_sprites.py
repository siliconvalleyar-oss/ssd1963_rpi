#!/usr/bin/env python3
"""
Batch convert all PNG sprites in assets/naves/ to .spr format.

.spr format:
  - 2 bytes: width  (uint16 LE)
  - 2 bytes: height (uint16 LE)
  - N bytes: RGB565 pixel data (uint16 LE, 2 bytes per pixel)
  - Transparency: pixels with alpha < 128 are encoded as 0x0001 (key color)
"""

import os, glob, struct
from PIL import Image, ImageFile

ImageFile.LOAD_TRUNCATED_IMAGES = True

SPRITES_DIR = "assets/naves"
ALPHA_THRESH = 128
KEY_COLOR   = 0x0001  # near-black, treated as transparent

def rgba_to_rgb565(r, g, b, a):
    if a < ALPHA_THRESH:
        return KEY_COLOR
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def convert_png_to_spr(png_path):
    basename = os.path.splitext(os.path.basename(png_path))[0]
    spr_path = os.path.join(SPRITES_DIR, basename + ".spr")

    img = Image.open(png_path).convert("RGBA")
    w, h = img.size
    pixels = img.load()

    with open(spr_path, "wb") as f:
        f.write(struct.pack("<HH", w, h))
        for y in range(h):
            for x in range(w):
                r, g, b, a = pixels[x, y]
                rgb565 = rgba_to_rgb565(r, g, b, a)
                f.write(struct.pack("<H", rgb565))

    print(f"  {basename:30s} {w:3d}x{h:3d}  -> {spr_path}")

def main():
    print(f"Converting PNGs in {SPRITES_DIR}/ ...")
    pngs = sorted(glob.glob(os.path.join(SPRITES_DIR, "*.png")))
    if not pngs:
        print("No PNG files found.")
        return
    for png in pngs:
        convert_png_to_spr(png)
    print(f"Done. {len(pngs)} sprites converted.")

if __name__ == "__main__":
    main()
