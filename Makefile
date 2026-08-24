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
BB_CFLAGS    := $(COMMON_FLAGS) -DRECHORD_BB_BUILD -D_RK_EQ_ $(addprefix -I,$(BB_INCLUDE_DIRS))
AP_CFLAGS    := $(COMMON_FLAGS) -DRECHORD_AP_BUILD $(addprefix -I,$(AP_INCLUDE_DIRS))

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
        bb-stub extract-section3 clean

# The default remains the currently linkable BB firmware. AP is intentionally
# opt-in until its missing source modules and linker layout are restored.
all: bb

bb: build-bb link-bb
	@echo ""
	@echo "ReChord BB firmware built:"
	@$(SIZE) $(BB_ELF)

ap: build-ap

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
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CC) $(BB_CFLAGS) -c $< -o $@

$(BB_BUILD_DIR)/startup.o: firmware/startup/startup.c
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CC) $(BB_CFLAGS) -c $< -o $@

$(BB_BUILD_DIR)/stubs.o: firmware/stubs.c
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CC) $(BB_CFLAGS) -c $< -o $@

$(BB_BUILD_DIR)/fault.o: firmware/fault.c
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CC) $(BB_CFLAGS) -c $< -o $@

$(AP_BUILD_DIR)/stubs.o: firmware/stubs.c
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CC) $(AP_CFLAGS) -c $< -o $@

$(AP_BUILD_DIR)/fault.o: firmware/fault.c
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CC) $(AP_CFLAGS) -c $< -o $@

$(AP_BUILD_DIR)/ap_startup.o: firmware/startup/ap_startup.c
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CC) $(AP_CFLAGS) -c $< -o $@

$(BB_BUILD_DIR)/rechord_win.o: firmware/rechord_win.c
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CC) $(BB_CFLAGS) -c $< -o $@

$(BB_BUILD_DIR)/rechord_app.o: firmware/rechord_app.c
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CC) $(BB_CFLAGS) -c $< -o $@

$(BB_BUILD_DIR)/rechord_dsp.o: firmware/rockchip/audio/RkEQ/Effect/rechord_dsp.c \
		firmware/rockchip/audio/RkEQ/Effect/rechord_dsp.h
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CC) $(BB_CFLAGS) -c $< -o $@

$(BB_BUILD_DIR)/entry_stubs.o: firmware/entry_stubs.S
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
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
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
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
# This target compiles the 165 AP sources currently present in the repository.
# It does not link yet: AP_MISSING_SRCS documents 33 effective Keil inputs that
# must be imported or replaced first, and fw1 still needs its own linker script.
build-ap: toolchain manifests $(AP_OBJS)
	@echo "AP SDK compile check passed: $(words $(AP_OBJS)) imported objects"
	@echo "AP link remains blocked by $(words $(AP_MISSING_SRCS)) missing sources"

$(AP_OBJ_DIR)/%.o: firmware/rockchip/%
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CC) $(AP_CFLAGS) -c $< -o $@

# ---- checks and packaging ------------------------------------------------
compile-check: toolchain
	$(PYTHON) tools/compile_check.py

# Identity-test the stock section_3 splice operation.
pack-img:
	$(PYTHON) tools/pack_img.py --identity-test

# Produce a test IMG containing the custom BB while preserving the stock AP.
pack-bb-img: $(BB_BIN)
	$(PYTHON) tools/pack_img.py --pack $(BB_BIN) -o $(BUILD_DIR)/ReChord_BB.IMG

# Emit the fw1 (AP) RKnanoFW scatter image from the AP ELF.
FW1_IMG := $(AP_BUILD_DIR)/fw1_custom.img
pack-fw1: $(AP_ELF)
	$(PYTHON) tools/pack_fw1.py $(AP_ELF) -o $(FW1_IMG)

# Pack BOTH halves (fw1 + section_3) into one flashable IMG.
pack-full: $(FW1_IMG) $(BB_BIN)
	$(PYTHON) tools/pack_img.py --pack-full --fw1 $(FW1_IMG) --bb $(BB_BIN) -o $(BUILD_DIR)/ReChord_APBB.IMG

extract-section3:
	$(PYTHON) tools/pack_img.py --extract -o $(BUILD_DIR)/section3_stock.bin

clean:
	@powershell -NoProfile -Command "if (Test-Path 'build') { Remove-Item -Recurse -Force 'build' }"
