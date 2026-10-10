/*
 * SHIM — resolved. The real services/input.h header landed; this shim forwards to it so
 * host tests and target builds share ONE canonical API (duplicate definitions
 * here caused redeclaration clashes during integration).
 *
 * Tests still provide their own mock implementations of these functions.
 */
#ifndef RECHORD_UI_SHIM_INPUT_H
#define RECHORD_UI_SHIM_INPUT_H

#include "../../../services/input.h"

#endif /* RECHORD_UI_SHIM_INPUT_H */
