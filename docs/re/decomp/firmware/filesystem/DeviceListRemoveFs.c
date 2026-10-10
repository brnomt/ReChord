/**
 * DeviceListRemoveFs @ 0x03022354
 * Structural-match v2 (validated by SDK-callee overlap).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined1 * DeviceListRemoveFs(undefined4 param_1,uint param_2,int param_3)



{

  char cVar1;

  char *pcVar2;

  uint uVar3;

  int iVar4;

  uint uVar5;

  uint uVar6;

  int iVar7;

  int iVar8;

  uint uVar9;

  ushort uVar10;

  uint unaff_r9;

  undefined1 in_stack_0000000c;

  ushort uStack0000000e;

  uint in_stack_00000144;

  ushort uStack00000150;

  ushort uStack00000152;

  uint in_stack_00000154;

  uint in_stack_00000158;

  uint in_stack_0000015c;

  ushort in_stack_00000244;

  

  uVar3 = FUN_02ffce6c();

  pcVar2 = DAT_030225e4;

  if (((uVar3 <= param_2) ||

      (iVar4 = USBMSCHost_Read(*(int *)(DAT_030225e4 + 4) + *(int *)(DAT_030225e4 + 0x10) + param_2

                               + -2,0,param_1), iVar4 != 0)) ||

     (uVar5 = memset(&stack0x00000244,2,param_1), uVar5 < 2)) {

    return (undefined1 *)0x0;

  }

  if (*pcVar2 != '\x01') {

    in_stack_00000244 = in_stack_00000244 << 8 | in_stack_00000244 >> 8;

  }

  uVar5 = RKDev_Open(param_1);

  if (param_3 == 1) {

    iVar4 = USBMSCHost_Read((uint)in_stack_00000244 * 0xc +

                            *(int *)(pcVar2 + 4) + *(int *)(pcVar2 + 0x10) + 8,0,param_1);

    if (iVar4 != 0) {

      return (undefined1 *)0x0;

    }

    uVar6 = memset(&stack0x00000250,4,param_1);

    if (uVar6 < 4) {

      return (undefined1 *)0x0;

    }

    if (*pcVar2 != '\x01') {

      unaff_r9 = unaff_r9 << 0x18 | (unaff_r9 >> 8 & 0xff) << 0x10 | (unaff_r9 >> 0x10 & 0xff) << 8

                 | unaff_r9 >> 0x18;

    }

    USBMSCHost_Read(uVar5,0,param_1);

  }

  FUN_02ff895c(&stack0x0000000c,0x144);

  in_stack_0000000c = (undefined1)param_3;

  iVar4 = 0;

  uStack0000000e = in_stack_00000244;

  in_stack_00000144 = unaff_r9;

  if (in_stack_00000244 != 0) {

    do {

      if (uVar3 <= uVar5) {

        return (undefined1 *)0x0;

      }

      iVar7 = USBMSCHost_Read(uVar5,0,param_1);

      if (iVar7 != 0) {

        return (undefined1 *)0x0;

      }

      uVar5 = memset(&stack0x00000150,0xc,param_1);

      uVar6 = in_stack_00000158;

      if (uVar5 < 0xc) {

        return (undefined1 *)0x0;

      }

      if (*pcVar2 != '\x01') {

        uStack00000150 = uStack00000150 << 8 | uStack00000150 >> 8;

        uStack00000152 = uStack00000152 << 8 | uStack00000152 >> 8;

        in_stack_00000154 =

             in_stack_00000154 << 0x18 | (in_stack_00000154 >> 8 & 0xff) << 0x10 |

             (in_stack_00000154 >> 0x10 & 0xff) << 8 | in_stack_00000154 >> 0x18;

        in_stack_00000158 =

             in_stack_00000158 << 0x18 | (in_stack_00000158 >> 8 & 0xff) << 0x10 |

             (in_stack_00000158 >> 0x10 & 0xff) << 8 | in_stack_00000158 >> 0x18;

      }

      if (uStack00000150 == 0) {

        return (undefined1 *)0x0;

      }

      if (0x1f < in_stack_00000154) {

        in_stack_00000154 = 0x1f;

      }

      uVar5 = RKDev_Open(param_1);

      if (uStack00000152 == 2 || uStack00000152 == 7) {

        if (in_stack_00000154 < 5) {

          FUN_03022206(&stack0x0000000c,uStack00000150,uStack00000152,in_stack_00000154);

        }

        else {

          if (0x40 < in_stack_00000154) {

            FUN_02ff895c(&stack0x00000220,in_stack_00000154);

          }

          iVar7 = USBMSCHost_Read(in_stack_00000158 + *(int *)(pcVar2 + 4) + *(int *)(pcVar2 + 0x10)

                                  + -2,0,param_1);

          if ((iVar7 == 0) &&

             (uVar6 = memset(&stack0x00000220,in_stack_00000154,param_1), in_stack_00000154 <= uVar6

             )) {

            FUN_03022206(&stack0x0000000c,uStack00000150,uStack00000152,in_stack_00000154);

          }

          else {

LAB_030226bc:

            FUN_03022206(&stack0x0000000c,uStack00000150,uStack00000152,in_stack_00000154);

          }

        }

      }

      else if (uStack00000152 == 5 || uStack00000152 == 10) {

        iVar7 = in_stack_00000154 * 2;

        uVar6 = in_stack_00000154 * 8;

        if (((uVar6 < *(ushort *)(DAT_030225ec + 2)) &&

            (iVar8 = USBMSCHost_Read(in_stack_00000158 +

                                     *(int *)(pcVar2 + 4) + *(int *)(pcVar2 + 0x10) + -2,0,param_1),

            iVar8 == 0)) &&

           ((uVar9 = memset(&stack0x00000160,uVar6,param_1), uVar6 <= uVar9 &&

            (iVar8 = 0, 0 < iVar7)))) {

          do {

            uVar6 = *(uint *)(&stack0x00000160 + iVar8 * 4);

            *(uint *)(&stack0x00000160 + iVar8 * 4) =

                 uVar6 << 0x18 | (uVar6 >> 8 & 0xff) << 0x10 | (uVar6 >> 0x10 & 0xff) << 8 |

                 uVar6 >> 0x18;

            iVar8 = iVar8 + 1;

          } while (iVar7 - iVar8 != 0 && iVar8 <= iVar7);

        }

LAB_030225a2:

        FUN_03022206(&stack0x0000000c,uStack00000150,uStack00000152,in_stack_00000154);

      }

      else if (((uStack00000152 == 1 || uStack00000152 == 3) ||

               (uStack00000152 == 4 || uStack00000152 == 6)) ||

              (uStack00000152 == 8 || uStack00000152 == 9)) {

        if (1 < in_stack_00000154) {

          iVar7 = 4;

          if (uStack00000152 == 1 || uStack00000152 == 6) {

            iVar7 = 1;

          }

          else if (uStack00000152 == 3 || uStack00000152 == 8) {

            iVar7 = 2;

          }

          uVar6 = in_stack_00000154 * iVar7;

          if (uVar6 < 5) {

            if (iVar7 == 1) {

              iVar7 = 0;

              if (0 < (int)in_stack_00000154) {

                do {

                  *(uint *)(&stack0x00000160 + iVar7 * 4) = (uint)(byte)(&stack0x00000248)[iVar7];

                  iVar7 = iVar7 + 1;

                } while (iVar7 < (int)in_stack_00000154);

              }

            }

            else if (iVar7 == 2) {

              cVar1 = *pcVar2;

              iVar7 = 0;

              do {

                uVar10 = *(ushort *)(&stack0x00000248 + iVar7 * 2);

                if (cVar1 != '\x01') {

                  uVar10 = uVar10 << 8 | uVar10 >> 8;

                }

                *(uint *)(&stack0x00000160 + iVar7 * 4) = (uint)uVar10;

                iVar7 = iVar7 + 1;

              } while (iVar7 < 2);

            }

          }

          else {

            iVar8 = USBMSCHost_Read(in_stack_00000158 +

                                    *(int *)(pcVar2 + 4) + *(int *)(pcVar2 + 0x10) + -2,0,param_1);

            if ((iVar8 != 0) || (uVar9 = memset(&stack0x000001e0,uVar6,param_1), uVar9 < uVar6))

            goto LAB_030226bc;

            iVar8 = 0;

            if (0 < (int)in_stack_00000154) {

              do {

                if (0x1f < iVar8 * iVar7) break;

                FUN_02ff890e(&stack0x0000015c,&stack0x000001e0 + iVar8 * iVar7,iVar7);

                if (iVar7 == 4) {

                  in_stack_0000015c =

                       in_stack_0000015c << 0x18 | (in_stack_0000015c >> 8 & 0xff) << 0x10 |

                       (in_stack_0000015c >> 0x10 & 0xff) << 8 | in_stack_0000015c >> 0x18;

                }

                else if ((iVar7 == 2) && (*pcVar2 != '\x01')) {

                  in_stack_0000015c =

                       (in_stack_0000015c & 0xff) << 8 | (in_stack_0000015c & 0xffff) >> 8;

                }

                *(uint *)(&stack0x00000160 + iVar8 * 4) = in_stack_0000015c;

                iVar8 = iVar8 + 1;

              } while (iVar8 < (int)in_stack_00000154);

            }

          }

          goto LAB_030225a2;

        }

        in_stack_0000015c = in_stack_00000158;

        if (uStack00000152 == 1 || uStack00000152 == 6) {

          in_stack_0000015c = uVar6 & 0xff;

        }

        else if (uStack00000152 == 3 || uStack00000152 == 8) {

          if (*pcVar2 != '\x01') {

            uVar6 = (uint)(ushort)((ushort)uVar6 << 8 | (ushort)uVar6 >> 8);

          }

          in_stack_0000015c = uVar6 & 0xffff;

        }

        FUN_03022206(&stack0x0000000c,uStack00000150,uStack00000152,in_stack_00000154);

      }

      iVar4 = iVar4 + 1;

    } while (iVar4 < (int)(uint)in_stack_00000244);

  }

  return &stack0x0000000c;

}
