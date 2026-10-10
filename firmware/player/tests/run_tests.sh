#!/bin/sh
# Build and run the player host tests with a strict host compiler.
# Usage: ./run_tests.sh   (from anywhere; runs inside tests/)
set -e
cd "$(dirname "$0")"

CC="${CC:-cc}"
CFLAGS="${CFLAGS:--Wall -Werror -O1}"

mkdir -p .work

# Sources shared by the tests (the real player stack + mock codec).
PLAYER_SRCS="../player.c ../codec_wav.c ../codec_mp3_stub.c \
             ../codec_flac_stub.c ../ipc_b_mock.c"

$CC $CFLAGS -I.. -o .work/test_ringbuf   test_ringbuf.c ../ringbuf.c
$CC $CFLAGS -I.. -o .work/test_codec_wav test_codec_wav.c ../codec_wav.c \
    ../codec_io_stdio.c -lm
$CC $CFLAGS -I.. -o .work/test_player_state test_player_state.c \
    mock_codec.c $PLAYER_SRCS ../codec_io_stdio.c ../ringbuf.c
$CC $CFLAGS -I.. -o .work/test_ipc_mock  test_ipc_mock.c \
    mock_codec.c $PLAYER_SRCS ../codec_io_stdio.c ../ringbuf.c

.work/test_ringbuf
.work/test_codec_wav
.work/test_player_state
.work/test_ipc_mock
echo "ALL PLAYER TESTS PASSED"
