/*
 * SHIM — resolved. The real services/log.h header landed; this shim forwards to it so
 * host tests and target builds share ONE canonical API (duplicate definitions
 * here caused redeclaration clashes during integration).
 *
 * Tests still provide their own mock implementations of these functions.
 */
#ifndef RECHORD_UI_SHIM_LOG_H
#define RECHORD_UI_SHIM_LOG_H

#include "../../../services/log.h"

#endif /* RECHORD_UI_SHIM_LOG_H */
