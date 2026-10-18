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
# Link exactly ONE codec_io backend per image:
#   - codec_io_stdio.c : host tests + newlib target path (fopen)
#   - codec_io_fs.c    : target path over the fs.h abstraction (SD/eMMC)
# codec_io.c holds the shared dispatch wrappers, always linked. The root
# Makefile picks the backend via PLAYER_CODEC_IO_IMPL (default: stdio).
PLAYER_CODEC_IO_IMPL ?= $(PLAYER_DIR)/codec_io_stdio.c
#
# Host tests: firmware/player/tests/run_tests.sh (cc -Wall -Werror).
# Target: arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -c on every source
# below (verification standard, docs/rewrite/architecture.md §5).

PLAYER_DIR := firmware/player

PLAYER_IPC_IMPL ?= $(PLAYER_DIR)/ipc_b_target.c

# Module sources (public API: player.h, codec.h, ipc_b.h; ringbuf.h,
# codec_io.h, codecs/codec_registry.h are internal contracts).
# Codec layout (one file per codec, self-describing descriptors — see
# docs/rewrite/audio.md): codecs/{wav,mp3,flac}.c + codecs/codec_registry.c.
# Vendored decoder sources (minimp3/dr_flac) are header-included by their
# adapter TUs — they add no build entries here.
PLAYER_SRCS := \
  $(PLAYER_DIR)/player.c \
  $(PLAYER_DIR)/ringbuf.c \
  $(PLAYER_DIR)/playlist.c \
  $(PLAYER_DIR)/codec_io.c \
  $(PLAYER_CODEC_IO_IMPL) \
  $(PLAYER_DIR)/tags.c \
  $(PLAYER_DIR)/rg_math.c \
  $(PLAYER_DIR)/codecs/codec_registry.c \
  $(PLAYER_DIR)/codecs/wav.c \
  $(PLAYER_DIR)/codecs/mp3.c \
  $(PLAYER_DIR)/codecs/flac.c \
  $(PLAYER_IPC_IMPL)

# Host-side per-module tests (run via tests/run_tests.sh).
PLAYER_TEST_SRCS := \
  $(PLAYER_DIR)/tests/test_ringbuf.c \
  $(PLAYER_DIR)/tests/test_codec_wav.c \
  $(PLAYER_DIR)/tests/test_codec_io_fs.c \
  $(PLAYER_DIR)/tests/test_codec_mp3.c \
  $(PLAYER_DIR)/tests/test_codec_flac.c \
  $(PLAYER_DIR)/tests/test_player_state.c \
  $(PLAYER_DIR)/tests/test_ipc_mock.c \
  $(PLAYER_DIR)/tests/test_player_gapless.c \
  $(PLAYER_DIR)/tests/test_player_replaygain.c \
  $(PLAYER_DIR)/tests/test_rg_math.c \
  $(PLAYER_DIR)/tests/test_tags.c \
  $(PLAYER_DIR)/tests/mock_codec.c

# Extra host-test libraries (WAV golden + rg_math cross-checks use sin/pow).
PLAYER_HOST_LIBS := -lm
