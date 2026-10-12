#!/bin/sh
# Build and run the config module host tests with a strict host compiler.
# Usage: ./run_tests.sh   (from anywhere; runs inside tests/)
set -e
cd "$(dirname "$0")"

CC="${CC:-cc}"
CFLAGS="${CFLAGS:--Wall -Werror -O1}"

mkdir -p .work

$CC $CFLAGS -I.. -I../../services -o .work/test_config \
    test_config.c ../rechord_config.c ../../services/fs_host.c

.work/test_config
echo "ALL CONFIG TESTS PASSED"
