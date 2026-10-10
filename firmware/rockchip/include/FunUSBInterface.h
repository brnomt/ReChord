/* FunUSBInterface.h — USB interface hooks (Service.c).
 *
 * 2026-10-10: the old synthesized UHC_CHN_INFO here (duplicated twice, and
 * missing the vendor's ConnectStatus member) is gone - the vendor
 * driver/USB/host/USB_DWCHost.h owns UHC_CHN_INFO and this re-exports it.
 */
#ifndef FUNUSBINTERFACE_H
#define FUNUSBINTERFACE_H

#include "typedef.h"
#include "driver/USB/host/USB_DWCHost.h"   /* UHC_CHN_INFO (full vendor struct) */

#endif /* FUNUSBINTERFACE_H */
