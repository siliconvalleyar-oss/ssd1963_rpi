#!/usr/bin/env python3
"""
Generate sprite list header for SpriteViewerScene.
Scans assets/naves/*.spr and produces sprite_list.hpp.
"""
import os, glob

SPR_DIR = "assets/naves"
OUTPUT  = "include/engine/sprite_list.hpp"

pairs = []
for f in sorted(glob.glob(os.path.join(SPR_DIR, "*.spr"))):
    basename = os.path.splitext(os.path.basename(f))[0]
    label = basename.replace("_", " ").title()
    pairs.append((f, label))

with open(OUTPUT, "w") as out:
    out.write("#pragma once\n")
    out.write("#include <engine/SpriteViewerScene.hpp>\n\n")
    out.write("static void populate_sprites(SpriteViewerScene& scene) {\n")
    for path, label in pairs:
        out.write(f'    scene.add_sprite("{path}", "{label}");\n')
    out.write("}\n")

print(f"Generated {OUTPUT} with {len(pairs)} sprites.")
