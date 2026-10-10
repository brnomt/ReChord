/**
 * wma_audio_parse @ 0x0308cddc
 * Tags: codec, wma
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Restarted to delay deadcode elimination for space: stack */



undefined4 wma_audio_parse(uint *param_1,int param_2)



{

  byte bVar1;

  byte bVar2;

  byte bVar3;

  undefined2 uVar4;

  short sVar5;

  uint uVar6;

  undefined4 *puVar7;

  undefined4 *puVar8;

  short sVar9;

  int iVar10;

  uint uVar11;

  int iVar12;

  uint uVar13;

  uint uVar14;

  uint uVar15;

  int iStack_48;

  short sStack_44;

  short sStack_42;

  byte bStack_40;

  byte bStack_3f;

  byte bStack_3e;

  byte bStack_3d;

  byte bStack_3c;

  byte bStack_3b;

  byte bStack_3a;

  byte bStack_39;

  int iStack_38;

  short sStack_34;

  short sStack_32;

  byte bStack_30;

  byte bStack_2f;

  byte bStack_2e;

  byte bStack_2d;

  byte bStack_2c;

  byte bStack_2b;

  byte bStack_2a;

  byte bStack_29;

  uint *local_28;

  

  uVar15 = 0;

  local_28 = (uint *)0x0;

  if (param_1 == (uint *)0x0) {

    return 2;

  }

  uVar13 = param_2 - 0x18;

  if (uVar13 < 0x36) {

    return 3;

  }

  iVar10 = wma_input_cache_read(param_1,&local_28,*param_1,param_1[1]);

  if (iVar10 != 0x36) {

    return 3;

  }

  iStack_38 = (uint)(byte)*local_28 + (uint)*(byte *)((int)local_28 + 3) * 0x1000000 +

              (uint)*(byte *)((int)local_28 + 2) * 0x10000 +

              (uint)*(byte *)((int)local_28 + 1) * 0x100;

  sStack_34 = (ushort)(byte)local_28[1] + (ushort)*(byte *)((int)local_28 + 5) * 0x100;

  sStack_32 = (ushort)*(byte *)((int)local_28 + 6) + (ushort)*(byte *)((int)local_28 + 7) * 0x100;

  bStack_30 = (byte)local_28[2];

  bStack_2f = *(byte *)((int)local_28 + 9);

  bStack_2e = *(byte *)((int)local_28 + 10);

  bStack_2d = *(byte *)((int)local_28 + 0xb);

  bStack_2c = (byte)local_28[3];

  bStack_2b = *(byte *)((int)local_28 + 0xd);

  bStack_2a = *(byte *)((int)local_28 + 0xe);

  bStack_29 = *(byte *)((int)local_28 + 0xf);

  iStack_48 = (uint)(byte)local_28[4] + (uint)*(byte *)((int)local_28 + 0x13) * 0x1000000 +

              (uint)*(byte *)((int)local_28 + 0x12) * 0x10000 +

              (uint)*(byte *)((int)local_28 + 0x11) * 0x100;

  sStack_44 = (ushort)(byte)local_28[5] + (ushort)*(byte *)((int)local_28 + 0x15) * 0x100;

  sStack_42 = (ushort)*(byte *)((int)local_28 + 0x16) +

              (ushort)*(byte *)((int)local_28 + 0x17) * 0x100;

  bStack_40 = (byte)local_28[6];

  bStack_3f = *(byte *)((int)local_28 + 0x19);

  bStack_3e = *(byte *)((int)local_28 + 0x1a);

  bStack_3d = *(byte *)((int)local_28 + 0x1b);

  bStack_3c = (byte)local_28[7];

  bStack_3b = *(byte *)((int)local_28 + 0x1d);

  bStack_3a = *(byte *)((int)local_28 + 0x1e);

  bStack_39 = *(byte *)((int)local_28 + 0x1f);

  uVar14 = (uint)(byte)local_28[10] + (uint)*(byte *)((int)local_28 + 0x2b) * 0x1000000 +

           (uint)*(byte *)((int)local_28 + 0x2a) * 0x10000 +

           (uint)*(byte *)((int)local_28 + 0x29) * 0x100;

  uVar6 = local_28[0xb];

  bVar1 = *(byte *)((int)local_28 + 0x2f);

  bVar2 = *(byte *)((int)local_28 + 0x2d);

  bVar3 = *(byte *)((int)local_28 + 0x2e);

  uVar11 = local_28[0xc];

  local_28 = (uint *)((int)local_28 + 0x36);

  iVar10 = wma_memcmp(DAT_0308d330,&iStack_38,0x10);

  if (iVar10 == 0) {

    FUN_03081592(s_FILE___s__LINE___d__WMV_0308d36c,s_______Common_Codec_Audio_Wma_wma_0308d338,

                 0x1be);

    return 1;

  }

  iVar10 = wma_memcmp(*DAT_0308d334,&iStack_38,0x10);

  if (iVar10 != 0) {

    uVar15 = *param_1;

    *param_1 = uVar15 + uVar13;

    param_1[1] = param_1[1] + (uint)CARRY4(uVar15,uVar13);

    return 0;

  }

  sVar9 = *(short *)((int)param_1 + 0xea) + 1;

  *(short *)((int)param_1 + 0xea) = sVar9;

  if (sVar9 != (short)param_1[0x3a]) goto LAB_0308cf94;

  *(ushort *)(param_1 + 0x38) = (byte)uVar11 & 0x7f;

  uVar11 = 0x36;

  if (uVar14 != 0) {

    if (uVar13 < uVar14 + 0x36) {

      return 3;

    }

    uVar11 = wma_input_cache_read

                       (param_1,param_1[1],*param_1 + 0x36,

                        param_1[1] + (uint)(0xffffffc9 < *param_1));

    if (uVar11 != uVar14) {

      return 3;

    }

    uVar11 = uVar11 + 0x36;

    sVar9 = (short)*local_28;

    *(short *)((int)param_1 + 0x3e) = sVar9;

    if (sVar9 == 0x160) {

      if (uVar14 < 0x16) {

        return 1;

      }

      *(undefined2 *)(param_1 + 0xf) = 1;

      param_1[0x10] =

           (uint)*(byte *)((int)local_28 + 6) * 0x10000 + (uint)*(byte *)((int)local_28 + 5) * 0x100

           + (uint)(byte)local_28[1] + (uint)*(byte *)((int)local_28 + 7) * 0x1000000;

      param_1[0x11] =

           (uint)*(byte *)((int)local_28 + 10) * 0x10000 +

           (uint)*(byte *)((int)local_28 + 9) * 0x100 +

           (uint)(byte)local_28[2] + (uint)*(byte *)((int)local_28 + 0xb) * 0x1000000;

      param_1[0x12] = (uint)(ushort)local_28[3];

      sVar5 = *(short *)((int)local_28 + 2);

      *(short *)(param_1 + 0x13) = sVar5;

      *(ushort *)(param_1 + 0x15) =

           (ushort)(byte)local_28[5] + (ushort)*(byte *)((int)local_28 + 0x15) * 0x100;

      param_1[0x14] =

           (uint)*(byte *)((int)local_28 + 0x12) + (uint)*(byte *)((int)local_28 + 0x13) * 0x100;

      *(undefined2 *)((int)param_1 + 0x56) = 1;

      sVar9 = (ushort)*(byte *)((int)local_28 + 0xe) +

              (ushort)*(byte *)((int)local_28 + 0xf) * 0x100;

      *(short *)(param_1 + 0x16) = sVar9;

      *(short *)((int)param_1 + 0x5a) = sVar9;

      *(short *)(param_1 + 0x1c) = sVar9;

      if (sVar5 == 1) {

LAB_0308d18a:

        param_1[0x17] = 4;

      }

      else {

        if (sVar5 != 2) {

          return 1;

        }

LAB_0308d18e:

        param_1[0x17] = 3;

      }

    }

    else if (sVar9 == 0x161) {

      if (uVar14 < 0x1c) {

        return 1;

      }

      *(undefined2 *)(param_1 + 0xf) = 2;

      param_1[0x10] =

           (uint)*(byte *)((int)local_28 + 6) * 0x10000 + (uint)*(byte *)((int)local_28 + 5) * 0x100

           + (uint)(byte)local_28[1] + (uint)*(byte *)((int)local_28 + 7) * 0x1000000;

      param_1[0x11] =

           (uint)*(byte *)((int)local_28 + 10) * 0x10000 +

           (uint)*(byte *)((int)local_28 + 9) * 0x100 +

           (uint)(byte)local_28[2] + (uint)*(byte *)((int)local_28 + 0xb) * 0x1000000;

      param_1[0x12] = (uint)(ushort)local_28[3];

      sVar9 = *(short *)((int)local_28 + 2);

      *(short *)(param_1 + 0x13) = sVar9;

      *(undefined2 *)((int)param_1 + 0x56) = 1;

      uVar4 = *(undefined2 *)((int)local_28 + 0xe);

      *(undefined2 *)(param_1 + 0x16) = uVar4;

      *(undefined2 *)((int)param_1 + 0x5a) = uVar4;

      *(undefined2 *)(param_1 + 0x1c) = uVar4;

      param_1[0x14] =

           (uint)(byte)local_28[5] * 0x10000 + (uint)*(byte *)((int)local_28 + 0x13) * 0x100 +

           (uint)*(byte *)((int)local_28 + 0x12) + (uint)*(byte *)((int)local_28 + 0x15) * 0x1000000

      ;

      *(ushort *)(param_1 + 0x15) =

           (ushort)*(byte *)((int)local_28 + 0x16) + (ushort)*(byte *)((int)local_28 + 0x17) * 0x100

      ;

      if (sVar9 == 1) goto LAB_0308d18a;

      if (sVar9 == 2) goto LAB_0308d18e;

      if (sVar9 != 6) {

        return 1;

      }

      param_1[0x17] = 0x3f;

    }

    else {

      if (sVar9 != 0x162 && sVar9 != 0x163) {

        return 1;

      }

      if (uVar14 < 0x24) {

        return 1;

      }

      *(undefined2 *)(param_1 + 0xf) = 3;

      param_1[0x10] =

           (uint)*(byte *)((int)local_28 + 6) * 0x10000 + (uint)*(byte *)((int)local_28 + 5) * 0x100

           + (uint)(byte)local_28[1] + (uint)*(byte *)((int)local_28 + 7) * 0x1000000;

      param_1[0x11] =

           (uint)*(byte *)((int)local_28 + 10) * 0x10000 +

           (uint)*(byte *)((int)local_28 + 9) * 0x100 +

           (uint)(byte)local_28[2] + (uint)*(byte *)((int)local_28 + 0xb) * 0x1000000;

      param_1[0x12] = (uint)(ushort)local_28[3];

      *(ushort *)(param_1 + 0x13) =

           (ushort)*(byte *)((int)local_28 + 2) + (ushort)*(byte *)((int)local_28 + 3) * 0x100;

      *(undefined2 *)((int)param_1 + 0x56) = 0xfffe;

      sVar9 = *(short *)((int)local_28 + 0xe);

      *(short *)((int)param_1 + 0x5a) = sVar9;

      *(ushort *)(param_1 + 0x16) = sVar9 + 7U & 0xfff8;

      param_1[0x17] =

           (uint)*(byte *)((int)local_28 + 0x16) * 0x10000 +

           (uint)*(byte *)((int)local_28 + 0x15) * 0x100 +

           (uint)(byte)local_28[5] + (uint)*(byte *)((int)local_28 + 0x17) * 0x1000000;

      *(ushort *)(param_1 + 0x15) =

           (ushort)(byte)local_28[8] + (ushort)*(byte *)((int)local_28 + 0x21) * 0x100;

      param_1[0x18] = 1;

      *(undefined2 *)(param_1 + 0x19) = 0;

      *(undefined2 *)((int)param_1 + 0x66) = 0x10;

      *(undefined1 *)(param_1 + 0x1a) = 0x80;

      *(undefined1 *)((int)param_1 + 0x69) = 0;

      *(undefined1 *)((int)param_1 + 0x6a) = 0;

      *(undefined1 *)((int)param_1 + 0x6b) = 0xaa;

      *(undefined1 *)(param_1 + 0x1b) = 0;

      *(undefined1 *)((int)param_1 + 0x6d) = 0x38;

      *(undefined1 *)((int)param_1 + 0x6e) = 0x9b;

      *(undefined1 *)((int)param_1 + 0x6f) = 0x71;

    }

  }

  puVar7 = DAT_0308d384;

  if ((uint)(byte)uVar6 + (uint)bVar1 * 0x1000000 + (uint)bVar3 * 0x10000 + (uint)bVar2 * 0x100 != 0

     ) {

    iVar10 = wma_memcmp(*DAT_0308d384,&iStack_48,0x10);

    puVar8 = DAT_0308d388;

    if (iVar10 == 0) {

      iVar10 = 9;

    }

    else {

      iVar10 = wma_memcmp(*DAT_0308d388,&iStack_48,0x10);

      if (iVar10 != 0) {

        return 1;

      }

      iVar10 = 8;

    }

    if ((uVar13 < uVar11 + iVar10) ||

       (iVar12 = wma_input_cache_read

                           (param_1,param_1[1],*param_1 + uVar11,

                            param_1[1] + (uint)CARRY4(*param_1,uVar11)), iVar12 != iVar10)) {

      return 3;

    }

    iVar10 = wma_memcmp(*puVar7,&iStack_48,0x10);

    if (iVar10 == 0) {

      param_1[6] = *local_28;

    }

    else {

      iVar10 = wma_memcmp(*puVar8,&iStack_48,0x10);

      if ((iVar10 != 0) ||

         (param_1[6] = (uint)(byte)*local_28 * (uint)*(byte *)((int)local_28 + 1),

         1 < (byte)*local_28)) {

        return 1;

      }

    }

  }

  if (param_1[0x12] != 0) {

    uVar15 = param_1[6] / param_1[0x12];

  }

  param_1[6] = (uint)(ushort)param_1[0x13] * param_1[0x14] * uVar15 * 2;

LAB_0308cf94:

  uVar15 = *param_1;

  *param_1 = uVar15 + uVar13;

  param_1[1] = param_1[1] + (uint)CARRY4(uVar15,uVar13);

  return 0;

}
