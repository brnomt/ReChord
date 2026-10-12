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

# The shared codec_io dispatch wrappers now live in codec_io.c; both backends
# (codec_io_stdio.c for host tests, codec_io_fs.c for the on-device FS path)
# are built as a selectable backend. Host tests link the stdio backend +
# codec_io.c; the FS-backend test links codec_io_fs.c + fs_host.c + codec_io.c.
IO_DISPATCH=../codec_io.c

# ringbuf: standalone (no codec_io).
$CC $CFLAGS -I.. -o .work/test_ringbuf   test_ringbuf.c ../ringbuf.c

# WAV golden test: stdio backend + dispatch wrappers + newlib.
$CC $CFLAGS -I.. -o .work/test_codec_wav test_codec_wav.c ../codec_wav.c \
    ../codec_io_stdio.c $IO_DISPATCH -lm

# Player state machine + IPC end-to-end: stdio backend + dispatch wrappers.
$CC $CFLAGS -I.. -o .work/test_player_state test_player_state.c \
    mock_codec.c $PLAYER_SRCS ../codec_io_stdio.c $IO_DISPATCH ../ringbuf.c \
    ../rg_math.c -lm
$CC $CFLAGS -I.. -o .work/test_ipc_mock  test_ipc_mock.c \
    mock_codec.c $PLAYER_SRCS ../codec_io_stdio.c $IO_DISPATCH ../ringbuf.c \
    ../rg_math.c -lm

# codec_io_fs: the target FS backend exercised through fs_host on the host.
# codec_io_fs.c + codec_io.c (+ fs_host.c backing fs.h) decode a WAV and are
# checked byte-for-byte against the analytical sine.
$CC $CFLAGS -I.. -I../../services -o .work/test_codec_io_fs \
    test_codec_io_fs.c ../codec_wav.c ../codec_io_fs.c \
    ../codec_io.c ../../services/fs_host.c -lm

# Gapless: leading/trailing trim + seamless same-format auto-advance
# (no B-core restart), via the mock codec.
$CC $CFLAGS -I.. -o .work/test_player_gapless \
    test_player_gapless.c mock_codec.c $PLAYER_SRCS ../ringbuf.c \
    ../codec_io_stdio.c ../codec_io.c ../rg_math.c -lm

# ReplayGain fixed-point helpers.
$CC $CFLAGS -I.. -o .work/test_rg_math \
    test_rg_math.c ../rg_math.c -lm

# ReplayGain end-to-end through the player state machine + IPC mock sink.
$CC $CFLAGS -I.. -o .work/test_player_replaygain \
    test_player_replaygain.c mock_codec.c $PLAYER_SRCS ../ringbuf.c \
    ../codec_io_stdio.c ../codec_io.c ../rg_math.c -lm

# ReplayGain tag parsers (ID3v2 TXXX + Vorbis comments) via stdio codec_io.
$CC $CFLAGS -I.. -o .work/test_tags \
    test_tags.c ../tags.c ../codec_io_stdio.c ../codec_io.c

.work/test_ringbuf
.work/test_codec_wav
.work/test_player_state
.work/test_ipc_mock
.work/test_codec_io_fs
.work/test_player_gapless
.work/test_rg_math
.work/test_player_replaygain
.work/test_tags
echo "ALL PLAYER TESTS PASSED"
