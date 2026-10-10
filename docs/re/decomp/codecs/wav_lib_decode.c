/**
 * wav_lib_decode @ 0x030a42ee
 * Tags: codec, wav
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 wav_lib_decode(uint *param_1,uint param_2)



{

  char cVar1;

  undefined4 *puVar2;

  uint uVar3;

  uint uVar4;

  uint uVar5;

  int iVar6;

  bool bVar7;

  bool bVar8;

  undefined4 local_78;

  uint local_74;

  uint local_70;

  undefined4 local_6c;

  undefined4 local_68;

  uint local_64;

  uint local_60;

  uint local_5c;

  uint local_58;

  uint local_54;

  uint local_50;

  uint local_4c;

  uint local_48;

  uint local_40;

  uint uStack_3c;

  uint uStack_38;

  uint uStack_34;

  uint local_30;

  uint uStack_2c;

  uint local_28;

  uint uStack_24;

  uint local_20;

  uint uStack_1c;

  

  puVar2 = DAT_030a4534;

  local_78 = 0;

  local_74 = 0;

  local_70 = 0;

  local_6c = 0;

  local_68 = 0;

  local_64 = 0;

  local_60 = 0;

  local_5c = 0;

  local_58 = 0;

  local_54 = 0;

  local_50 = 0;

  local_4c = 0;

  local_48 = 0;

  (*(code *)*DAT_030a4538)(param_1[0x2085],0,*DAT_030a4534);

  uVar3 = (*(code *)*DAT_030a453c)(param_1 + 2,param_2,*puVar2);

  if (uVar3 == param_2) {

    param_1[0x2085] = param_1[0x2085] + param_2;

    hifi_debug_printf_ovl_09e0

              (s_FILE___s__LINE___d__wav_formatTa_030a459c,

               s_______Common_Codec_Audio_Wav_WAV_030a4554,0x106,(char)param_1[2]);

    cVar1 = (char)param_1[2];

    if (cVar1 == '\x02') {

      if (param_2 == 0x32) {

        local_74 = param_1[3];

        local_70 = param_1[4];

        uVar3 = param_1[5];

        uVar5 = param_1[6];

        local_78._2_2_ = (ushort)(param_1[2] >> 0x10);

        bVar8 = 1 < local_78._2_2_;

        bVar7 = local_78._2_2_ == 2;

        if (local_78._2_2_ < 3) {

          bVar8 = 0xfff < (uVar3 & 0xffff);

          bVar7 = (uVar3 & 0xffff) == 0x1000;

        }

        if (!bVar8 || bVar7) {

          uVar4 = uVar3 >> 0x10;

          bVar7 = uVar4 == 4;

          if (bVar7) {

            uVar4 = uVar5 & 0xffff;

          }

          if (bVar7 && uVar4 == 0x20) {

            local_68._2_2_ = (ushort)(uVar5 >> 0x10);

            bVar7 = local_68._2_2_ == 0x800;

            if (local_68._2_2_ < 0x801) {

              bVar7 = (param_1[7] & 0xffff) == 7;

            }

            local_78 = param_1[2];

            local_68 = uVar5;

            if (bVar7) goto LAB_030a44fc;

          }

        }

      }

    }

    else if (cVar1 == '\x11') {

      local_74 = param_1[3];

      local_70 = param_1[4];

      uVar3 = param_1[5];

      uVar5 = param_1[6];

      local_78._2_2_ = (ushort)(param_1[2] >> 0x10);

      bVar8 = 1 < local_78._2_2_;

      bVar7 = local_78._2_2_ == 2;

      if (local_78._2_2_ < 3) {

        bVar8 = 0xfff < (uVar3 & 0xffff);

        bVar7 = (uVar3 & 0xffff) == 0x1000;

      }

      if (!bVar8 || bVar7) {

        bVar7 = (uVar5 & 0xffff) == 2;

      }

      if (((bVar7) && (local_68._2_2_ = (ushort)(uVar5 >> 0x10), local_68._2_2_ < 0x1001)) &&

         (local_6c._2_2_ = (short)(uVar3 >> 0x10), local_78 = param_1[2], local_68 = uVar5,

         local_6c._2_2_ == 3 || local_6c._2_2_ == 4)) {

LAB_030a44fc:

        local_6c = uVar3;

        param_1[0x208a] = local_70;

        param_1[0x208b] = local_74;

        *(undefined2 *)((int)param_1 + 0x8232) = (undefined2)local_6c;

        *(char *)(param_1 + 0x208c) = (char)(local_78 >> 0x10);

        *param_1 = local_78 >> 0x10;

        *(ushort *)(param_1 + 0x208d) = local_68._2_2_;

        *(ushort *)(param_1 + 1) = local_68._2_2_;

        *(short *)((int)param_1 + 0x8236) = local_6c._2_2_;

        *(short *)((int)param_1 + 6) = local_6c._2_2_;

        *(undefined2 *)(param_1 + 0x208e) = (undefined2)local_78;

        return 1;

      }

    }

    else if (cVar1 == '\x01') {

      if (param_2 < 0x35) {

        FUN_0309f692(&local_78,param_1 + 2,param_2);

      }

      else {

        local_78 = param_1[2];

        local_74 = param_1[3];

        local_70 = param_1[4];

        local_6c = param_1[5];

        local_68 = param_1[6];

        local_64 = param_1[7];

        local_60 = param_1[8];

        local_5c = param_1[9];

        local_58 = param_1[10];

        local_54 = param_1[0xb];

        local_50 = param_1[0xc];

        local_4c = param_1[0xd];

        local_48 = param_1[0xe];

      }

      bVar8 = 1 < local_78._2_2_;

      bVar7 = local_78._2_2_ == 2;

      if (local_78._2_2_ < 3) {

        bVar8 = 0x2fff < (local_6c & 0xffff);

        bVar7 = (local_6c & 0xffff) == 0x3000;

      }

      if ((!bVar8 || bVar7) &&

         ((uVar3 = local_6c, local_6c._2_2_ == 8 || local_6c._2_2_ == 0x10 ||

          (local_6c._2_2_ == 0x18 || local_6c._2_2_ == 0x20)))) goto LAB_030a44fc;

      hifi_debug_printf_ovl_09e0

                (s_FILE___s__LINE___d__nBlockAlign___030a45c8,

                 s_______Common_Codec_Audio_Wav_WAV_030a4554,0x17a,local_6c & 0xffff);

    }

    else if (*(char *)((int)param_1 + 9) == -1 && cVar1 == -2) {

      local_40 = param_1[2];

      uStack_3c = param_1[3];

      uStack_38 = param_1[4];

      uStack_34 = param_1[5];

      local_30 = param_1[6];

      uStack_2c = param_1[7];

      local_28 = param_1[8];

      uStack_24 = param_1[9];

      local_20 = param_1[10];

      uStack_1c = param_1[0xb];

      iVar6 = FUN_0309f6e8(&uStack_24,DAT_030a45f0,0xc);

      if (iVar6 == 0) {

        local_74 = uStack_3c;

        local_70 = uStack_38;

        local_6c = uStack_34;

        uVar3 = local_6c;

        local_68 = local_30;

        local_78._2_2_ = (ushort)(local_40 >> 0x10);

        local_78 = CONCAT22(local_78._2_2_,(short)local_28);

        if ((local_28 & 0xffff) == 1) {

          bVar8 = 1 < local_78._2_2_;

          bVar7 = local_78._2_2_ == 2;

          if (local_78._2_2_ < 3) {

            bVar8 = 0xfff < (uStack_34 & 0xffff);

            bVar7 = (uStack_34 & 0xffff) == 0x1000;

          }

          if ((!bVar8 || bVar7) &&

             ((local_6c._2_2_ = (short)(uStack_34 >> 0x10),

              local_6c._2_2_ == 8 || local_6c._2_2_ == 0x10 ||

              (local_6c._2_2_ == 0x18 || local_6c._2_2_ == 0x20)))) goto LAB_030a44fc;

        }

      }

    }

  }

  else {

    hifi_debug_printf_ovl_09e0

              (s_FILE___s__LINE___d__030a4584,s_______Common_Codec_Audio_Wav_WAV_030a4554,0xff);

  }

  return 0;

}
