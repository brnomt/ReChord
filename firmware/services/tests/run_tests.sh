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
$CC $CFLAGS -I.. -o .work/test_settings_store test_settings_store.c ../settings.c ../settings_store.c
$CC $CFLAGS -I.. -o .work/test_resume test_resume.c ../resume.c
$CC $CFLAGS -I.. -o .work/test_log     test_log.c     ../log.c     ../fs_host.c
$CC $CFLAGS -I.. -o .work/test_input   test_input.c   ../input.c
$CC $CFLAGS -I.. -o .work/test_fs      test_fs.c      ../fs_host.c
$CC $CFLAGS -I.. -o .work/test_store   test_store.c   ../store.c
$CC $CFLAGS -I.. -o .work/test_scratch test_scratch.c ../scratch.c
$CC $CFLAGS -I.. -o .work/test_media_db test_media_db.c ../media_db.c ../scratch.c ../fs_host.c
$CC $CFLAGS -I.. -o .work/test_media_query test_media_query.c ../media_db.c ../media_query.c ../scratch.c ../fs_host.c
$CC $CFLAGS -I.. -I../.. -o .work/test_media_scan test_media_scan.c ../media_scan.c ../media_db.c ../scratch.c ../fs_host.c ../log.c ../../player/tags.c ../../player/codec_io.c ../../player/codec_io_stdio.c
$CC $CFLAGS -I.. -I../.. -o .work/test_eq_profile test_eq_profile.c ../eq_profile.c ../fs_host.c
$CC $CFLAGS -I.. -I../.. -o .work/test_art_pack test_art_pack.c ../art_pack.c ../fs_host.c \
    ../../drivers/display.c ../../drivers/panel_init_rknanoc.c ../../drivers/font.c \
    ../../drivers/font8x8.c ../../drivers/panel_io_sim.c

.work/test_settings
.work/test_settings_store
.work/test_resume
.work/test_log
.work/test_input
.work/test_fs
.work/test_store
.work/test_scratch
.work/test_media_db
.work/test_media_query
.work/test_media_scan
.work/test_eq_profile
.work/test_art_pack
echo "ALL SERVICES TESTS PASSED"
