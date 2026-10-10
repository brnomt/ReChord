#!/bin/sh
# Build and run the ReChord display/font host tests with plain `cc`.
#
# Usage: ./run_tests.sh
# Env:   CC=...                compiler (default: cc)
#        RECHORD_TEST_OUT=dir  where to put binaries + PPM artifact
#                              (default: fresh mktemp -d)
set -e

cd "$(dirname "$0")"
CC="${CC:-cc}"
CFLAGS="-std=c99 -Wall -Werror"
OUT="${RECHORD_TEST_OUT:-$(mktemp -d)}"
mkdir -p "$OUT"

$CC $CFLAGS -I.. -o "$OUT/test_display" \
    test_display.c ../display.c ../font.c ../font8x8.c ../panel_io_sim.c
$CC $CFLAGS -I.. -o "$OUT/test_font_ppm" \
    test_font_ppm.c ../display.c ../font.c ../font8x8.c ../panel_io_sim.c

"$OUT/test_display"
"$OUT/test_font_ppm" "$OUT/font_render.ppm"

echo "all host tests passed (PPM artifact: $OUT/font_render.ppm)"
