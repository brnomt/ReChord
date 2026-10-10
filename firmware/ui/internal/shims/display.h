/*
 * SHIM — resolved. The real drivers/display.h header landed; this shim forwards to it so
 * host tests and target builds share ONE canonical API (duplicate definitions
 * here caused redeclaration clashes during integration).
 *
 * Tests still provide their own mock implementations of these functions.
 */
#ifndef RECHORD_UI_SHIM_DISPLAY_H
#define RECHORD_UI_SHIM_DISPLAY_H

#include "../../../drivers/display.h"

#endif /* RECHORD_UI_SHIM_DISPLAY_H */
