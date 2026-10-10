# startup.mk — build fragment for the boot/startup core (firmware/startup/).
#
# Included by the root Makefile (which owns the rules and stays thin):
#     include firmware/startup/startup.mk
#
# Sources:
#   startup.c — RKnanoFW image header + the clean-room C-runtime startup
#               (boot contract: IRQs off -> boot params -> .data -> .bss
#               -> DSB/ISB -> rechord_main; docs/re/route-b-minimum.md §2).
#   vectors.c — Cortex-M3 core vector table (SP 0x03004000, Reset ->
#               rechord_startup_entry), 256-byte aligned, .vectors section.
#
# NOT listed here (legacy wiring owned by the root Makefile):
#   ap_startup.c — reset handler for the AP (fw1) image only.
#
# Target compile per source (architecture.md §5):
#   arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -c -Ifirmware <src>
#
# The startup has no host tests by contract: it runs before the C runtime
# exists and is verified by cross-compilation plus on-device boot logs
# (docs/rewrite/core.md).

STARTUP_DIR := firmware/startup

STARTUP_SRCS := \
  $(STARTUP_DIR)/startup.c \
  $(STARTUP_DIR)/vectors.c

# Include roots every fragment consumer needs (-I flags):
#   firmware/ — for "app/boot_params.h" style includes.
STARTUP_INCLUDE_DIRS := firmware

# Startup code is ARM-only glue (naked entry, barrier instructions); it is
# compiled for the target only, never into host test binaries.
STARTUP_TARGET_ONLY := yes
