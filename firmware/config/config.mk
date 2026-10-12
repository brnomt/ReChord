# firmware/config/config.mk — build wiring for the ReChord persistent
# config module (fragment style per docs/rewrite/architecture.md §2; the
# root Makefile includes it). One source set: the same code links into the
# target firmware and the host tests (tests link it with fs_host.c).

CONFIG_DIR := firmware/config

CONFIG_SRCS := \
  $(CONFIG_DIR)/rechord_config.c

CONFIG_INCLUDE_DIRS := \
  $(CONFIG_DIR)
