# firmware/ap.mk -- build fragment for the clean-room AP (fw1) image,
# milestone 1a (fragment style per docs/rewrite/architecture.md section 2 and
# firmware/libc/libc.mk).  It lists the AP link inputs and the compile/link
# flags; build rules stay in the root Makefile.
#
# Usage (root Makefile, clean-room pivot -- replaces the stock-AP wiring
# from firmware/rockchip/manifests/ap.mk):
#     include firmware/ap.mk
#
# Per-source compile:
#     $(CC) $(AP_CFLAGS) -c <src> -o <obj>        # one .o per $(AP_SRCS)
# Link (mirrors link-bb in the Makefile):
#     $(CC) $(AP_LDFLAGS) $(AP_OBJS) $(AP_LDLIBS) -o $(AP_ELF)
#
# Image layout / linker contract: firmware/firmware_ap.ld (SYS_CODE
# 0x03060000 entry via ap_vectors, SYS_DATA 0x03000000, MSP 0x03050000 --
# docs/fw1-packing.md).
#
# firmware/player is now linked (milestone 2a): player.c, codec_io, the codec
# adapters, tags/replaygain, rg_math, and the A<->B IPC target backend
# (ipc_b_target.c). It is the clean-road audio path; see docs/rewrite/audio.md.
#
# NOT linked in milestone 1a (by design):
#   firmware/dsp, firmware/modules                      (later milestones)
#   firmware/rockchip/**                                (stock SDK, hybrid era)
#   firmware/services/fs_host.c                         (host-only backend;
#       the device FS backend is fs_target.c, wired via fs.h)
#   firmware/stubs.c, firmware/fault.c                  (hybrid-era glue)
#
# NOTE on -std: the Makefile's COMMON_FLAGS passes no -std (default gnu17).
# The clean-room sources are C99 (ui.mk uses -std=c99) and do NOT compile
# under -std=gnu89 (for-loop initial declarations in ui/settings_schema.c
# and libc/math_min.c), so this fragment matches the Makefile: no -std.
#
# NOTE on weak stubs: firmware/startup/ap_main.c carries weak link stubs
# (rechord_startup_entry for the legacy vectors.c table, settings_view_load/
# settings_view_save until the storage adapters exist, strchr/strrchr until
# they move into firmware/libc/str.c, and fopen/fgets/fgetc/feof/fclose for
# theme_file.c) so the link closes with zero undefined symbols.  Each is a
# stub/provisional only and is reported in the milestone-1a report.
# panel_init_sequence is NOT stubbed: the real panel_init_rknanoc.c is in
# AP_DRIVER_SRCS above.

AP_LDSCRIPT := firmware/firmware_ap.ld

# ---- link inputs (the clean-room tree) ---------------------------------
# startup: vector table + reset recipe (ap_startup.c) and the minimal AP
# main + weak link stubs (ap_main.c).
AP_STARTUP_SRCS := \
  firmware/startup/ap_startup.c \
  firmware/startup/vectors.c \
  firmware/startup/ap_main.c \
  firmware/startup/ap_ui.c

# UI layer (same set as $(UI_SRCS) in firmware/ui/ui.mk).
AP_UI_SRCS := \
  firmware/ui/ui.c \
  firmware/ui/settings_schema.c \
  firmware/ui/settings_view.c \
  firmware/ui/screens/boot_screen.c \
  firmware/ui/screens/menu_screen.c \
  firmware/ui/screens/browser_screen.c \
  firmware/ui/screens/player_screen.c \
  firmware/ui/screens/settings_screen.c

# Drivers: the TARGET panel backend + the panel init sequence tables
# (same set as $(DRIVERS_TARGET_SRCS) in firmware/drivers/drivers.mk,
# including panel_init_rknanoc.c from DRIVERS_COMMON_SRCS).
AP_DRIVER_SRCS := \
  firmware/drivers/display.c \
  firmware/drivers/font.c \
  firmware/drivers/font8x8.c \
  firmware/drivers/panel_init_rknanoc.c \
  firmware/drivers/panel_io_rknanoc.c \
  firmware/drivers/emmc_rknanoc.c \
  firmware/drivers/usb_msc_rknanoc.c \
  firmware/drivers/usb_cdc_rknanoc.c \
  firmware/drivers/cs43131.c \
  firmware/drivers/rknano_i2s.c

# Audio output HAL (modular pipeline, docs/rewrite/audio.md): controller +
# pure per-rate table. Replaces the monolithic dac_cs43131.c (register
# code moved verbatim; see firmware/drivers/ao_regs.h).
AP_AUDIO_SRCS := \
  firmware/audio/audio_output.c \
  firmware/audio/ao_rate_table.c

