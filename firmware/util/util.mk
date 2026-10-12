# firmware/util/util.mk — build wiring for ReChord utility modules
# (fragment style per docs/rewrite/architecture.md §2; the root Makefile
# includes it). One source set: the same code links into the target
# firmware and the host tests (tests link it with fs_host.c).

UTIL_DIR := firmware/util

UTIL_SRCS := \
  $(UTIL_DIR)/rechord_log.c

UTIL_INCLUDE_DIRS := \
  $(UTIL_DIR)
