#!/bin/sh
# Build and run the frontend (firmware/ui) host tests with a strict host
# compiler. Mirrors firmware/player/tests/run_tests.sh and
# firmware/services/tests/run_tests.sh.
# Usage: ./run_tests.sh   (from anywhere; runs inside tests/)
set -e
cd "$(dirname "$0")"

CC="${CC:-cc}"
CFLAGS="${CFLAGS:--Wall -Werror -O1}"

mkdir -p .work

# The mocks replace the hardware/player/settings seams (mock_services.c is
# the link seam for player_*/settings_*; mock_display.c captures draws).
$CC $CFLAGS -I.. -I../.. -I../../services -I../../player -o .work/ui_tests \
    main.c test_screen_stack.c test_ui_redraw.c test_menu_nav.c \
    test_settings_keys.c test_browser_pagination.c \
    ../ui.c ../screens/*.c ../settings_schema.c \
    ../../services/media_query.c ../../services/scratch.c \
    ../../theme/theme.c ../../theme/themes_builtin.c ../../theme/theme_pkg.c \
    mock_display.c mock_services.c -lm

.work/ui_tests

# The target view<->store glue (settings_view.c) is replaced by the mocks in
# the binary above, so it gets its own: real glue + real schema converters,
# with only the settings service I/O stubbed (test_settings_view.c).
$CC $CFLAGS -I.. -I../.. -I../../services -o .work/settings_view_tests \
    test_settings_view.c ../settings_view.c ../settings_schema.c \
    ../../services/settings.c

.work/settings_view_tests

echo "ALL UI TESTS PASSED"
