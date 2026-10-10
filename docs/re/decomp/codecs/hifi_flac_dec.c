/**
 * hifi_flac_dec @ 0x030df64c
 * Tags: codec, flac
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 hifi_flac_dec(int param_1,int param_2,int param_3)



{

  int iVar1;

  uint uVar2;

  int iVar3;

  int iVar4;

  uint uVar5;

  undefined4 uVar6;

  int iVar7;

  int iVar8;

  int iVar9;

  undefined4 uVar10;

  int iVar11;

  int iVar12;

  uint uVar13;

  uint uVar14;

  bool bVar15;

  bool bVar16;

  int local_40;

  

  hifi_flac_helper(param_1 + 0x20,6);

  iVar1 = flac_bitstream_getbits_u(param_1 + 0x20,2);

  if (1 < iVar1) {

    hifi_debug_printf_ovl_0dc5

              (s_FILE___s__LINE___d____Error___030dfa74,s_______Common_Codec_Audio_HIFI_fl_030df5c0,

               0x150);

    return 0xffffffff;

  }

  uVar2 = flac_bitstream_getbits_u(param_1 + 0x20,4);

  iVar9 = *(int *)(param_1 + 0x30) >> (uVar2 & 0xff);

  if (iVar9 < param_3) {

    hifi_debug_printf_ovl_0dc5

              (s_FILE___s__LINE___d____Error___030dfa74,s_______Common_Codec_Audio_HIFI_fl_030df5c0,

               0x159);

    return 0xffffffff;

  }

  local_40 = 0;

  iVar3 = 1 << (uVar2 & 0xff);

  if (0 < iVar3) {

    iVar4 = param_1 + param_2 * 0x4800;

    iVar12 = param_3;

    do {

      hifi_flac_helper(param_1 + 0x20,10);

      if (iVar1 == 0) {

        uVar10 = 4;

      }

      else {

        uVar10 = 5;

      }

      uVar2 = flac_bitstream_getbits_u(param_1 + 0x20,uVar10);

      if (iVar1 == 0) {

        uVar5 = 0xf;

      }

      else {

        uVar5 = 0x1f;

      }

      if (uVar5 == uVar2) {

        uVar10 = flac_bitstream_getbits_u(param_1 + 0x20,5);

        for (; param_3 < iVar9; param_3 = param_3 + 1) {

          hifi_flac_helper(param_1 + 0x20,uVar10);

          uVar6 = FUN_030dd842(param_1 + 0x20,uVar10);

          iVar7 = iVar12 * 4;

          iVar12 = iVar12 + 1;

          *(undefined4 *)(iVar4 + iVar7 + 0x48) = uVar6;

        }

      }

      else {

        for (; param_3 < iVar9; param_3 = param_3 + 1) {

          hifi_flac_helper(param_1 + 0x20,0x100);

          uVar13 = *(uint *)(param_1 + 0x28);

          iVar7 = FUN_030dd6d0(*(int *)(param_1 + 0x20) + ((int)uVar13 >> 3));

          uVar14 = iVar7 << (uVar13 & 7);

          iVar8 = FUN_030dd956(uVar14 | 1);

          iVar8 = 0x1f - iVar8;

          uVar5 = iVar8 - uVar2;

          bVar16 = SBORROW4(uVar5,7);

          iVar7 = uVar5 - 7;

          bVar15 = uVar5 == 7;

          if (6 < (int)uVar5) {

            iVar11 = 0x20 - iVar8;

            bVar16 = SBORROW4(0x7fffffff,iVar11);

            iVar7 = 0x7fffffff - iVar11;

            bVar15 = iVar11 == 0x7fffffff;

          }

          if (bVar15 || iVar7 < 0 != bVar16) {

            iVar7 = 0;

            do {

              if ((int)uVar14 < 0) break;

              uVar13 = uVar13 + 1;

              iVar8 = FUN_030dd6d0(*(int *)(param_1 + 0x20) + ((int)uVar13 >> 3));

              uVar14 = iVar8 << (uVar13 & 7);

              iVar7 = iVar7 + 1;

            } while (iVar7 < 0x7fffffff);

            iVar8 = uVar13 + 1;

            uVar14 = uVar14 << 1;

            if (iVar7 == 0x7fffffff) {

              uVar14 = 0xffffffff;

            }

            if (iVar7 < 0x7ffffffe) {

              if (uVar2 == 0) {

                uVar14 = 0;

              }

              else {

                uVar14 = uVar14 >> (0x20 - uVar2 & 0xff);

                iVar8 = iVar8 + uVar2;

              }

              *(int *)(param_1 + 0x28) = iVar8;

              uVar14 = uVar14 + (iVar7 << (uVar2 & 0xff));

            }

            else if (iVar7 == 0x7ffffffe) {

              uVar14 = 1;

              *(int *)(param_1 + 0x28) = iVar8;

            }

          }

          else {

            uVar14 = (uVar14 >> (uVar5 & 0xff)) + (0x1e - iVar8 << (uVar2 & 0xff));

            *(uint *)(param_1 + 0x28) = (uVar2 - iVar8) + uVar13 + 0x20;

          }

          iVar7 = iVar12 * 4;

          iVar12 = iVar12 + 1;

          *(uint *)(iVar4 + iVar7 + 0x48) = -(uVar14 & 1) ^ (int)uVar14 >> 1;

        }

      }

      param_3 = 0;

      local_40 = local_40 + 1;

    } while (local_40 < iVar3);

  }

  return 0;

}
