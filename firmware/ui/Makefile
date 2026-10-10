# firmware/ui/Makefile — standalone build for the frontend workstream:
#   make test    host tests        (cc -Wall -Werror, mock backends)
#   make target  compile the non-test sources for Cortex-M3
#                (arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -c)
#
# The root Makefile integrates the same sources via ui.mk; this file exists
# so the module verifies standalone (verification standard,
# docs/rewrite/architecture.md §5).

CC  ?= cc
ARM ?= arm-none-eabi-gcc

CFLAGS    := -std=c99 -Wall -Werror -O1 -g
ARMCFLAGS := -mcpu=cortex-m3 -mthumb -Os -std=c99 -Wall -Werror

UI_SRCS := ui.c \
           settings_schema.c \
           screens/boot_screen.c \
           screens/menu_screen.c \
           screens/browser_screen.c \
           screens/player_screen.c \
           screens/settings_screen.c

TEST_SRCS := tests/main.c \
             tests/mock_display.c \
             tests/mock_services.c \
             tests/test_screen_stack.c \
             tests/test_ui_redraw.c \
             tests/test_menu_nav.c \
             tests/test_settings_keys.c \
             tests/test_browser_pagination.c

BUILD := build

test: $(BUILD)/ui_tests
	./$(BUILD)/ui_tests

$(BUILD)/ui_tests: $(UI_SRCS) $(TEST_SRCS)
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -I. $(UI_SRCS) $(TEST_SRCS) -o $@

target:
	mkdir -p $(BUILD)/target
	@for f in $(UI_SRCS); do \
	    echo "CC(target) $$f"; \
	    $(ARM) $(ARMCFLAGS) -I. -c $$f -o $(BUILD)/target/$$(basename $$f .c).o || exit 1; \
	done

clean:
	rm -rf $(BUILD)

.PHONY: test target clean
