#!/bin/sh
# Build and run the player host tests with a strict host compiler.
# Usage: ./run_tests.sh   (from anywhere; runs inside tests/)
set -e
cd "$(dirname "$0")"

CC="${CC:-cc}"
CFLAGS="${CFLAGS:--Wall -Werror -O1 -I.. -I../..}"

mkdir -p .work

# Sources shared by the tests (the real player stack + codec registry +
# the three codecs + mock codec). minimp3/dr_flac come in through the
# adapter TUs' header includes (no separate build entries).
PLAYER_SRCS="../player.c ../codecs/codec_registry.c ../codecs/wav.c \
             ../codecs/mp3.c ../codecs/flac.c ../tags.c ../ipc_b_mock.c \
             ../bfs.c ../playlist.c"

# The shared codec_io dispatch wrappers now live in codec_io.c; both backends
# (codec_io_stdio.c for host tests, codec_io_fs.c for the on-device FS path)
# are built as a selectable backend. Host tests link the stdio backend +
# codec_io.c; the FS-backend test links codec_io_fs.c + fs_host.c + codec_io.c.
IO_DISPATCH=../codec_io.c

# ringbuf: standalone (no codec_io).
$CC $CFLAGS -I.. -o .work/test_ringbuf   test_ringbuf.c ../ringbuf.c

# playlist: pure logic (order, repeat, shuffle) — standalone.
$CC $CFLAGS -I.. -o .work/test_playlist  test_playlist.c ../playlist.c

# Shared codec/update RAM region (flac.c + mp3.c now claim their buffers
# from services/workbuf.c instead of file-statics).
WORKBUF=../../services/workbuf.c

# WAV golden test: stdio backend + dispatch wrappers + newlib.
$CC $CFLAGS -I.. -o .work/test_codec_wav test_codec_wav.c ../codecs/wav.c \
    ../codec_io_stdio.c $IO_DISPATCH -lm

# Player state machine + IPC end-to-end: stdio backend + dispatch wrappers.
$CC $CFLAGS -I.. -o .work/test_player_state test_player_state.c \
    mock_codec.c $PLAYER_SRCS ../codec_io_stdio.c $IO_DISPATCH ../ringbuf.c \
    ../rg_math.c $WORKBUF -lm
$CC $CFLAGS -I.. -o .work/test_ipc_mock  test_ipc_mock.c \
    mock_codec.c $PLAYER_SRCS ../codec_io_stdio.c $IO_DISPATCH ../ringbuf.c \
    ../rg_math.c $WORKBUF -lm

# codec_io_fs: the target FS backend exercised through fs_host on the host.
# codec_io_fs.c + codec_io.c (+ fs_host.c backing fs.h) decode a WAV and are
# checked byte-for-byte against the analytical sine.
$CC $CFLAGS -I.. -I../../services -o .work/test_codec_io_fs \
    test_codec_io_fs.c ../codecs/wav.c ../codec_io_fs.c \
    ../codec_io.c ../../services/fs_host.c -lm

# Gapless: leading/trailing trim + seamless same-format auto-advance
# (no B-core restart), via the mock codec.
$CC $CFLAGS -I.. -o .work/test_player_gapless \
    test_player_gapless.c mock_codec.c $PLAYER_SRCS ../ringbuf.c \
    ../codec_io_stdio.c ../codec_io.c ../rg_math.c $WORKBUF -lm

# Unplayable file in a playlist: pre-open must be non-fatal and the boundary
# must skip to the next entry (BRNOMT-106 field report).
$CC $CFLAGS -I.. -o .work/test_player_skip \
    test_player_skip.c mock_codec.c $PLAYER_SRCS ../ringbuf.c \
    ../codec_io_stdio.c ../codec_io.c ../rg_math.c $WORKBUF -lm

# ReplayGain fixed-point helpers.
$CC $CFLAGS -I.. -o .work/test_rg_math \
    test_rg_math.c ../rg_math.c -lm

# ReplayGain end-to-end through the player state machine + IPC mock sink.
$CC $CFLAGS -I.. -o .work/test_player_replaygain \
    test_player_replaygain.c mock_codec.c $PLAYER_SRCS ../ringbuf.c \
    ../codec_io_stdio.c ../codec_io.c ../rg_math.c $WORKBUF -lm

# ReplayGain tag parsers (ID3v2 TXXX + Vorbis comments) via stdio codec_io.
$CC $CFLAGS -I.. -o .work/test_tags \
    test_tags.c ../tags.c ../codec_io_stdio.c ../codec_io.c

# MP3 adapter: crafted ID3v2 + Xing/Info + LAME + silence frames.
$CC $CFLAGS -I.. -o .work/test_codec_mp3 \
    test_codec_mp3.c ../codecs/mp3.c ../tags.c \
    ../codec_io_stdio.c $IO_DISPATCH $WORKBUF -lm

# Same fixture, AP metadata-only build (no decoder): the open/format/duration/
# gapless/tags/meta contract must hold and decode() must say UNSUPPORTED.
$CC $CFLAGS -I.. -DRECHORD_AP_METADATA_ONLY -o .work/test_codec_mp3_meta \
    test_codec_mp3.c ../codecs/mp3.c ../tags.c \
    ../codec_io_stdio.c $IO_DISPATCH $WORKBUF -lm

# FLAC adapter: synthesized VERBATIM-subframe FLAC (exact golden) + RG tags.
$CC $CFLAGS -I.. -o .work/test_codec_flac \
    test_codec_flac.c ../codecs/flac.c ../tags.c \
    ../codec_io_stdio.c $IO_DISPATCH $WORKBUF -lm

# EQ curve transport (BRNOMT-76): band packing + the AP->B seqlock.
$CC $CFLAGS -o .work/test_eq_ipc test_eq_ipc.c

# Audio HAL pure pieces: slot packing + the per-rate table constants
# (regression guard for the 2x-speed framing bug class, DFIU 75271e5f).
$CC $CFLAGS -I.. -I../.. -o .work/test_ao_slots \
    test_ao_slots.c ../../audio/ao_rate_table.c

.work/test_ringbuf
.work/test_playlist
.work/test_codec_wav
.work/test_player_state
.work/test_ipc_mock
.work/test_codec_io_fs
.work/test_player_gapless
.work/test_player_skip
.work/test_rg_math
.work/test_player_replaygain
.work/test_tags
.work/test_codec_mp3
.work/test_codec_mp3_meta
.work/test_codec_flac
.work/test_eq_ipc
.work/test_ao_slots
echo "ALL PLAYER TESTS PASSED"
