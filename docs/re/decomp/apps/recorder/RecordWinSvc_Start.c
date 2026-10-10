/**
 * RecordWinSvc_Start @ 0x0300aabc
 * Tags: recording, service
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void RecordWinSvc_Start(undefined4 param_1)



{

  int iVar1;

  

  iVar1 = FUN_02fe878c(*DAT_0300ab10,DAT_0300ab0c);

  if (iVar1 != 1) {

    FUN_02ff9d4c(0x3f,7);

    FUN_02fe86a6(DAT_0300ab10,DAT_0300ab0c,param_1);

    FUN_02ff9700(0x75);

    FUN_02fff884(s_RecordWinSvcStart_0300ab14);

    FUN_02ff9700(0x5a);

    return;

  }

  FUN_02fff884(s_Record_again_0300ab2c);

  FUN_02ff9700(0x5b);

  FUN_02ff9700(0x5a);

  return;

}
