# modules.mk — build fragment for the ReChord overlay module system
# (firmware/modules/).
#
# Included by the root Makefile (which owns the rules and stays thin):
#     include firmware/modules/modules.mk
#
# The loader and registry are freestanding C (no libc, no vendor headers),
# so the same objects serve the target image and the host tests. Host tests
# are NOT listed here as link inputs — they are run standalone via
#     sh firmware/modules/tests/run_tests.sh
# which compiles each test with `cc -Wall -Werror` and also performs the
# target compile check (`arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -c`).
#
# The offline packer (module_builder.py) is a build TOOL, not a source: the
# root Makefile invokes it with python3 when a real feature module image
# needs packing, e.g.
#     python3 firmware/modules/module_builder.py pack \
#         --elf build/mod_player.elf --id 44 -o build/m44.rmf1

MODULES_DIR := firmware/modules

# Module system sources (public API: module_format.h, module_loader.h,
# module_registry.h).
MODULES_SRCS := \
  $(MODULES_DIR)/module_loader.c \
  $(MODULES_DIR)/module_registry.c

# Host-side per-module tests (see tests/run_tests.sh).
MODULES_TEST_SRCS := \
  $(MODULES_DIR)/tests/test_loader.c \
  $(MODULES_DIR)/tests/test_registry.c \
  $(MODULES_DIR)/tests/test_builder.c

# Offline module packer (python3, stdlib only).
MODULES_BUILDER := $(MODULES_DIR)/module_builder.py

# No extra host libraries needed (loader/registry are libc-free).
MODULES_HOST_LIBS :=
