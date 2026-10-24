# ReChord — Echo Mini custom firmware build system
#
# Target: ARM Cortex-M3 (Thumb-2), Rockchip RKnanoC
#
# Clean-room product line only. `make` builds the standalone AP5M image from
# tracked sources:
#   AP (fw1)       — our UI, filesystem, drivers, services, player, DSP
#   BB (Core B)    — our decoder + codecs, embedded in the AP as a blob
#
# It does NOT read stock/ images, community/ SDK blobs or proprietary .lib
# archives. The legacy SDK/stock packing targets were removed in BRNOMT-82;
# tools/ keeps the historical packers for reference only.

ifeq ($(OS),Windows_NT)
    PYTHON ?= py -3
else
    PYTHON ?= python3
endif

# Portable dir create / remove (the previous recipes shelled out to PowerShell,
# which broke builds on Linux/macOS). Python is already a required dependency.
MKDIR_P := $(PYTHON) -c "import os,sys; os.makedirs(sys.argv[1], exist_ok=True)"
RMDIR_R := $(PYTHON) -c "import shutil,sys; shutil.rmtree(sys.argv[1], ignore_errors=True)"

CROSS_COMPILE ?= arm-none-eabi-
CC      := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy
SIZE    := $(CROSS_COMPILE)size

.DEFAULT_GOAL := standalone-ap5m

BUILD_DIR    := build

# ReChord module fragments (docs/rewrite/architecture.md §2): one subsystem,
# one folder, one .mk. Fragments only define *_SRCS/*_INCLUDE_DIRS variables;
# build targets stay in this file so the graph lives in one place.
# Header-change tracking for the product pipeline (missing build/cfw once left
# a stale 64 KiB ipc_b_target.o in a flashable image).
-include $(shell find build/cfw -name "*.d" 2>/dev/null)

include firmware/ap.mk

# ---- CFW AP (fw1) clean-room image ---------------------------------------
# This links ONLY firmware/{startup,ui,drivers,services,theme,libc,player,dsp}
# plus our Core B decoder blob. No SDK AP/BB objects, no vendor .lib.
CFW_BUILD_DIR ?= build/cfw
CFW_OBJS := $(patsubst %.c,$(CFW_BUILD_DIR)/objs/%.o,$(AP_SRCS))
CFW_ELF := $(CFW_BUILD_DIR)/rechord_ap.elf

# ---- Core B heartbeat image (BRNOMT-68 phase 1) --------------------------
# Flat image linked at HIGHRAM0 0x01000000, embedded in the AP as a rodata
# blob and copied by ap_core_b_start(). See docs/re/core-b-bringup.md.
BB_CORE_DIR  := $(CFW_BUILD_DIR)/bbcore
BB_CORE_ELF  := $(BB_CORE_DIR)/bb_core.elf
BB_CORE_BIN  := $(BB_CORE_DIR)/bb_core.bin
BB_CORE_BLOB := $(BB_CORE_DIR)/bb_core_blob.o

# Phase 3c: Core B decodes. It links our codecs + registry-free descriptors +
# tags + the shared workbuf (decode pool) + codec_io over the A file ring.
BB_INC := -Ifirmware -Ifirmware/libc -Ifirmware/services -Ifirmware/player \
          -Ifirmware/drivers -Ifirmware/dsp

BB_CORE_SRCS := \
  firmware/bb/bb_main.c \
  firmware/bb/bcio.c \
  firmware/player/codec_io.c \
  firmware/player/codecs/flac.c \
  firmware/player/codecs/mp3.c \
  firmware/player/codecs/wav.c \
  firmware/player/tags.c \
  firmware/player/rg_math.c \
  firmware/dsp/eq_bank.c \
  firmware/services/workbuf.c \
  firmware/libc/mem.c \
  firmware/libc/str.c \
  firmware/libc/math_min.c

$(BB_CORE_ELF): $(BB_CORE_SRCS) firmware/player/ipc_shared.h firmware/bb/bb.ld
	@$(MKDIR_P) $(BB_CORE_DIR)
	$(CC) -mcpu=cortex-m3 -mthumb -Os -ffreestanding -fno-builtin \
	    -nostdlib -ffunction-sections -fdata-sections -fcommon \
	    -Wno-incompatible-pointer-types -Wno-int-conversion \
	    -Wno-implicit-function-declaration -Wno-implicit-int \
	    -Wno-builtin-declaration-mismatch -Wno-return-mismatch \
	    -Wno-return-type -Wno-unused-parameter -Wno-unused-variable \
	    $(BB_INC) -include firmware/rockchip/include/armcc_compat.h \
	    -T firmware/bb/bb.ld $(BB_CORE_SRCS) -lgcc -o $@

$(BB_CORE_BIN): $(BB_CORE_ELF)
	$(OBJCOPY) -O binary $< $@

$(BB_CORE_BLOB): $(BB_CORE_BIN)
	cd $(BB_CORE_DIR) && $(OBJCOPY) -I binary -O elf32-littlearm -B arm \
	    --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	    bb_core.bin bb_core_blob.o

