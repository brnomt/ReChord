# app.mk — build fragment for the app glue (firmware/app/).
#
# Included by the root Makefile (which owns the rules and stays thin):
#     include firmware/app/app.mk
#
# Sources:
#   main.c            — rechord_main: bring-up order + main loop
#   module_registry.c — static feature-module registry (host-tested)
#   boot_params.h     — header-only boot-param struct/accessors (storage is
#                       defined by firmware/startup/startup.c on target)
#
# Host tests (firmware/app/tests/) link the sources above against mocks that
# implement the real service/driver headers. Exact commands:
#
#   # build + run (everything must pass with -Wall -Werror). The small
#   # RECHORD_IDLE_FRAMES/RECHORD_SLEEP_FRAMES make the idle/sleep policy
#   # observable within a few mocked frames (they are the production
#   # knobs' test values, see main.c).
#   cc -Wall -Werror -DRECHORD_UI_TARGET \
#      -DRECHORD_IDLE_FRAMES=2 -DRECHORD_SLEEP_FRAMES=4 \
#      -Ifirmware -Ifirmware/services -Ifirmware/drivers \
#      firmware/app/main.c firmware/app/module_registry.c \
#      firmware/app/tests/main.c firmware/app/tests/mocks.c \
#      firmware/app/tests/test_bringup.c firmware/app/tests/test_registry.c \
#      firmware/app/tests/test_boot_params.c \
#      -o build/test_app && ./build/test_app
#
#   # target compile check of the non-test sources:
#   arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -c -Ifirmware \
#      -Ifirmware/services -Ifirmware/drivers -DRECHORD_UI_TARGET <src>
#
# NOTE on -DRECHORD_UI_TARGET: ui/ui.h reaches services/drivers through
# firmware/ui/internal/hal.h, which serves forward shims unless this macro
# is set. The app consumes the REAL headers, so the macro must be set (see
# docs/rewrite/core.md, "Build wiring").

APP_DIR := firmware/app

APP_SRCS := \
  $(APP_DIR)/main.c \
  $(APP_DIR)/module_registry.c

# Host-side tests (mocks + cases + runner).
APP_TEST_SRCS := \
  $(APP_DIR)/tests/mocks.c \
  $(APP_DIR)/tests/test_bringup.c \
  $(APP_DIR)/tests/test_registry.c \
  $(APP_DIR)/tests/test_boot_params.c \
  $(APP_DIR)/tests/main.c

# Include roots: firmware/ for "app/...", "ui/...", "theme/..." includes;
# services/ + drivers/ so ui/internal/hal.h (RECHORD_UI_TARGET mode) finds
# the real service/driver headers.
APP_INCLUDE_DIRS := firmware firmware/services firmware/drivers
APP_CPPFLAGS     := -DRECHORD_UI_TARGET

# Extra libraries the host tests need (none today).
APP_HOST_LIBS :=
