# firmware/libc/libc.mk — build wiring for the freestanding libc module
# (fragment style per docs/rewrite/architecture.md §2). One source set: the
# same code links into target firmware and host tests (host tests may
# override rechord_console_write).

LIBC_DIR := firmware/libc

LIBC_SRCS := \
  $(LIBC_DIR)/mem.c \
  $(LIBC_DIR)/str.c \
  $(LIBC_DIR)/stdio_min.c \
  $(LIBC_DIR)/math_min.c

LIBC_INCLUDE_DIRS := $(LIBC_DIR)
