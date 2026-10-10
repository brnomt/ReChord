/*
 * driverlib_def.h — compatibility wrapper.
 *
 * History: this file used to carry a hand-maintained "minimal" copy of the
 * SDK register/enum definitions (eCLOCK_GATE, eSOFT_RST, IntrType, ...),
 * which collided with the real vendor ../driver/driverlib_def.h whenever a
 * translation unit saw both (they are separate files -> separate include
 * guards -> duplicate enums). Since 2026-10-10 this header simply re-exports
 * the vendor one; if ReChord code needs a definition the vendor lacks, add it
 * HERE below the include with a distinct name instead of redefining vendor
 * symbols.
 */
#ifndef RECHORD_DRIVERLIB_DEF_WRAPPER_H
#define RECHORD_DRIVERLIB_DEF_WRAPPER_H

#include "../driver/driverlib_def.h"

#endif /* RECHORD_DRIVERLIB_DEF_WRAPPER_H */
