/**
 * HifiFileClose @ 0x0306bade
 * Tags: fs
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void HifiFileClose(void)



{

  int *piVar1;

  

  piVar1 = DAT_0306bc38;

  if (*DAT_0306bc38 != 0) {

    return;

  }

  FUN_0306c878(0x17);

  FUN_0306c878(0x11);

  if (*piVar1 != 0) {

    return;

  }

  FUN_0306c6c4(0x17,DAT_0306bc3c);

  FUN_0306c6c4(0x11,DAT_0306bc40);

  if (*piVar1 != 0) {

    return;

  }

  FUN_0306c772(0x17);

  FUN_0306c772(0x11);

  return;

}
