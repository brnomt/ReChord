#!/bin/sh
# Build and run the services host tests with a strict host compiler.
# Usage: ./run_tests.sh   (from anywhere; runs inside tests/)
set -e
cd "$(dirname "$0")"

CC="${CC:-cc}"
CFLAGS="${CFLAGS:--Wall -Werror -O1}"

mkdir -p .work

$CC $CFLAGS -I.. -o .work/test_settings test_settings.c ../settings.c ../fs_host.c
$CC $CFLAGS -I.. -o .work/test_log     test_log.c     ../log.c     ../fs_host.c
$CC $CFLAGS -I.. -o .work/test_input   test_input.c   ../input.c
$CC $CFLAGS -I.. -o .work/test_fs      test_fs.c      ../fs_host.c

.work/test_settings
.work/test_log
.work/test_input
.work/test_fs
echo "ALL SERVICES TESTS PASSED"
