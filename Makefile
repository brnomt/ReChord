# ReChord — Echo Mini custom firmware build system
#
# Target: ARM Cortex-M3 (Thumb-2), Rockchip RKnanoC
# SDK: Rockchip RKnanoD MP3 v1.3
#
# The device contains two independently built firmware images:
#   AP (fw1)       — application, UI, filesystem, and hardware drivers
#   BB (section_3) — audio services, codecs, and DSP
#
# Never build the entire SDK as one target. The source manifests below are
# derived from the original Keil project and keep the AP and BB object graphs
# isolated.

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

BUILD_DIR    := build
BB_BUILD_DIR := $(BUILD_DIR)/bb
AP_BUILD_DIR := $(BUILD_DIR)/ap
BB_OBJ_DIR   := $(BB_BUILD_DIR)/objs
AP_OBJ_DIR   := $(AP_BUILD_DIR)/objs
BB_LINKER    := firmware/firmware.ld

include firmware/rockchip/manifests/bb.mk
include firmware/rockchip/manifests/ap.mk

ARCH_FLAGS   := -mcpu=cortex-m3 -mthumb -mfloat-abi=soft
COMMON_FLAGS := $(ARCH_FLAGS) -Os -Wall -Wno-unused-parameter \
                -Wno-unused-variable -ffunction-sections -fdata-sections \
                -include firmware/rockchip/include/armcc_compat.h
BB_CFLAGS    := $(COMMON_FLAGS) -DRECHORD_BB_BUILD -D_RK_EQ_ -Ifirmware $(addprefix -I,$(BB_INCLUDE_DIRS))
AP_CFLAGS    := $(COMMON_FLAGS) -DRECHORD_AP_BUILD -Ifirmware $(addprefix -I,$(AP_INCLUDE_DIRS))

BB_OBJS := $(foreach src,$(BB_SRCS),\
             $(BB_OBJ_DIR)/$(patsubst firmware/rockchip/%,%,$(src)).o)
AP_OBJS := $(foreach src,$(AP_SRCS),\
             $(AP_OBJ_DIR)/$(patsubst firmware/rockchip/%,%,$(src)).o)

BB_RECHORD_OBJS := \
    $(BB_BUILD_DIR)/startup.o \
    $(BB_BUILD_DIR)/stubs.o \
    $(BB_BUILD_DIR)/fault.o \
    $(BB_BUILD_DIR)/rechord_win.o \
    $(BB_BUILD_DIR)/rechord_app.o \
    $(BB_BUILD_DIR)/rechord_dsp.o \
    $(BB_BUILD_DIR)/entry_stubs.o

BB_ELF := $(BB_BUILD_DIR)/rechord_bb.elf
BB_BIN := $(BB_BUILD_DIR)/section3_custom.bin
AP_ELF := $(AP_BUILD_DIR)/rechord_ap.elf
AP_BIN := $(AP_BUILD_DIR)/fw1_custom.bin

