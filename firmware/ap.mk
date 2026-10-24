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

# Debug-only actions in the root System menu (DAC test, USB CDC console).
# Off by default: they cost ~3 KB of code and the AP code span must stay below
# LOADER_CODE_BASE 0x03088000 (DFIU 51cc212d). Build with
#   make cfw-ap AP_DEBUG_TOOLS=1
# only after freeing room elsewhere; the linker/packer gate will refuse an
# image that crosses the loader code.
AP_DEBUG_TOOLS ?= 0

# Runtime probe for the 0x03088000 overlay window. Off by default; the
# probe image is built with AP_MEM_PROBE=1 in a separate CFW_BUILD_DIR.
AP_MEM_PROBE ?= 0

# Runtime code overlay in the free 0x0304AE64 loader-data window. Off by
# default; the overlay image is built with AP_MEM_OVERLAY=1 separately.
AP_MEM_OVERLAY ?= 0

# CDC console overlay loaded from /.rechord/cdc.bin and executed at
# 0x0304AE64. Off by default; the CDC image is built separately.
AP_CDC_OVERLAY ?= 0

# Local decode fallback in the AP. Core B owns decoding on the target, so the
# AP only needs the codecs' metadata paths (open/format/duration/tags/prescan);
# the decoder libraries (minimp3 ~20 KB, dr_flac ~35 KB of the AP's code span)
# are linked only when this is 1. Off by default because the AP code span is
# at the loader gate (DFIU 51cc212d): the fallback costs more than the whole
# remaining budget. Build with AP_LOCAL_DECODE=1 for a diagnostic image.
AP_LOCAL_DECODE ?= 0

# ---- link inputs (the clean-room tree) ---------------------------------
# startup: vector table + reset recipe (ap_startup.c) and the minimal AP
# main + weak link stubs (ap_main.c).
AP_STARTUP_SRCS := \
  firmware/startup/ap_startup.c \
  firmware/startup/ap_main.c \
  firmware/startup/ap_ui.c \
  firmware/startup/mem_probe.c \
  firmware/startup/overlay_demo.c \
  firmware/startup/cdc_overlay.c

# UI layer (the AP subset; boot_screen.c is not used by the clean-room AP).
AP_UI_SRCS := \
  firmware/ui/ui.c \
  firmware/ui/settings_schema.c \
  firmware/ui/settings_view.c \
  firmware/ui/screens/menu_screen.c \
  firmware/ui/screens/browser_screen.c \
  firmware/ui/screens/player_screen.c \
  firmware/ui/screens/settings_screen.c \
  firmware/ui/screens/library_screen.c \
  firmware/ui/screens/library_cat_screen.c \
  firmware/ui/screens/skin_screen.c \
  firmware/ui/screens/quick_screen.c \
  firmware/ui/screens/theme_ui.c

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
  $(if $(filter 1,$(AP_DEBUG_TOOLS)),firmware/drivers/usb_cdc_rknanoc.c) \
  firmware/drivers/cs43131.c \
  firmware/drivers/core_b.c \
  firmware/drivers/cru_pll.c \
  firmware/drivers/rknano_i2s.c \
  firmware/drivers/battery.c

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
  firmware/services/resume.c \
  firmware/services/log.c \
  firmware/services/input.c \
  firmware/services/scratch.c \
  firmware/services/workbuf.c \
  firmware/services/media_db.c \
  firmware/services/media_scan.c \
  firmware/services/media_query.c \
  firmware/services/art_pack.c \
  firmware/services/fw_update.c \
  firmware/services/fs_target.c \
  firmware/services/rechord_files.c

# Theme module (same set as $(THEME_SRCS) in firmware/theme/theme.mk).
AP_THEME_SRCS := \
  firmware/theme/theme.c \
  firmware/theme/theme_pkg.c \
  firmware/theme/theme_runtime.c \
  firmware/theme/theme_assets.c \
  firmware/theme/theme_skin.c \
  firmware/theme/themes_builtin.c \
  firmware/theme/theme_file.c

# Freestanding libc, build-from-source (same set as $(LIBC_SRCS) in
# firmware/libc/libc.mk -- listed explicitly so this fragment is standalone).
# stdio_fs.c is target-only (it needs services/fs.h): the same stdio subset
# is real stdio on the host, so it is deliberately NOT in libc.mk.
# math_min.c is deliberately NOT linked: only the DSP core (BB image) uses
# sin/cos/pow and no AP object references them (verified with nm -u), while
# the floating-point versions cost 1.4 KB of the tight FW_CODE window.
AP_LIBC_SRCS := \
  firmware/libc/mem.c \
  firmware/libc/str.c \
  firmware/libc/stdio_min.c \
  firmware/libc/stdio_fs.c

# Player module (audio path): linked from milestone 2a. Includes the
# target IPC backend (ipc_b_target.c) and the codecs — one file per codec
# under player/codecs/ (self-describing descriptors + registry), vendored
# minimp3/dr_flac are header-included by their adapter TUs.
AP_PLAYER_SRCS := \
  firmware/player/player.c \
  firmware/player/bfs.c \
  firmware/player/playlist.c \
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
# SUPERSEDED: the root Makefile defines AP_CFLAGS *after* including this file
# (Makefile "AP_CFLAGS :="), and every rule uses that definition — so the block
# below is only a fallback and must not disagree with it. In particular
# LOG_BUFFER_LINES is owned by the Makefile (currently 48; the 12 here was a
# stale duplicate that once shadowed nothing but read as if it applied).
AP_ARCH_FLAGS := -mcpu=cortex-m3 -mthumb -mfloat-abi=soft

AP_CFLAGS := $(AP_ARCH_FLAGS) -Os -Wall \
  -Wno-unused-parameter -Wno-unused-variable \
  -Wno-implicit-function-declaration -Wno-implicit-int \
  -Wno-builtin-declaration-mismatch -Wno-incompatible-pointer-types \
  -Wno-return-mismatch -Wno-int-conversion -Wno-return-type \
  -ffunction-sections -fdata-sections -MMD -MP -fcommon \
  -DRECHORD_AP_BUILD \
  -DAP_DEBUG_TOOLS=$(AP_DEBUG_TOOLS) \
  "-DLOG_BUFFER_LINES=48" \
  $(addprefix -I,$(AP_INCLUDE_DIRS)) \
  -include firmware/rockchip/include/armcc_compat.h

# Link: no toolchain libc/startup -- the firmware provides its own libc
# sources (AP_LIBC_SRCS); -lgcc supplies the compiler runtime
# (__aeabi_* soft-float helpers), same as link-bb in the Makefile.
AP_LDFLAGS := $(AP_ARCH_FLAGS) -T $(AP_LDSCRIPT) -Wl,-z,max-page-size=0x1000 \
  -nostartfiles -nostdlib -ffreestanding

AP_LDLIBS := -lgcc
