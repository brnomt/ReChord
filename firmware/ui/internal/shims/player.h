/*
 * SHIM — forwards to the canonical player API (firmware/player/player.h),
 * same pattern as shims/fs.h: ONE API for host tests and target builds.
 *
 * Tests provide their own mock implementations of the player_* functions
 * (tests/mock_services.c) so the UI links without the real player module.
 */
#ifndef RECHORD_UI_SHIM_PLAYER_H
#define RECHORD_UI_SHIM_PLAYER_H

#include "../../../player/player.h"

#endif /* RECHORD_UI_SHIM_PLAYER_H */