# Prebuilt codec .lib binaries the Keil RkNano project links (ARM AR archives).
SDK_LIB := community/sdks/RKNanoD_MP3_V1.3_20161102/Common/Codec
AP_CODEC_LIBS := \
  $(SDK_LIB)/Image/Jpg/RkNanoD_JPG_DEC_V150906.lib \
  $(SDK_LIB)/Image/Bmp/RkNano_BMP_DEC_V20150511.lib \
  $(SDK_LIB)/BlueTooth/RKNanoD_LwBT_20161014.lib \
  $(SDK_LIB)/Audio/AAC/RkNanoD_BAAC_20151223.lib \
  $(SDK_LIB)/Audio/DSDIFF/RkNanoD_BDSDIFF_20160929.lib \
  $(SDK_LIB)/Audio/DSF/RkNanoD_BDSF_20160929.lib \
  $(SDK_LIB)/Audio/HIFI/alac/RkNanoD_BHALAC_20160926.lib \
  $(SDK_LIB)/Audio/HIFI/ape/RkNanoD_BHAPE_20160806.lib \
  $(SDK_LIB)/Audio/HIFI/flac/RkNanoD_BHFLAC_20160807.lib \
  $(SDK_LIB)/Audio/Library/RkNano_EQ_24BIT_20150630.lib \
  $(SDK_LIB)/Audio/Library/RkNano_FADE_24BIT_20150611.lib \
  $(SDK_LIB)/Audio/Library/RkNano_Spectrum_V09_0420.lib \
  $(SDK_LIB)/Audio/Mp3/RkNanoD_BMP3_20161031.lib \
  $(SDK_LIB)/Audio/Ogg/RkNanoD_BOGG_20160901.lib \
  $(SDK_LIB)/Audio/RecordControl/NS/RkNanoD_BNS_20151223.lib \
  $(SDK_LIB)/Audio/sbc/RKNanoD_SBCEnc_20160718.lib \
  $(SDK_LIB)/Audio/ShuffleAll/RKNANO_MyRandom_20150927.lib \
  $(SDK_LIB)/Audio/SSRC/RKNanoD_SSRC_20160718.lib \
  $(SDK_LIB)/Audio/Wav/RkNanoD_BWAV_20151223.lib

.PHONY: all bb ap build-bb build-ap link-bb link-ap build-sdk link-firmware \
        toolchain manifests compile-check pack-img pack-bb-img pack-bb-stub-img \
        bb-stub release pack-full apbb-experimental extract-section3 clean

# Single-command SAFE product build: custom BB (audio/DSP) + STOCK AP (UI),
# packed into one flashable IMG. This combination is hardware-verified to
# boot (cassette UI + working mailbox). `release` is the documented entry
# point; `all` is the alias.
#
# DO NOT ship a custom fw1 (AP) in the default build: the fw1 memory-map
# table format is NOT fully reverse-engineered — the stock table's 91
# entries describe the whole system RAM layout (UI framebuffer, audio
# buffers, FAT cache, stacks) that the Mask ROM sets up at boot. Our
# 3-entry replacement destroys that layout and BRICKS the device
# (confirmed on hardware 2026-08-25). Use `make apbb-experimental` for
# fw1-replacement research only.
release: bb pack-bb-img

all: release

bb: build-bb link-bb
	@echo ""
	@echo "ReChord BB firmware built:"
	@$(SIZE) $(BB_ELF)

# AP (fw1) image: compile + link + emit the scatter image consumed by pack-full.
ap: build-ap link-ap pack-fw1
	@echo ""
	@echo "ReChord AP firmware built:"
	@$(SIZE) $(AP_ELF)

# Backward-compatible aliases used by existing documentation and scripts.
build-sdk: build-bb
link-firmware: link-bb

# ---- toolchain and manifest validation -----------------------------------
toolchain:
	@$(CC) -dumpversion
	@echo "Toolchain OK."

manifests:
	$(PYTHON) tools/check_sdk_manifests.py

# ---- BB / section_3 ------------------------------------------------------
build-bb: toolchain manifests $(BB_OBJS)
	@echo "BB SDK compiled: $(words $(BB_OBJS)) objects"

$(BB_OBJ_DIR)/%.o: firmware/rockchip/%
	@$(MKDIR_P) $(dir $@)
	$(CC) $(BB_CFLAGS) -c $< -o $@

$(BB_BUILD_DIR)/startup.o: firmware/startup/startup.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(BB_CFLAGS) -c $< -o $@

$(BB_BUILD_DIR)/stubs.o: firmware/stubs.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(BB_CFLAGS) -c $< -o $@

$(BB_BUILD_DIR)/fault.o: firmware/fault.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(BB_CFLAGS) -c $< -o $@

$(AP_BUILD_DIR)/stubs.o: firmware/stubs.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(AP_CFLAGS) -c $< -o $@

