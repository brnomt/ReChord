/**
 * dict_build_frame_index @ 0x0304f90a
 * Tags: codec, parser
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Restarted to delay deadcode elimination for space: stack */



undefined4 dict_build_frame_index(short *param_1,int param_2,int param_3)



{

  short sVar1;

  uint uVar2;

  int iVar3;

  int iVar4;

  int iVar5;

  int iVar6;

  int iVar7;

  undefined4 uVar8;

  undefined1 local_48 [4];

  undefined4 local_44;

  uint local_40;

  int iStack_3c;

  int local_38;

  int iStack_34;

  short *psStack_30;

  int local_2c;

  int iStack_28;

  

  uVar8 = 2;

  if ((((param_1 != (short *)0x0) && (param_2 != -1 && param_3 != 0)) &&

      (psStack_30 = param_1, local_2c = param_2, iStack_28 = param_3,

      uVar2 = FUN_0304fe5c(param_1,param_2), uVar2 != 0x7fffffff)) &&

     ((-1 < (int)uVar2 && ((int)(uVar2 & 0x3fffff) < *(int *)(param_1 + 4))))) {

    if ((uVar2 & 0x400000) != 0) {

      uVar2 = uVar2 & 0x3fffff;

      uVar8 = 1;

    }

    if (*(uint **)(param_3 + 0xc) != (uint *)0x0) {

      **(uint **)(param_3 + 0xc) = uVar2;

    }

    sVar1 = *param_1;

    local_44 = 1;

    iStack_3c = uVar2 * (ushort)param_1[8] +

                (uint)(ushort)param_1[6] * (uint)(ushort)param_1[7] + (uint)(ushort)param_1[0xb];

    local_48[0] = 6;

    local_40 = (uint)*(byte *)(param_1 + 2);

    local_38 = iStack_3c;

    iVar3 = FUN_0304f550(local_2c,local_48);

    if (iVar3 == 0) {

      return 0;

    }

    iVar3 = FUN_0304ed3e(local_2c,0,local_38);

    if (iVar3 == 0) {

      local_38 = 0;

      iVar3 = dict_stream_read(local_2c,4,&local_38);

      if (iVar3 == 4) {

        local_48[0] = 3;

        local_44 = 3;

        local_40 = (uint)*(byte *)(param_1 + 2) << 8 | 1;

        iStack_3c = local_38;

        iVar3 = FUN_0304f550(local_2c,local_48);

        if (iVar3 == 0) {

          return 0;

        }

        iVar3 = FUN_0304ed3e(local_2c,0,local_38);

        if (iVar3 == 0) {

          local_38 = 0;

          iVar3 = dict_stream_read(local_2c,4,&local_38);

          if (iVar3 == 4) {

            local_48[0] = 3;

            local_44 = 4;

            local_40 = (uint)*(byte *)(param_1 + 2) << 8 | 1;

            iStack_3c = local_38;

            iVar4 = FUN_0304f550(local_2c,local_48);

            iVar3 = local_38;

            if (iVar4 == 0) {

              return 0;

            }

            if (*(int **)(param_3 + 0x24) != (int *)0x0) {

              **(int **)(param_3 + 0x24) = local_38;

            }

            if (*(int **)(param_3 + 0x10) != (int *)0x0) {

              **(int **)(param_3 + 0x10) = (uint)(ushort)param_1[10] + local_38;

            }

            iStack_34 = (uint)(ushort)param_1[10] + local_38;

            iVar4 = FUN_0304ed3e(local_2c,0);

            if (iVar4 == 0) {

              local_38 = 0;

              iVar4 = dict_stream_read(local_2c,1,&local_38);

              if (iVar4 == 1) {

                iVar4 = local_38 - (uint)(ushort)param_1[10];

                local_38 = 0;

                iVar5 = dict_stream_read(local_2c,1,&local_38);

                if (iVar5 == 1) {

                  iVar6 = 0;

                  iVar7 = (local_38 - (uint)(ushort)param_1[10]) - iVar4;

                  local_38 = 0;

                  iVar5 = dict_stream_read(local_2c,2,&local_38);

                  if (iVar5 == 2) {

                    if (sVar1 != 0x75 && sVar1 != 0x76) {

                      iVar6 = ((local_38 - (uint)(ushort)param_1[10]) - iVar4) - iVar7;

                      if (*(undefined4 **)(param_3 + 0x20) != (undefined4 *)0x0) {

                        **(undefined4 **)(param_3 + 0x20) = 0;

                      }

                    }

                    if (*(int **)(param_3 + 0x14) != (int *)0x0) {

                      **(int **)(param_3 + 0x14) = iVar4;

                    }

                    if (*(int **)(param_3 + 0x18) != (int *)0x0) {

                      **(int **)(param_3 + 0x18) = iVar7;

                    }

                    if (*(int **)(param_3 + 0x1c) != (int *)0x0) {

                      **(int **)(param_3 + 0x1c) = iVar6;

                    }

                    local_48[0] = 3;

                    local_44 = 4;

                    local_40 = (uint)*(byte *)(param_1 + 2) << 8 | 1;

                    iStack_3c = iVar3;

                    iVar3 = FUN_0304f550(local_2c,local_48);

                    if (((iVar3 != 0) && (iVar3 = FUN_0304ed3e(local_2c,0,iStack_34), iVar3 == 0))

                       && (*(undefined1 **)(param_3 + 0x28) != (undefined1 *)0x0)) {

                      **(undefined1 **)(param_3 + 0x28) = 0;

                      if (*(int *)(param_3 + 0x2c) <= iVar4 + 1) {

                        iVar4 = *(int *)(param_3 + 0x2c);

                      }

                      dict_stream_read(local_2c,iVar4,*(undefined4 *)(param_3 + 0x28));

                      *(undefined1 *)(*(int *)(param_3 + 0x28) + iVar4) = 0;

                    }

                    return uVar8;

                  }

                }

              }

            }

          }

        }

      }

    }

  }

  return 0;

}
