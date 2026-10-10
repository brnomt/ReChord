/**
 * sdio_init_func @ 0x030cc462
 * Structural-matcher sweep (post-boundary-repair, module-validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 sdio_init_func(int param_1,int param_2,int param_3)



{

  undefined4 *puVar1;

  int iVar2;

  uint uVar3;

  uint *puVar4;

  int iVar5;

  uint uVar6;

  uint uVar7;

  uint uVar8;

  undefined4 uVar9;

  bool bVar10;

  undefined4 local_68;

  int local_64;

  undefined4 local_60;

  int local_5c;

  uint local_58;

  uint local_54;

  undefined4 local_50;

  undefined4 local_4c;

  undefined4 local_48;

  int local_44;

  undefined4 local_40;

  undefined4 local_3c;

  int iStack_30;

  int iStack_2c;

  int local_28;

  

  puVar1 = DAT_030cc898;

  local_40 = 0;

  local_3c = 0;

  local_48 = 0;

  local_44 = 0;

  local_68 = 0;

  local_64 = 0;

  local_60 = 0;

  local_5c = 0;

  local_58 = 0;

  local_54 = 0;

  local_50 = 0;

  uVar9 = 0;

  local_4c = 0;

  iStack_30 = param_1;

  iStack_2c = param_2;

  local_28 = param_3;

  do {

    if (*(int *)(param_1 + 0x68) == 5) {

      while( true ) {

        iVar2 = FUN_030c775e(*(undefined4 *)(param_1 + 0x88),&local_68);

        uVar7 = local_54;

        uVar8 = local_58;

        *puVar1 = 0;

        if (iVar2 < 0) {

          uVar9 = 0xfffffffd;

          goto LAB_030cc6a2;

        }

        if (iVar2 < 1) break;

        iVar2 = FUN_030c4238(*(undefined4 *)(param_1 + 0x8c),&local_68,1);

        if (iVar2 == 0) {

          uVar3 = printi(*(undefined4 *)(param_1 + 0x8c),0);

          uVar6 = *(uint *)(param_1 + 0x80);

          *(uint *)(param_1 + 0x80) = uVar3 + uVar6;

          *(uint *)(param_1 + 0x84) =

               *(int *)(param_1 + 0x84) + ((int)uVar3 >> 0x1f) + (uint)CARRY4(uVar3,uVar6);

          uVar6 = *(uint *)(param_1 + 0x78);

          uVar3 = local_64 * 8;

          *(uint *)(param_1 + 0x78) = uVar6 + uVar3;

          *(uint *)(param_1 + 0x7c) =

               *(int *)(param_1 + 0x7c) + ((int)uVar3 >> 0x1f) + (uint)CARRY4(uVar6,uVar3);

          if ((uVar8 != 0xffffffff || uVar7 != 0xffffffff) && (local_5c == 0)) {

            bVar10 = *(int *)(param_1 + 4) != 0;

            if (bVar10) {

              iVar2 = *(int *)(param_1 + 0x70);

            }

            else {

              iVar2 = 0;

            }

            if ((bVar10 && iVar2 != 0) && -1 < iVar2) {

              puVar4 = (uint *)(*(int *)(param_1 + 0x2c) + iVar2 * 0x10);

              uVar3 = *puVar4;

              bVar10 = uVar8 < uVar3;

              uVar8 = uVar8 - uVar3;

              uVar7 = (uVar7 - puVar4[1]) - (uint)bVar10;

            }

            if ((int)uVar7 < 0) {

              uVar7 = 0;

              uVar8 = uVar7;

            }

            uVar3 = printi(*(undefined4 *)(param_1 + 0x8c),0);

            uVar6 = uVar8 - uVar3;

            iVar5 = (uVar7 - ((int)uVar3 >> 0x1f)) - (uint)(uVar8 < uVar3);

            if (0 < iVar2) {

              puVar4 = (uint *)(*(int *)(param_1 + 0x2c) + 8);

              do {

                bVar10 = CARRY4(*puVar4,uVar6);

                uVar6 = *puVar4 + uVar6;

                iVar5 = puVar4[1] + iVar5 + (uint)bVar10;

                puVar4 = puVar4 + 4;

                iVar2 = iVar2 + -1;

              } while (iVar2 != 0);

            }

            *(uint *)(param_1 + 0x60) = uVar6;

            *(int *)(param_1 + 100) = iVar5;

          }

          uVar9 = 1;

          goto LAB_030cc6a2;

        }

      }

    }

    iVar2 = *(int *)(param_1 + 0x68);

    if (iVar2 < 2) {

LAB_030cc5fe:

      if (iVar2 < 3) {

        if (*(int *)(param_1 + 4) == 0) {

          iVar2 = fifoDevShellCreate(param_1,param_1 + 0x30,param_1 + 0x50,param_1 + 0x6c,&local_48)

          ;

          if (iVar2 != 0) goto LAB_030cc6a2;

          *(int *)(param_1 + 0x70) = *(int *)(param_1 + 0x70) + 1;

        }

        else {

          iVar2 = FUN_030c6bc2(&local_48);

          *(int *)(param_1 + 0x6c) = iVar2;

          uVar7 = *(uint *)(param_1 + 0x1c);

          uVar8 = 0;

          if ((int)uVar7 < 1) {

            uVar3 = 0;

          }

          else {

            uVar3 = uVar7 & 1;

            if (uVar3 != 0) {

              do {

                if (*(int *)(*(int *)(param_1 + 0x28) + uVar8 * 4) == iVar2) goto LAB_030cc664;

                uVar8 = uVar8 + 1;

              } while ((int)uVar8 < (int)uVar3);

            }

          }

          uVar8 = uVar3;

          if ((int)uVar8 < (int)uVar7) {

            do {

              if (*(int *)(*(int *)(param_1 + 0x28) + uVar8 * 4) == iVar2) break;

              if (*(int *)(*(int *)(param_1 + 0x28) + uVar8 * 4 + 4) == iVar2) {

                uVar8 = uVar8 + 1;

                break;

              }

              uVar8 = uVar8 + 2;

            } while ((int)uVar8 < (int)uVar7);

          }

LAB_030cc664:

          if (uVar7 == uVar8) goto LAB_030cc6a2;

          *(uint *)(param_1 + 0x70) = uVar8;

          iVar2 = fifoDevShellCreate(param_1,param_1 + 0x30,param_1 + 0x50,param_1 + 0x6c,&local_48)

          ;

          if (iVar2 != 0) goto LAB_030cc6a2;

        }

      }

      iVar2 = sdio_memcpy_toio(param_1);

      if (iVar2 != 0) {

        return 0xffffff77;

      }

    }

    else {

      if ((param_2 == 0) || (iVar2 = FwCheck(param_1,&local_48,0xffffffff,0xffffffff), iVar2 < 0))

      goto LAB_030cc6a2;

      uVar7 = *(uint *)(param_1 + 0x78);

      uVar8 = local_44 * 8;

      *(uint *)(param_1 + 0x78) = uVar7 + uVar8;

      *(uint *)(param_1 + 0x7c) =

           *(int *)(param_1 + 0x7c) + ((int)uVar8 >> 0x1f) + (uint)CARRY4(uVar7,uVar8);

      iVar2 = *(int *)(param_1 + 0x68);

      if (iVar2 != 5) goto LAB_030cc5fe;

      iVar2 = FUN_030c6bc2(&local_48);

      if (iVar2 != *(int *)(param_1 + 0x6c)) {

        if (local_28 != 0) {

          iVar2 = *(int *)(param_1 + 0x68);

          if (iVar2 == 5) {

            UdpRecive(*(undefined4 *)(param_1 + 0x8c));

            *(undefined4 *)(param_1 + 0x8c) = 0;

            *(undefined4 *)(param_1 + 0x68) = 3;

          }

          else if (iVar2 < 3) goto LAB_030cc5fe;

          FUN_030c7b12(param_1 + 0x30);

          GUI_MsgBoxDelete(param_1 + 0x50);

          iVar2 = 2;

          *(undefined4 *)(param_1 + 0x68) = 2;

          goto LAB_030cc5fe;

        }

LAB_030cc6a2:

        sdio_read_fbr(&local_68);

        mbedtls_ecp_keypair_init(&local_48);

        return uVar9;

      }

      iVar2 = *(int *)(param_1 + 0x68);

      if (iVar2 != 5) goto LAB_030cc5fe;

    }

    sdio_read_cis(*(undefined4 *)(param_1 + 0x88),&local_48);

  } while( true );

}
