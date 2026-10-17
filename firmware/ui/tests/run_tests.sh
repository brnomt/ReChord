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
$CC $CFLAGS -I.. -I../.. -I../../services -o .work/ui_tests \
    main.c test_screen_stack.c test_ui_redraw.c test_menu_nav.c \
    test_settings_keys.c test_browser_pagination.c \
    ../ui.c ../screens/*.c ../settings_schema.c \
    ../../theme/theme.c ../../theme/themes_builtin.c \
    mock_display.c mock_services.c -lm

.work/ui_tests
echo "ALL UI TESTS PASSED"
