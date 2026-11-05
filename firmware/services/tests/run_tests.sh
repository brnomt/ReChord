#!/bin/sh
# Build and run the services host tests with a strict host compiler.
# Usage: ./run_tests.sh   (from anywhere; runs inside tests/)
set -e
cd "$(dirname "$0")"

CC="${CC:-cc}"
CFLAGS="${CFLAGS:--Wall -Werror -O1}"

# Start every run from a clean tree: the media tests walk everything under
# .work, so fixtures left by a previous run (long-path dirs, extra files) would
# leak into their assertions.
rm -rf .work && mkdir -p .work

$CC $CFLAGS -I.. -o .work/test_settings test_settings.c ../settings.c ../settings_fs.c ../fs_host.c
$CC $CFLAGS -I.. -o .work/test_settings_store test_settings_store.c ../settings.c ../settings_store.c ../file_io.c
$CC $CFLAGS -I.. -o .work/test_resume test_resume.c ../resume.c ../file_io.c
$CC $CFLAGS -I.. -o .work/test_log     test_log.c     ../log.c     ../fs_host.c
$CC $CFLAGS -I.. -o .work/test_input   test_input.c   ../input.c
$CC $CFLAGS -I.. -o .work/test_fs      test_fs.c      ../fs_host.c ../file_io.c
$CC $CFLAGS -I.. -o .work/test_store   test_store.c   ../store.c
$CC $CFLAGS -I.. -o .work/test_scratch test_scratch.c ../scratch.c
$CC $CFLAGS -I.. -o .work/test_media_db test_media_db.c ../media_db.c ../scratch.c ../fs_host.c
$CC $CFLAGS -I.. -o .work/test_media_query test_media_query.c ../media_db.c ../media_query.c ../library_extras.c ../scratch.c ../fs_host.c ../file_io.c
$CC $CFLAGS -I.. -I../.. -o .work/test_media_scan test_media_scan.c ../media_scan.c ../media_db.c ../scratch.c ../fs_host.c ../log.c ../../player/tags.c ../../player/codec_io.c ../../player/codec_io_stdio.c
$CC $CFLAGS -I.. -I../.. -o .work/test_eq_profile test_eq_profile.c ../eq_profile.c ../fs_host.c ../file_io.c
$CC $CFLAGS -I.. -o .work/test_bt_proto test_bt_proto.c ../bt_proto.c
$CC $CFLAGS -I.. -o .work/test_power_save test_power_save.c ../power_save.c
$CC $CFLAGS -I.. -o .work/test_audio_scope test_audio_scope.c ../audio_scope.c -lm
$CC $CFLAGS -I.. -I../.. -o .work/test_output_mem test_output_mem.c ../output_mem.c ../fs_host.c ../file_io.c
$CC $CFLAGS -I.. -o .work/test_jack_sense test_jack_sense.c ../jack_sense.c
$CC $CFLAGS -I.. -I../../player -o .work/test_usb_dac_sink test_usb_dac_sink.c \
    ../usb_dac_sink.c ../../player/ringbuf.c
$CC $CFLAGS -I.. -o .work/test_library_extras test_library_extras.c \
    ../library_extras.c ../fs_host.c ../file_io.c
$CC $CFLAGS -I.. -I../.. -o .work/test_bt_link test_bt_link.c ../bt_link.c \
    ../bt_store.c ../bt_proto.c ../log.c ../fs_host.c ../file_io.c
$CC $CFLAGS -I.. -I../.. -o .work/test_art_pack test_art_pack.c ../art_pack.c ../fs_host.c \
    ../../drivers/display.c ../../drivers/panel_init_rknanoc.c ../../drivers/font.c \
    ../../drivers/font8x8.c ../../drivers/panel_io_sim.c

.work/test_settings
.work/test_settings_store
.work/test_resume
.work/test_log
.work/test_input
.work/test_fs
$CC $CFLAGS -I.. -o .work/test_crash_record test_crash_record.c ../crash_record.c
.work/test_crash_record
# fs_target multi-sector reads on real FAT16/FAT32 images (needs mkfs.fat).
MKFS=$(command -v mkfs.fat || command -v /usr/sbin/mkfs.fat || true)
if [ -n "$MKFS" ]; then
    $CC $CFLAGS -Wno-error=maybe-uninitialized -I.. -o .work/test_fs_bulk test_fs_bulk.c ../fs_target.c 2>/dev/null
    truncate -s 32M .work/bulk16.img && "$MKFS" -F 16 -s 8 .work/bulk16.img >/dev/null
    .work/test_fs_bulk .work/bulk16.img
    truncate -s 300M .work/bulk32.img && "$MKFS" -F 32 -s 8 .work/bulk32.img >/dev/null
    .work/test_fs_bulk .work/bulk32.img
    rm -f .work/bulk16.img .work/bulk32.img
else
    echo "test_fs_bulk: skipped (no mkfs.fat)"
fi
.work/test_store
.work/test_scratch
.work/test_media_db
.work/test_media_query
.work/test_media_scan
.work/test_eq_profile
.work/test_bt_proto
.work/test_power_save
.work/test_audio_scope
.work/test_output_mem
.work/test_jack_sense
.work/test_usb_dac_sink
.work/test_library_extras
.work/test_bt_link
.work/test_art_pack
echo "ALL SERVICES TESTS PASSED"
