/*
 * File.h - compatibility wrapper.
 * The old synthesized prototypes here (int FileOpen(void*) style) clashed
 * with the vendor filesys/file.h signatures whenever both were seen.
 * Since 2026-10-10 this re-exports the vendor header; file.c provides the
 * implementations.
 */
#ifndef RECHORD_FILE_WRAPPER_H
#define RECHORD_FILE_WRAPPER_H

#include "filesys/file.h"

#endif /* RECHORD_FILE_WRAPPER_H */
