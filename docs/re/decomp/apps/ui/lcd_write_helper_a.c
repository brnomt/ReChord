/**
 * lcd_write_helper_a @ 0x03028c9e
 * Named via cross-version lineage (v3.4.0/v3.0.0 clusters); decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void lcd_write_helper_a(short *param_1,undefined2 *param_2)



{

  int iVar1;

  

  iVar1 = (*param_1 + 4 >> 3) + 0x80;

  if (iVar1 < 0) {

    iVar1 = 0;

  }

  else if (0xff < iVar1) {

    iVar1 = 0xff;

  }

  *param_2 = (short)iVar1;

  return;

}