# BRNOMT-82: tiny code overlay linked at the free 0x0304AE64 window and
# embedded in the AP image below the initial-row gate. Linked only when
# AP_MEM_OVERLAY=1.
AP_OVERLAY_DIR  := $(CFW_BUILD_DIR)/overlay
AP_OVERLAY_ELF  := $(AP_OVERLAY_DIR)/overlay_demo.elf
AP_OVERLAY_BIN  := $(AP_OVERLAY_DIR)/overlay_demo.bin
AP_OVERLAY_BLOB := $(AP_OVERLAY_DIR)/overlay_demo_blob.o
AP_OVERLAY_LINK := $(if $(filter 1,$(AP_MEM_OVERLAY)),$(AP_OVERLAY_BLOB),)

$(AP_OVERLAY_ELF): firmware/overlay/overlay_demo.c firmware/overlay/overlay_demo.ld
	@$(MKDIR_P) $(AP_OVERLAY_DIR)
	$(CC) -mcpu=cortex-m3 -mthumb -Os -ffreestanding -nostdlib \
	    -ffunction-sections -fdata-sections \
	    -T firmware/overlay/overlay_demo.ld \
	    firmware/overlay/overlay_demo.c -o $@

$(AP_OVERLAY_BIN): $(AP_OVERLAY_ELF)
	$(OBJCOPY) -O binary $< $@

$(AP_OVERLAY_BLOB): $(AP_OVERLAY_BIN)
	cd $(AP_OVERLAY_DIR) && $(OBJCOPY) -I binary -O elf32-littlearm -B arm \
	    --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	    overlay_demo.bin overlay_demo_blob.o

# BRNOMT-82: CDC-ACM console overlay, loaded from /.rechord/cdc.bin at
# runtime. It links against the AP USB symbols as absolute addresses, so
# it must be built after the AP ELF that will be flashed.
CDC_OVERLAY_DIR  := $(CFW_BUILD_DIR)/cdc
CDC_OVERLAY_ELF  := $(CDC_OVERLAY_DIR)/cdc_overlay.elf
CDC_OVERLAY_BIN  := $(CDC_OVERLAY_DIR)/cdc_overlay.bin

$(CDC_OVERLAY_ELF): firmware/overlay/cdc_overlay.c firmware/overlay/cdc_overlay.ld \
                    firmware/drivers/usb_cdc_rknanoc.c $(CFW_ELF)
	@$(MKDIR_P) $(CDC_OVERLAY_DIR)
	@set -e; \
	CABLE=$$(arm-none-eabi-nm -n $(CFW_ELF) | awk '$$3=="usb_cable"{print "0x"$$1}'); \
	INIT=$$(arm-none-eabi-nm -n $(CFW_ELF) | awk '$$3=="usb_dwc2_init"{print "0x"$$1}'); \
	STOP=$$(arm-none-eabi-nm -n $(CFW_ELF) | awk '$$3=="usb_dwc2_stop"{print "0x"$$1}'); \
	echo "CDC AP symbols: cable=$$CABLE init=$$INIT stop=$$STOP"; \
	$(CC) -mcpu=cortex-m3 -mthumb -Os -ffreestanding -nostdlib \
	    -ffunction-sections -fdata-sections -fcommon \
	    -Wno-incompatible-pointer-types -Wno-int-conversion \
	    -Wno-implicit-function-declaration -Wno-implicit-int \
	    -Wno-builtin-declaration-mismatch -Wno-return-mismatch \
	    -Wno-return-type -Wno-unused-parameter -Wno-unused-variable \
	    -Ifirmware -Ifirmware/libc -Ifirmware/drivers -Ifirmware/services \
	    -Ifirmware/rockchip/include \
	    -include firmware/rockchip/include/armcc_compat.h \
	    -T firmware/overlay/cdc_overlay.ld \
	    -Wl,--defsym=usb_cable=$$CABLE \
	    -Wl,--defsym=usb_dwc2_init=$$INIT \
	    -Wl,--defsym=usb_dwc2_stop=$$STOP \
	    firmware/overlay/cdc_overlay.c firmware/drivers/usb_cdc_rknanoc.c \
	    -lgcc -o $@

$(CDC_OVERLAY_BIN): $(CDC_OVERLAY_ELF)
	$(OBJCOPY) -O binary $< $@

cfw-ap: $(CFW_ELF)
	@echo "CFW AP ELF: $(CFW_ELF)"

$(CFW_BUILD_DIR)/objs/%.o: %.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(AP_CFLAGS) -c $< -o $@

$(CFW_ELF): $(CFW_OBJS) $(AP_LDSCRIPT) $(BB_CORE_BLOB) $(AP_OVERLAY_LINK)
	@$(MKDIR_P) $(dir $@)
	$(CC) $(AP_LDFLAGS) $(CFW_OBJS) $(BB_CORE_BLOB) $(AP_OVERLAY_LINK) $(AP_LDLIBS) -o $@

