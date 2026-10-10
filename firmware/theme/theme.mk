# theme.mk — build fragment for the ReChord theme module (firmware/theme/).
#
# Included by the root Makefile (which owns the rules and stays thin):
#     include firmware/theme/theme.mk
#
# The module is freestanding C99: no SDK includes, compiles for host tests
# (cc -Wall -Werror) and target (arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb
# -Os -c) unchanged. theme_file.c uses stdio only for the file loader; on the
# target it will route through the services/ fs abstraction once that lands
# (open item in docs/rewrite/customization.md).

THEME_DIR := firmware/theme

# Module sources (public API theme.h; theme_priv.h is internal).
THEME_SRCS := \
  $(THEME_DIR)/theme.c \
  $(THEME_DIR)/themes_builtin.c \
  $(THEME_DIR)/theme_file.c

# Host-side per-module tests (run with cc, see docs/rewrite/customization.md).
THEME_TEST_SRCS := \
  $(THEME_DIR)/tests/test_theme.c
