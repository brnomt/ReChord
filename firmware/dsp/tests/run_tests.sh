#!/bin/sh
# run_tests.sh — host test driver for firmware/dsp.
#
# 1. Compiles every effect/chain test with `cc -Wall -Werror` and runs it.
# 2. Target compile check: arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -c
#    on every DSP source (skipped with a note if the toolchain is absent).
#
# Usage: sh firmware/dsp/tests/run_tests.sh
set -eu

cd "$(dirname "$0")"          # -> firmware/dsp/tests
DSP_DIR=..
CORE="$DSP_DIR/../rechord_dsp_core.c"
BUILD=$(mktemp -d)
trap 'rm -rf "$BUILD"' EXIT

CC=${CC:-cc}
CFLAGS="-Wall -Werror"
LIBS="-lm"

echo "== compile host tests =="
$CC $CFLAGS -o "$BUILD/test_mod_eq"     test_mod_eq.c     "$DSP_DIR/mod_eq.c"     "$CORE" $LIBS
$CC $CFLAGS -o "$BUILD/test_mod_bass"   test_mod_bass.c   "$DSP_DIR/mod_bass.c"   "$CORE" $LIBS
$CC $CFLAGS -o "$BUILD/test_mod_volume" test_mod_volume.c "$DSP_DIR/mod_volume.c"          $LIBS
$CC $CFLAGS -o "$BUILD/test_dsp_chain"  test_dsp_chain.c  "$DSP_DIR/dsp_chain.c" \
    "$DSP_DIR/mod_eq.c" "$DSP_DIR/mod_bass.c" "$DSP_DIR/mod_volume.c" "$CORE" $LIBS
$CC $CFLAGS -o "$BUILD/test_eq_bank"    test_eq_bank.c    "$DSP_DIR/eq_bank.c"            $LIBS

echo "== run test_mod_eq =="
"$BUILD/test_mod_eq"
echo "== run test_mod_bass =="
"$BUILD/test_mod_bass"
echo "== run test_mod_volume =="
"$BUILD/test_mod_volume"
echo "== run test_dsp_chain =="
"$BUILD/test_dsp_chain"
echo "== run test_eq_bank =="
"$BUILD/test_eq_bank"

if command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    echo "== target compile check (arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os) =="
    for src in dsp_chain.c mod_eq.c mod_bass.c mod_volume.c eq_bank.c; do
        arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -Wall -Werror \
            -Ifirmware -c "$DSP_DIR/$src" -o "$BUILD/$(basename "$src" .c).o"
    done
else
    echo "arm-none-eabi-gcc not found: target checks SKIPPED"
fi

echo "ALL DSP TESTS PASSED"
