/**
 * aac_aac_dec @ 0x030ad690
 * Tags: codec, aac
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */



uint aac_aac_dec(int param_1,undefined4 param_2,undefined4 param_3)



{

  int *piVar1;

  undefined4 *puVar2;

  undefined4 *puVar3;

  undefined4 *puVar4;

  int *piVar5;

  int iVar6;

  uint uVar7;

  uint uVar8;

  int iVar9;

  bool bVar10;

  undefined8 uVar11;

  undefined8 uVar12;

  byte local_3c;

  byte local_3b [3];

  undefined1 auStack_38 [2];

  byte local_36;

  byte local_35;

  int iStack_30;

  undefined4 uStack_2c;

  undefined4 local_28;

  

  piVar1 = DAT_030ad6dc;

  iVar6 = param_1;

  if (DAT_030ad6dc[7] < param_1) {

    iVar6 = DAT_030ad6dc[7];

  }

  iStack_30 = param_1;

  uStack_2c = param_2;

  local_28 = param_3;

  uVar11 = FUN_030acaf4(*DAT_030ad6dc);

  uVar12 = FUN_030acaf4(piVar1[3]);

  uVar12 = FUN_030ac932((int)uVar12,(int)((ulonglong)uVar12 >> 0x20),0,_DAT_030ad6f0);

  uVar11 = FUN_030aca16((int)uVar12,(int)((ulonglong)uVar12 >> 0x20),(int)uVar11,

                        (int)((ulonglong)uVar11 >> 0x20));

  uVar12 = FUN_030acaf4(iVar6);

  FUN_030aca16((int)uVar12,(int)((ulonglong)uVar12 >> 0x20),(int)uVar11,

               (int)((ulonglong)uVar11 >> 0x20));

  iVar6 = SDDecodeCSD();

  puVar2 = DAT_030adaf8;

  if (piVar1[2] < iVar6) {

    iVar6 = piVar1[2];

  }

  uVar8 = piVar1[4] * iVar6 + *DAT_030adaf4;

  (*(code *)*DAT_030adaf8)(uVar8,0,local_28);

  puVar3 = DAT_030adafc;

  iVar6 = (*(code *)*DAT_030adafc)(&local_3c,1,local_28);

  puVar4 = DAT_030adb5c;

  if (0 < iVar6) {

    iVar6 = (*(code *)*DAT_030adb5c)(local_28);

    piVar5 = DAT_030adb60;

    while( true ) {

      if (iVar6 != 0) {

        hifi_debug_printf_ovl

                  (s_FILE___s__LINE___d__flag____0_030adba4,

                   s_______Common_Codec_Audio_AAC_lib_030adb00,0x1dd);

        return 0xffffffff;

      }

      iVar6 = (*(code *)*puVar3)(local_3b,1,local_28);

      if (iVar6 < 1) break;

      uVar7 = (uint)local_3c;

      bVar10 = uVar7 == 0xff;

      if (bVar10) {

        uVar7 = (uint)local_3b[0];

      }

      if (bVar10 && uVar7 >> 4 == 0xf) {

        (*(code *)*puVar2)(0xfffffffe,1,local_28);

        FUN_030ab3d6(auStack_38,0,4);

        (*(code *)*puVar3)(auStack_38,4,local_28);

        (*(code *)*puVar2)(0xfffffffe,1,local_28);

        if ((*(int *)(*piVar5 + ((local_36 & 1) << 2 | (uint)(local_35 >> 6)) * 4) == piVar1[1]) &&

           (*piVar1 == *(int *)(*DAT_030adb64 + (local_36 & 0x3c)))) {

          (*(code *)*puVar2)(0xfffffffe,1,local_28);

          iVar9 = uVar8 / (uint)piVar1[4] + 1;

          piVar1[5] = iVar9;

          uVar11 = FUN_030acaf4(*piVar1);

          uVar12 = FUN_030acaf4(piVar1[3]);

          uVar12 = FUN_030ac932((int)uVar12,(int)((ulonglong)uVar12 >> 0x20),0,_DAT_030adbc4);

          uVar11 = FUN_030aca16((int)uVar12,(int)((ulonglong)uVar12 >> 0x20),(int)uVar11,

                                (int)((ulonglong)uVar11 >> 0x20));

          uVar12 = FUN_030acaf4(iVar9);

          FUN_030ac932((int)uVar12,(int)((ulonglong)uVar12 >> 0x20),(int)uVar11,

                       (int)((ulonglong)uVar11 >> 0x20));

          iVar6 = SDDecodeCSD();

          piVar1[6] = iVar6;

          hifi_debug_printf_ovl

                    (s__FILE___s__LINE___d__curFrame_____030adbc7 + 1,

                     s_______Common_Codec_Audio_AAC_lib_030adb00,0x1e6,iVar9,iVar6);

          return uVar8;

        }

        hifi_debug_printf_ovl(s_seek_position_is_not_correct__i_w_030adb68);

      }

      local_3c = local_3b[0];

      iVar6 = (*(code *)*puVar4)(local_28);

      uVar8 = uVar8 + 1;

    }

    hifi_debug_printf_ovl

              (s_FILE___s__LINE___d__rkfile_read_r_030adb30,

               s_______Common_Codec_Audio_AAC_lib_030adb00,0x1b9);

    return 0xffffffff;

  }

  hifi_debug_printf_ovl

            (s_FILE___s__LINE___d__rkfile_read_r_030adb30,

             s_______Common_Codec_Audio_AAC_lib_030adb00,0x1ad);

  return 0xffffffff;

}
