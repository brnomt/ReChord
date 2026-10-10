# player.mk — build fragment for the ReChord player workstream
# (firmware/player/: playback state machine, codecs, dual-core IPC).
#
# Included by the root Makefile (which owns the rules and stays thin):
#     include firmware/player/player.mk
#
# Link exactly ONE ipc_b implementation per image:
#   - ipc_b_mock.c   : host tests / on-device bring-up against the sim
#   - ipc_b_target.c : real hardware (shared block + clock negotiation;
#                      register-level bring-up TODO in docs/rewrite/player.md)
# The root Makefile picks via PLAYER_IPC_IMPL (default: target).
#
# codec_io_stdio.c is the stdio backend for codec file I/O; on the target
# it rides newlib syscalls until the storage workstream lands (see the
# header comment there).
#
# Host tests: firmware/player/tests/run_tests.sh (cc -Wall -Werror).
# Target: arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -c on every source
# below (verification standard, docs/rewrite/architecture.md §5).

PLAYER_DIR := firmware/player

PLAYER_IPC_IMPL ?= $(PLAYER_DIR)/ipc_b_target.c

# Module sources (public API: player.h, codec.h, ipc_b.h; ringbuf.h and
# codec_io.h are internal contracts shared inside the module).
PLAYER_SRCS := \
  $(PLAYER_DIR)/player.c \
  $(PLAYER_DIR)/ringbuf.c \
  $(PLAYER_DIR)/codec_io_stdio.c \
  $(PLAYER_DIR)/codec_wav.c \
  $(PLAYER_DIR)/codec_mp3_stub.c \
  $(PLAYER_DIR)/codec_flac_stub.c \
  $(PLAYER_IPC_IMPL)

# Host-side per-module tests (run via tests/run_tests.sh).
PLAYER_TEST_SRCS := \
  $(PLAYER_DIR)/tests/test_ringbuf.c \
  $(PLAYER_DIR)/tests/test_codec_wav.c \
  $(PLAYER_DIR)/tests/test_player_state.c \
  $(PLAYER_DIR)/tests/test_ipc_mock.c \
  $(PLAYER_DIR)/tests/mock_codec.c

# Extra host-test libraries (the WAV golden test cross-checks with sin()).
PLAYER_HOST_LIBS := -lm

# When minimp3/dr_flac are vendored in (docs/rewrite/player.md §"Codec
# vendoring"), their sources join PLAYER_SRCS from:
#   $(PLAYER_DIR)/codecs/minimp3/  $(PLAYER_DIR)/codecs/dr_flac/
