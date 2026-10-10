/**
 * music_ui_draw @ 0x0301231c
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Restarted to delay deadcode elimination for space: stack */



void music_ui_draw(short param_1,int param_2,short param_3,short param_4,undefined4 param_5,

                  int param_6,uint param_7)



{

  char *pcVar1;

  undefined1 uVar2;

  int iVar3;

  undefined1 *puVar4;

  int iVar5;

  short sVar6;

  int unaff_r8;

  ushort *unaff_r10;

  int unaff_r11;

  undefined4 in_stack_00000050;

  undefined4 in_stack_0000006c;

  

  param_3 = param_3 - param_4;

  sVar6 = 0;

  if (5 < param_3) {

    param_3 = 6;

  }

  param_4 = param_4 - param_1;

  FUN_02fede0e(&stack0x0000001c,unaff_r10 + 6,0x2a);

  param_5 = *(undefined4 *)(unaff_r10 + 4);

  param_6 = 0;

  param_7 = (uint)*unaff_r10;

  iVar3 = FUN_02ff3884(&stack0x00000048);

  pcVar1 = DAT_03012614;

  iVar5 = param_2;

  if (iVar3 != 0) {

    return;

  }

  do {

    if ((in_stack_00000050._3_1_ & 2) == 0) {

      if ((in_stack_00000050._3_1_ & 0x10) == 0) {

        if (param_3 != 0) {

          if (param_4 == 0) {

            FUN_02fede0e(param_2 + 0x4f,&stack0x00000048,0x20);

            *(undefined4 *)(param_2 + 8) = param_5;

            FUN_02fede0e(param_2 + 0x24,&stack0x0000001c,0x2a);

            *(int *)(param_2 + 0xc) = param_6 + -1;

            if (*pcVar1 == '\x03') {

              FUN_02ff30c4(param_2 + 0x24,param_6 + -1);

            }

            else {

              FUN_02ff3f2c(param_5);

            }

            if (*pcVar1 == '\x03') {

              FUN_02ff5638(&stack0x00000048,param_2 + 0x70);

            }

            puVar4 = (undefined1 *)FUN_03012194(&stack0x00000048);

            if (unaff_r11 == 1) {

              uVar2 = 1;

            }

            else {

              uVar2 = *puVar4;

            }

            *(undefined1 *)(param_2 + 0x4e) = uVar2;

            param_3 = param_3 + -1;

            param_2 = *(int *)(param_2 + 4);

          }

          else {

            param_4 = param_4 + -1;

          }

        }

      }

      else if (sVar6 != 0) {

        if (unaff_r8 == 0) {

          FUN_02fede0e(iVar5 + 0x4f,&stack0x00000048,0x20);

          *(undefined1 *)(iVar5 + 0x4e) = 0;

          *(undefined4 *)(iVar5 + 8) = param_5;

          FUN_02fede0e(iVar5 + 0x24,&stack0x0000001c,0x2a);

          *(int *)(iVar5 + 0xc) = param_6 + -1;

          if (*pcVar1 == '\x03') {

            FUN_02ff30c4(iVar5 + 0x24,param_6 + -1);

          }

          else {

            FUN_02ff3f2c(param_5);

          }

          sVar6 = sVar6 + -1;

          iVar5 = *(int *)(iVar5 + 4);

        }

        else {

          unaff_r8 = (int)(short)((short)unaff_r8 + -1);

        }

      }

      if (sVar6 == 0 && param_3 == 0) {

        return;

      }

    }

    iVar3 = FUN_02ff3858(&stack0x00000048,&param_5,in_stack_0000006c);

    if (iVar3 != 0) {

      return;

    }

  } while( true );

}