$(AP_BUILD_DIR)/fault.o: firmware/fault.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(AP_CFLAGS) -c $< -o $@

$(AP_BUILD_DIR)/ap_startup.o: firmware/startup/ap_startup.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(AP_CFLAGS) -c $< -o $@

$(BB_BUILD_DIR)/rechord_win.o: firmware/rechord_win.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(BB_CFLAGS) -c $< -o $@

$(BB_BUILD_DIR)/rechord_app.o: firmware/rechord_app.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(BB_CFLAGS) -c $< -o $@

$(BB_BUILD_DIR)/rechord_dsp.o: firmware/rockchip/audio/RkEQ/Effect/rechord_dsp.c \
		firmware/rockchip/audio/RkEQ/Effect/rechord_dsp.h
	@$(MKDIR_P) $(dir $@)
	$(CC) $(BB_CFLAGS) -c $< -o $@

$(BB_BUILD_DIR)/entry_stubs.o: firmware/entry_stubs.S
	@$(MKDIR_P) $(dir $@)
	$(CC) $(ARCH_FLAGS) -c $< -o $@

link-bb: $(BB_RECHORD_OBJS) $(BB_OBJS)
	$(CC) $(ARCH_FLAGS) -T $(BB_LINKER) -nostartfiles -ffreestanding \
		$(BB_RECHORD_OBJS) $(BB_OBJS) -lm -o $(BB_ELF)
	$(OBJCOPY) -O binary -j .fw_header -j .text $(BB_ELF) $(BB_BIN)
	@echo "Built: $(BB_BIN)"

# ---- minimal dummy BB (mailbox responder, no SDK) ------------------------
# Links startup.o + stubs.o + fault.o + entry_stubs.o + bb_stub.o only:
# bb_stub.c provides rechord_hw_init/rechord_main that entry_stubs.S calls,
# plus its own RAM vector table (bb_vect) and direct-MMIO mailbox ISRs.
BB_STUB_ELF := $(BB_BUILD_DIR)/rechord_bb_stub.elf
BB_STUB_BIN := $(BB_BUILD_DIR)/section3_stub.bin
BB_STUB_OBJS := $(BB_BUILD_DIR)/startup.o $(BB_BUILD_DIR)/stubs.o \
                $(BB_BUILD_DIR)/fault.o $(BB_BUILD_DIR)/entry_stubs.o \
                $(BB_BUILD_DIR)/bb_stub.o

$(BB_BUILD_DIR)/bb_stub.o: firmware/bb_stub/bb_stub.c firmware/rechord_version.h
	@$(MKDIR_P) $(dir $@)
	$(CC) $(ARCH_FLAGS) -Os -Wall -Ifirmware -c $< -o $@

bb-stub: $(BB_STUB_OBJS)
	$(CC) $(ARCH_FLAGS) -T $(BB_LINKER) -nostartfiles -ffreestanding \
		$(BB_STUB_OBJS) -o $(BB_STUB_ELF)
	$(OBJCOPY) -O binary -j .fw_header -j .text $(BB_STUB_ELF) $(BB_STUB_BIN)
	@$(SIZE) $(BB_STUB_ELF)
	@echo "Built: $(BB_STUB_BIN)"

# Pack the dummy BB into a flashable IMG (stock AP preserved).
# --keep-stock-tail: preserve stock section_3 bytes beyond our small stub so
# the stock AP's scatter-table/overlay references into that flash region
# stay valid (blank-UI-text / crash fix; see docs/re/AP-MAILBOX-GUIDE.md).
pack-bb-stub-img: $(BB_STUB_BIN)
	$(PYTHON) tools/pack_img.py --pack $(BB_STUB_BIN) --keep-stock-tail -o $(BUILD_DIR)/ReChord_BB_Stub.IMG