# Services: settings/log/input + the target fs backend (same set as
# $(SERVICES_TARGET_SRCS) in firmware/services/services.mk).
AP_SERVICE_SRCS := \
  firmware/services/settings.c \
  firmware/services/settings_store.c \
  firmware/services/log.c \
  firmware/services/input.c \
  firmware/services/store.c \
  firmware/services/scratch.c \
  firmware/services/media_db.c \
  firmware/services/fw_update.c \
  firmware/services/fs_target.c \
  firmware/services/rechord_files.c

# Theme module (same set as $(THEME_SRCS) in firmware/theme/theme.mk).
AP_THEME_SRCS := \
  firmware/theme/theme.c \
  firmware/theme/themes_builtin.c \
  firmware/theme/theme_file.c

# Freestanding libc, build-from-source (same set as $(LIBC_SRCS) in
# firmware/libc/libc.mk -- listed explicitly so this fragment is standalone).
# stdio_fs.c is target-only (it needs services/fs.h): the same stdio subset
# is real stdio on the host, so it is deliberately NOT in libc.mk.
AP_LIBC_SRCS := \
  firmware/libc/mem.c \
  firmware/libc/str.c \
  firmware/libc/stdio_min.c \
  firmware/libc/stdio_fs.c \
  firmware/libc/math_min.c

# Player module (audio path): linked from milestone 2a. Includes the
# target IPC backend (ipc_b_target.c) and the codecs — one file per codec
# under player/codecs/ (self-describing descriptors + registry), vendored
# minimp3/dr_flac are header-included by their adapter TUs.
AP_PLAYER_SRCS := \
  firmware/player/player.c \
  firmware/player/codec_io.c \
  firmware/player/codec_io_fs.c \
  firmware/player/tags.c \
  firmware/player/rg_math.c \
  firmware/player/codecs/codec_registry.c \
  firmware/player/codecs/wav.c \
  firmware/player/codecs/mp3.c \
  firmware/player/codecs/flac.c \
  firmware/player/ipc_b_target.c \
  firmware/player/ringbuf.c

# Every AP link input, milestone 2a (includes clean-road player).
AP_SRCS := \
  $(AP_STARTUP_SRCS) \
  $(AP_UI_SRCS) \
  $(AP_DRIVER_SRCS) \
  $(AP_AUDIO_SRCS) \
  $(AP_SERVICE_SRCS) \
  $(AP_THEME_SRCS) \
  $(AP_LIBC_SRCS) \
  $(AP_PLAYER_SRCS)

# ---- include roots (-I) ------------------------------------------------
# firmware/ itself comes first (as in AP_CFLAGS in the Makefile) so
# "drivers/display.h" style includes resolve; the module roots follow.
AP_INCLUDE_DIRS := \
  firmware \
  firmware/libc \
  firmware/drivers \
  firmware/services \
  firmware/player \
  firmware/ui \
  firmware/theme

# ---- flags -------------------------------------------------------------
# Mirrors the BB build exactly (Makefile: ARCH_FLAGS + COMMON_FLAGS +
# AP_CFLAGS), i.e. the SDK-era warning leniency set, -ffunction-sections /
# -fdata-sections / -fcommon, and the force-included armcc_compat.h.
AP_ARCH_FLAGS := -mcpu=cortex-m3 -mthumb -mfloat-abi=soft

AP_CFLAGS := $(AP_ARCH_FLAGS) -Os -Wall \
  -Wno-unused-parameter -Wno-unused-variable \
  -Wno-implicit-function-declaration -Wno-implicit-int \
  -Wno-builtin-declaration-mismatch -Wno-incompatible-pointer-types \
  -Wno-return-mismatch -Wno-int-conversion -Wno-return-type \
  -ffunction-sections -fdata-sections -MMD -MP -fcommon \
  -DRECHORD_AP_BUILD \
  "-DLOG_BUFFER_LINES=12" \
  $(addprefix -I,$(AP_INCLUDE_DIRS)) \
  -include firmware/rockchip/include/armcc_compat.h

# Link: no toolchain libc/startup -- the firmware provides its own libc
# sources (AP_LIBC_SRCS); -lgcc supplies the compiler runtime
# (__aeabi_* soft-float helpers), same as link-bb in the Makefile.
AP_LDFLAGS := $(AP_ARCH_FLAGS) -T $(AP_LDSCRIPT) -Wl,-z,max-page-size=0x1000 \
  -nostartfiles -nostdlib -ffreestanding

AP_LDLIBS := -lgcc
