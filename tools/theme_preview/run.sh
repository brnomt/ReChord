#!/bin/sh
# Render a theme package through the real skin engine and display driver.
# Usage: ./run.sh <themes-root> <package-id> <out-dir>
# Produces <out-dir>/*.ppm plus 1x/2x PNGs (PIL) next to them.
set -e
cd "$(dirname "$0")"
ROOT=${1:-../..}
ID=${2:?package id}
OUT=${3:?out dir}

CC="${CC:-cc}"
CFLAGS="${CFLAGS:--Wall -Werror -O2}"
mkdir -p "$OUT" .work
$CC $CFLAGS -I../../firmware/drivers -I../../firmware/theme \
    -o .work/render_theme render_theme.c \
    ../../firmware/drivers/display.c \
    ../../firmware/drivers/font.c \
    ../../firmware/drivers/font8x8.c \
    ../../firmware/drivers/panel_io_sim.c \
    ../../firmware/drivers/panel_init_rknanoc.c \
    ../../firmware/theme/theme.c \
    ../../firmware/theme/themes_builtin.c \
    ../../firmware/theme/theme_pkg.c \
    ../../firmware/theme/theme_file.c \
    ../../firmware/theme/theme_runtime.c \
    ../../firmware/theme/theme_assets.c \
    ../../firmware/theme/theme_skin.c -lm

.work/render_theme "$ROOT" "$ID" "$OUT"
python3 - "$OUT" <<'PY'
import glob, os, sys
from PIL import Image
out = sys.argv[1]
for p in sorted(glob.glob(os.path.join(out, '*.ppm'))):
    im = Image.open(p).convert('RGB')
    base = p[:-4]
    im.save(base + '.png')
    im.resize((im.width * 2, im.height * 2), Image.NEAREST).save(base + '_2x.png')
    os.remove(p)
print('rendered', len(glob.glob(os.path.join(out, '*.png'))), 'png')
PY
