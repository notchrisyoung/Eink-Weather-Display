#!/usr/bin/env bash
# Build and run the PC preview. Needs g++, zlib, and the EPD47 headers that
# PlatformIO downloads on the first `pio run` (or `pio pkg install`).
#   tools/preview/build.sh            -> docs/screenshots/*.png
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
EPD="$ROOT/.pio/libdeps/t5-47/LilyGoEPD47/src"
OUT="$ROOT/tools/preview/out"
mkdir -p "$OUT"
g++ -std=c++17 -O2 -DPREVIEW_BUILD \
    -I"$ROOT/include" -I"$ROOT/src" -I"$ROOT/tools/preview/stub" -I"$EPD" \
    "$ROOT/tools/preview/preview.cpp" "$ROOT/tools/preview/host_epd.cpp" \
    "$ROOT/src/screen.cpp" "$ROOT/src/canvas.cpp" "$ROOT/src/icons.cpp" \
    -lz -o "$OUT/preview"
# One screenshot per state shown in the README
SHOTS="$ROOT/docs/screenshots"
mkdir -p "$SHOTS"
render() {   # name, flags...
    local name="$1"; shift
    "$OUT/preview" "$OUT/$name.pgm" "$@" > /dev/null
    python3 -c "from PIL import Image; Image.open('$OUT/$name.pgm').save('$SHOTS/$name.png')"
    echo "docs/screenshots/$name.png"
}
render dashboard
render night --night --no-probe
render offline --offline
render waiting --message
