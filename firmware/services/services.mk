# firmware/services/services.mk — build wiring for the services module
# (fragment style per docs/rewrite/architecture.md §2; included by the root
# Makefile once services land in the firmware image).
#
# Exactly ONE fs backend is linked per image:
#   target firmware -> fs_target.c (SDK FAT TODO stub for now)
#   host tools/tests -> fs_host.c  (POSIX backend)

SERVICES_DIR := firmware/services

SERVICES_COMMON_SRCS := \
  $(SERVICES_DIR)/settings.c \
  $(SERVICES_DIR)/log.c \
  $(SERVICES_DIR)/input.c

# Sources compiled for the ARM target image.
SERVICES_TARGET_SRCS := \
  $(SERVICES_COMMON_SRCS) \
  $(SERVICES_DIR)/fs_target.c

# Sources compiled for host tests/tools.
SERVICES_HOST_SRCS := \
  $(SERVICES_COMMON_SRCS) \
  $(SERVICES_DIR)/fs_host.c

SERVICES_INCLUDE_DIRS := $(SERVICES_DIR)
