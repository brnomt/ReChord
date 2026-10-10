/*
 * SHIM — resolved. The real services/settings.h header landed; this shim forwards to it so
 * host tests and target builds share ONE canonical API (duplicate definitions
 * here caused redeclaration clashes during integration).
 *
 * Tests still provide their own mock implementations of these functions.
 */
#ifndef RECHORD_UI_SHIM_SETTINGS_H
#define RECHORD_UI_SHIM_SETTINGS_H

#include "../../../services/settings.h"

#endif /* RECHORD_UI_SHIM_SETTINGS_H */
