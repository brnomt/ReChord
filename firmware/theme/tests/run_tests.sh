#!/bin/sh
# Build and run the theme module host tests (palette, layout DSL, runtime).
# Usage: ./run_tests.sh   (from anywhere; runs inside tests/)
set -e
cd "$(dirname "$0")"

CC="${CC:-cc}"
CFLAGS="${CFLAGS:--Wall -Werror -O1}"
mkdir -p .work

$CC $CFLAGS -I.. -o .work/test_theme \
    test_theme.c ../theme.c ../themes_builtin.c ../theme_file.c
.work/test_theme

$CC $CFLAGS -I.. -o .work/test_theme_pkg \
    test_theme_pkg.c ../theme_pkg.c
.work/test_theme_pkg

$CC $CFLAGS -I.. -o .work/test_theme_runtime \
    test_theme_runtime.c ../theme.c ../themes_builtin.c ../theme_file.c \
    ../theme_pkg.c ../theme_runtime.c ../theme_assets.c ../theme_skin.c
.work/test_theme_runtime

echo "ALL THEME TESTS PASSED"
