# firmware/drivers/drivers.mk — build wiring for the drivers module
# (fragment style per docs/rewrite/architecture.md §2; included by the root
# Makefile).
#
# Exactly ONE panel I/O backend is linked per image:
#   target firmware -> panel_io_rknanoc.c (MMIO contract, target TODOs)
#   host tools/tests -> panel_io_sim.c    (simulated panel, PPM dumps)

DRIVERS_DIR := firmware/drivers

DRIVERS_COMMON_SRCS := \
  $(DRIVERS_DIR)/display.c \
  $(DRIVERS_DIR)/font.c \
  $(DRIVERS_DIR)/font8x8.c

# Sources compiled for the ARM target image.
DRIVERS_TARGET_SRCS := \
  $(DRIVERS_COMMON_SRCS) \
  $(DRIVERS_DIR)/panel_io_rknanoc.c

# Sources compiled for host tests/tools.
DRIVERS_HOST_SRCS := \
  $(DRIVERS_COMMON_SRCS) \
  $(DRIVERS_DIR)/panel_io_sim.c

DRIVERS_INCLUDE_DIRS := $(DRIVERS_DIR)
