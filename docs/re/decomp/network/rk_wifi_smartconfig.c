/**
 * rk_wifi_smartconfig @ 0x0301b10a
 * Structural-match v2 (validated by SDK-callee overlap).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 rk_wifi_smartconfig(void)



{

  int iVar1;

  int iVar2;

  

  iVar2 = 100;

  func_0x0301b22a(0);

  FUN_0301b270(DAT_0301b16c + -4);

  iVar1 = FUN_02ff6a24(2);

  while (iVar1 == 1) {

    FUN_030060e4(1);

    iVar2 = iVar2 + -1;

    if (iVar2 == 0) break;

    iVar1 = FUN_02ff6a24(2);

  }

  rk_printf(2,*DAT_0301b164,DAT_0301b168,DAT_0301b164[1],DAT_0301b160,&DAT_0301af67);

  return 1;

}