# Attempt the AP (fw1) link to enumerate remaining undefined symbols.
# The AP (fw1) is the resident A_CORE: UI + drivers + filesystem. The codec
# .lib (AP_CODEC_LIBS) and audio/image/BT/FM sources live in the BB/overlays,
# so they are NOT linked here; their entry points are weak stubs in stubs.c.
# ap_startup.o is linked FIRST so its .vectors table lands at SYS_CODE
# (0x03060000) — the reset vector [1] points at Main.
link-ap: $(AP_OBJS) $(AP_BUILD_DIR)/ap_startup.o $(AP_BUILD_DIR)/stubs.o $(AP_BUILD_DIR)/fault.o
	$(CC) $(ARCH_FLAGS) -T firmware/firmware_ap.ld -nostartfiles -ffreestanding \
		$(AP_BUILD_DIR)/ap_startup.o $(AP_BUILD_DIR)/stubs.o $(AP_BUILD_DIR)/fault.o $(AP_OBJS) -lm -o $(AP_ELF)
	$(OBJCOPY) -O binary -j .text -j .data $(AP_ELF) $(AP_BIN)
	@echo "AP linked: $(AP_ELF) -> $(AP_BIN)"

# ---- AP / fw1 ------------------------------------------------------------
# Compiles the resident A_CORE AP (UI + drivers + filesystem + LCD). The
# codec .lib / audio / image / BT / FM / record overlay sources live in the
# BB/overlays, so their entry points are weak stubs in stubs.c. Links and
# packs cleanly (see link-ap / pack-fw1 / pack-full).
build-ap: toolchain manifests $(AP_OBJS)
	@echo "AP SDK compile passed: $(words $(AP_OBJS)) imported objects"

$(AP_OBJ_DIR)/%.o: firmware/rockchip/%
	@$(MKDIR_P) $(dir $@)
	$(CC) $(AP_CFLAGS) -c $< -o $@

# ---- checks and packaging ------------------------------------------------
compile-check: toolchain
	$(PYTHON) tools/compile_check.py

# Identity-test the stock section_3 splice operation.
pack-img:
	$(PYTHON) tools/pack_img.py --identity-test

# Produce a test IMG containing the custom BB while preserving the stock AP.
# --keep-stock-tail: the stock fw1 memory-map table XIP-copies data from
# inside the section_3 flash region, so zero-padding it corrupts the AP.
pack-bb-img: $(BB_BIN)
	$(PYTHON) tools/pack_img.py --pack $(BB_BIN) --keep-stock-tail -o $(BUILD_DIR)/ReChord_BB.IMG

# Emit the fw1 (AP) RKnanoFW scatter image from the AP ELF.
FW1_IMG := $(AP_BUILD_DIR)/fw1_custom.img
pack-fw1: $(AP_ELF)
	$(PYTHON) tools/pack_fw1.py $(AP_ELF) -o $(FW1_IMG)

# Pack BOTH halves (fw1 + section_3) into one IMG. EXPERIMENTAL — KNOWN TO
# BRICK on hardware (2026-08-25): the custom fw1 memory-map table does not
# reproduce the stock 91-entry system RAM layout. Not part of `release`.
pack-full: $(FW1_IMG) $(BB_BIN)
	$(PYTHON) tools/pack_img.py --pack-full --fw1 $(FW1_IMG) --bb $(BB_BIN) -o $(BUILD_DIR)/ReChord_APBB.IMG

# Explicit, loud alias for the bricking combination (research only).
apbb-experimental: bb ap pack-full
	@echo ""
	@echo "==================================================================="
	@echo " WARNING: ReChord_APBB.IMG replaces fw1 (AP) with a custom image."
	@echo " The fw1 memory-map table is NOT validated — THIS BRICKED A DEVICE"
	@echo " on 2026-08-25. Flash only with maskrom recovery ready."
	@echo " Safe default: make release -> build/ReChord_BB.IMG"
	@echo "==================================================================="

extract-section3:
	$(PYTHON) tools/pack_img.py --extract -o $(BUILD_DIR)/section3_stock.bin

clean:
	@$(RMDIR_R) build
