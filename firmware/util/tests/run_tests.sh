#!/bin/sh
# Build and run the rechord_log module host tests with a strict host compiler.
# Usage: ./run_tests.sh   (from anywhere; runs inside tests/)
set -e
cd "$(dirname "$0")"

CC="${CC:-cc}"
CFLAGS="${CFLAGS:--Wall -Werror -O1}"

mkdir -p .work

$CC $CFLAGS -I.. -I../../services -o .work/test_log \
    test_log.c ../rechord_log.c ../../services/fs_host.c

.work/test_log
echo "ALL LOG TESTS PASSED"
