# dsp.mk — build fragment for the ReChord DSP effect-module system
# (firmware/dsp/).
#
# Included by the root Makefile (which owns the rules and stays thin):
#     include firmware/dsp/dsp.mk
#
# Note on rechord_dsp_core.c: it is NOT listed here. Each core-backed effect
# (mod_eq, mod_bass) compiles its own private instantiation of the core's
# biquad engine via `#include "../rechord_dsp_core.c"` (entry points renamed
# per TU) so every effect instance carries its own filter state — see the
# header comment in mod_eq.c. The root Makefile's own rechord_dsp_core.o is
# unaffected (symbols are renamed inside the effect objects).
#
# Host tests: cc -Wall -Werror (see docs/rewrite/customization.md for the
# exact per-test compile lines); target: arm-none-eabi-gcc -mcpu=cortex-m3
# -mthumb -Os -c on every source below.

DSP_DIR := firmware/dsp

# Module sources (public API dsp_module.h + dsp_chain.h; dsp_sat.h internal).
DSP_SRCS := \
  $(DSP_DIR)/dsp_chain.c \
  $(DSP_DIR)/mod_eq.c \
  $(DSP_DIR)/mod_bass.c \
  $(DSP_DIR)/mod_volume.c

# Host-side per-module tests.
DSP_TEST_SRCS := \
  $(DSP_DIR)/tests/test_mod_eq.c \
  $(DSP_DIR)/tests/test_mod_bass.c \
  $(DSP_DIR)/tests/test_mod_volume.c \
  $(DSP_DIR)/tests/test_dsp_chain.c

# Extra libraries the host tests need (the biquad math uses libm).
DSP_HOST_LIBS := -lm
