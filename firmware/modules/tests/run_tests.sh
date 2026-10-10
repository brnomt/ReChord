#!/bin/sh
# run_tests.sh — host test driver for firmware/modules (RMF1).
#
# 1. Compiles every test with `cc -Wall -Werror` and runs them.
# 2. Builder round trip: packs a raw blob and an ELF with
#    module_builder.py, then loads both through the C loader
#    (test_builder checks cross-implementation CRC/header agreement).
# 3. Target compile check: arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -c
#    on the module sources (skipped with a note if the toolchain is absent).
#
# Usage: sh firmware/modules/tests/run_tests.sh
set -eu

cd "$(dirname "$0")"          # -> firmware/modules/tests
MODULES_DIR=..
BUILD=$(mktemp -d)
trap 'rm -rf "$BUILD"' EXIT

CC=${CC:-cc}
PY=${PYTHON:-python3}
CFLAGS="-Wall -Werror"

echo "== compile host tests =="
$CC $CFLAGS -o "$BUILD/test_loader"   test_loader.c   "$MODULES_DIR/module_loader.c"
$CC $CFLAGS -o "$BUILD/test_registry" test_registry.c "$MODULES_DIR/module_loader.c" "$MODULES_DIR/module_registry.c"
$CC $CFLAGS -o "$BUILD/test_builder"  test_builder.c  "$MODULES_DIR/module_loader.c"

echo "== run test_loader =="
"$BUILD/test_loader"

echo "== run test_registry =="
"$BUILD/test_registry"

echo "== builder round trip (raw blob) =="
# 2980-byte pattern matching the C tests: byte i = (i*7 + 3) & 0xFF.
$PY -c 'import sys; sys.stdout.buffer.write(bytes((i*7+3)&0xFF for i in range(2980)))' > "$BUILD/code.bin"
$PY "$MODULES_DIR/module_builder.py" pack --code "$BUILD/code.bin" \
    --id 44 --entry 0 --bss-len 20220 -o "$BUILD/m44.rmf1"
$PY "$MODULES_DIR/module_builder.py" inspect "$BUILD/m44.rmf1"
"$BUILD/test_builder" "$BUILD/m44.rmf1"

if command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    echo "== target compile check (arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os) =="
    arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -Wall -Werror \
        -c "$MODULES_DIR/module_loader.c" -o "$BUILD/module_loader.o"
    arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -Wall -Werror \
        -c "$MODULES_DIR/module_registry.c" -o "$BUILD/module_registry.o"

    echo "== builder round trip (ELF via --elf) =="
    arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -nostdlib \
        -Wl,-e,rmf_fixture_entry -o "$BUILD/fixture.elf" fixture_module.c
    $PY "$MODULES_DIR/module_builder.py" pack --elf "$BUILD/fixture.elf" \
        --id 45 -o "$BUILD/m45.rmf1"
    $PY "$MODULES_DIR/module_builder.py" inspect "$BUILD/m45.rmf1"
    "$BUILD/test_builder" "$BUILD/m45.rmf1"
else
    echo "arm-none-eabi-gcc not found: target checks SKIPPED"
fi

echo "ALL MODULES TESTS PASSED"
