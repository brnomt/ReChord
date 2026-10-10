# firmware/ui/ui.mk — build fragment for the root Makefile
# (frontend workstream: UI core + screens + theme + settings schema).
#
# The root Makefile stays thin: include this fragment and add
# $(UI_OBJS) to the AP/UI link.  Object files go to $(BUILD_DIR)/ui/objs/.
#
# Usage (root Makefile):
#   include firmware/ui/ui.mk
#
# Notes for integration:
#   - when firmware/drivers/display.h and firmware/services/*.h exist, build
#     with -DRECHORD_UI_TARGET -Ifirmware/drivers -Ifirmware/services and
#     WITHOUT firmware/ui/internal/shims on the include path (see
#     firmware/ui/internal/hal.h).

UI_DIR := firmware/ui

UI_SRCS := $(UI_DIR)/ui.c \
           $(UI_DIR)/settings_schema.c \
           $(UI_DIR)/screens/boot_screen.c \
           $(UI_DIR)/screens/menu_screen.c \
           $(UI_DIR)/screens/browser_screen.c \
           $(UI_DIR)/screens/player_screen.c \
           $(UI_DIR)/screens/settings_screen.c

UI_CFLAGS := -std=c99 -Wall -Werror -Os -I$(UI_DIR)
UI_OBJDIR := $(BUILD_DIR)/ui/objs
UI_OBJS   := $(patsubst $(UI_DIR)/%.c,$(UI_OBJDIR)/%.o,$(UI_SRCS))

$(UI_OBJDIR)/%.o: $(UI_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(UI_CFLAGS) $(TARGET_CFLAGS) -c $< -o $@
