/**
 * hifi_flac_helper @ 0x030df52a
 * Tags: codec, flac
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 hifi_flac_helper(int *param_1,int param_2,undefined4 param_3,int param_4)



{

  int iVar1;

  int iVar2;

  

  iVar1 = FUN_030dd8b8();

  if (iVar1 < 0) {

    hifi_debug_printf_ovl_0dc5(&DAT_030df5b0);

    param_4 = param_1[2];

    hifi_debug_printf_ovl_0dc5

              (s_FILE___s__LINE___d__gb_>size_in__030df5f4,

               s_______Common_Codec_Audio_HIFI_fl_030df5c0,0x3e4,param_1[3],param_4);

    iVar1 = 0;

  }

  if (iVar1 < param_2) {

    iVar2 = iVar1 + 7 >> 3;

    iVar1 = *param_1;

    FUN_030dc25a(iVar1,iVar1 + (param_1[2] >> 3),iVar2,iVar1,param_4);

    iVar1 = (*(code *)*DAT_030df5ac)(*param_1 + iVar2,0x4000 - iVar2,*DAT_030df5a4);

    param_1[2] = param_1[2] % 8;

    if (iVar1 == 0) {

      return 0;

    }

  }

  return 1;

}
