#!/usr/bin/env bash
# Build and run the PC preview. Needs g++, zlib, and the EPD47 headers that
# PlatformIO downloads on the first `pio run` (or `pio pkg install`).
#   tools/preview/build.sh            -> docs/preview.png
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
EPD="$ROOT/.pio/libdeps/t5-47/LilyGoEPD47/src"
OUT="$ROOT/tools/preview/out"
mkdir -p "$OUT" "$ROOT/docs"
g++ -std=c++17 -O2 -DPREVIEW_BUILD \
    -I"$ROOT/include" -I"$ROOT/src" -I"$ROOT/tools/preview/stub" -I"$EPD" \
    "$ROOT/tools/preview/preview.cpp" "$ROOT/tools/preview/host_epd.cpp" \
    "$ROOT/src/screen.cpp" "$ROOT/src/canvas.cpp" "$ROOT/src/icons.cpp" \
    -lz -o "$OUT/preview"
"$OUT/preview" "$OUT/preview.pgm"
python3 -c "from PIL import Image; Image.open('$OUT/preview.pgm').save('$ROOT/docs/preview.png')"
echo "docs/preview.png"