# Product image: fw1 -> 5MB AP5M, no stock/ref reads. This is the default
# `make` target and the only image to flash.
#   make standalone-ap5m AP_MEM_PROBE=1 CFW_BUILD_DIR=build/cfw-probe \
#       AP5M_OUT=build/probe/ReChord_AP.IMG
AP5M_OUT ?= build/ReChord_AP.IMG
standalone-ap5m: $(CFW_ELF)
	$(PYTHON) tools/make_fw1.py $(CFW_ELF) --standalone-5m -o $(AP5M_OUT)

# CDC image: AP with the /.rechord/cdc.bin loader plus the CDC overlay blob.
# Build with:
#   make cdc-image AP_CDC_OVERLAY=1 CFW_BUILD_DIR=build/cfw-cdc \
#       AP5M_OUT=build/cdc/ReChord_AP.IMG
cdc-image: $(CFW_ELF) $(CDC_OVERLAY_BIN)
	$(PYTHON) tools/make_fw1.py $(CFW_ELF) --standalone-5m -o $(AP5M_OUT)
	@echo "CDC overlay blob: $(CDC_OVERLAY_BIN)"

# Local regression check against a saved known-good AP5M image. Run before
# `make standalone-ap5m` (or point GOLDEN at a saved copy).
GOLDEN ?= build/ReChord_AP5M.IMG
verify-standalone:
	$(PYTHON) tools/verify_fw1_5m.py --golden $(GOLDEN)

# Clean-clone acceptance: tracked files only (no stock/ or community/), built
# twice in independent worktrees; both images must be byte-identical.
# Optional boot-confirmed golden: EXPECT_SHA256=<sha256> make clean-clone-check
clean-clone-check:
	tools/clean_clone_check.sh

# Reference-image hook/trial/entry targets were removed with the legacy
# stock/refcfw build path (BRNOMT-82). firmware/hook/ and
# tools/make_hook_img.py stay in the tree as RE reference material only.

include firmware/libc/libc.mk
include firmware/startup/startup.mk
include firmware/app/app.mk
include firmware/drivers/drivers.mk
include firmware/services/services.mk
include firmware/ui/ui.mk
include firmware/theme/theme.mk
include firmware/dsp/dsp.mk
include firmware/modules/modules.mk
include firmware/player/player.mk

# Flags for our own sources. The SDK-era leniency is kept only because
# firmware/rockchip/include/armcc_compat.h is still on the include line; our
# clean-room sources pass -Wall cleanly.
COMMON_FLAGS := -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -Os -Wall \
                -Wno-unused-parameter -Wno-unused-variable \
                -Wno-implicit-function-declaration -Wno-implicit-int \
                -Wno-builtin-declaration-mismatch \
                -Wno-incompatible-pointer-types -Wno-return-mismatch \
                -Wno-int-conversion -Wno-return-type \
                -ffunction-sections -fdata-sections -MMD -MP -fcommon \
                -include firmware/rockchip/include/armcc_compat.h
# LOG_BUFFER_LINES: 12 lines dropped boundary lines during a track change
# (BRNOMT-73 field logs), so the ring holds 48 lines of the latest activity —
# the assembly buffer still fits under the AP .bss ceiling (0x0304A000).
AP_CFLAGS    := $(COMMON_FLAGS) -DRECHORD_AP_BUILD -DAP_DEBUG_TOOLS=$(AP_DEBUG_TOOLS) -DAP_MEM_PROBE=$(AP_MEM_PROBE) -DAP_MEM_OVERLAY=$(AP_MEM_OVERLAY) -DAP_CDC_OVERLAY=$(AP_CDC_OVERLAY) $(if $(filter 0,$(AP_LOCAL_DECODE)),-DRECHORD_AP_METADATA_ONLY=1) -DLOG_BUFFER_LINES=48 -Ifirmware $(addprefix -I,$(AP_INCLUDE_DIRS))

.PHONY: cfw-ap standalone-ap5m cdc-image verify-standalone clean-clone-check \
        test-dsp toolchain manifests compile-check clean

# Host unit test for the freestanding DSP core (runs on the build machine, no
# hardware). Verifies the biquad math before it ever touches the device.
# Links with the HOST compiler: $(CC) is the bare-metal cross toolchain and has
# no host libc (crt0/-lc/-lm), so it cannot build a runnable host binary.
HOSTCC ?= cc
test-dsp: firmware/rechord_dsp_core.c firmware/rechord_dsp_core.h firmware/test_dsp_core.c
	$(HOSTCC) -O2 -o $(BUILD_DIR)/test_dsp_core firmware/test_dsp_core.c firmware/rechord_dsp_core.c -lm
	$(BUILD_DIR)/test_dsp_core
	sh firmware/dsp/tests/run_tests.sh

# ---- RE tooling (optional; not part of the product build) ----------------
toolchain:
	@$(CC) -dumpversion
	@echo "Toolchain OK."

manifests:
	$(PYTHON) tools/check_sdk_manifests.py

compile-check: toolchain
	$(PYTHON) tools/compile_check.py

clean:
	@$(RMDIR_R) build
