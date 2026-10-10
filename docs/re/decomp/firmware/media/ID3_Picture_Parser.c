/**
 * ID3_Picture_Parser @ 0x03023526
 * Tags: media, codec
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 ID3_Picture_Parser(void)



{

  uint uVar1;

  byte bVar2;

  undefined1 *puVar3;

  int *piVar4;

  char *pcVar5;

  uint uVar6;

  int iVar7;

  uint uVar8;

  uint uVar9;

  undefined4 uVar10;

  uint uVar11;

  undefined1 *puVar12;

  uint uVar13;

  byte *pbVar14;

  int iVar15;

  uint uVar16;

  uint unaff_r4;

  int iVar17;

  int unaff_r5;

  uint unaff_r6;

  uint uVar18;

  uint uVar19;

  uint unaff_r8;

  undefined4 unaff_r10;

  bool bVar20;

  uint in_stack_00000738;

  int in_stack_00000740;

  uint uVar21;

  byte in_stack_00000748;

  char in_stack_0000074c;

  

  USBMSCHost_Read(unaff_r5 + 4,1);

  uVar19 = (unaff_r6 - unaff_r5) - 8;

  if (uVar19 != 0) {

    do {

      uVar6 = memset(&stack0x00000740,4);

      if (uVar6 >= 4) {

        unaff_r6 = unaff_r4 + 4;

      }

      if (uVar6 < 4 || uVar19 < unaff_r6) {

        return 0;

      }

      if (in_stack_00000740 < 1) {

        return 0;

      }

      iVar17 = 0;

      do {

        iVar7 = memset(&stack0x00000700 + iVar17,1);

        if (iVar7 == 0) {

          return 0;

        }

        unaff_r6 = unaff_r6 + 1;

        if (uVar19 < unaff_r6) break;

        uVar6 = (uint)(byte)(&stack0x00000700)[iVar17];

        bVar20 = SBORROW4(uVar6,0x3d);

        iVar7 = uVar6 - 0x3d;

        if (uVar6 != 0x3d) {

          bVar20 = SBORROW4(iVar17 + 1,0x32);

          iVar7 = iVar17 + -0x31;

          iVar17 = iVar17 + 1;

        }

      } while (iVar7 < 0 != bVar20);

      if (iVar17 == 0x32) {

        return 0;

      }

      uVar6 = (in_stack_00000740 - iVar17) - 1;

      iVar17 = FUN_02ff8a16(&stack0x00000700,s_METADATA_BLOCK_PICTURE_03023ab8,iVar17);

      piVar4 = DAT_03023ad0;

      if (iVar17 == 0) {

        uVar19 = uVar19 - unaff_r6;

        iVar17 = 0;

        uVar18 = 0;

        if (uVar6 != 0 && uVar19 != 0) {

          if (*DAT_03023ad0 != 0) {

            FUN_02ffc8bc((int)*DAT_03023ad4);

            *DAT_03023ad4 = -1;

            *piVar4 = 0;

          }

          FUN_03000296(&DAT_03023ae4,s_PICTURE_OGG_03023ad8);

          iVar7 = FUN_0300024e(&DAT_03023ae4,s_PICTURE_OGG_03023ad8);

          *DAT_03023ad4 = (char)iVar7;

          if (iVar7 != -1) {

            *piVar4 = 1;

            uVar9 = 0;

            uVar21 = unaff_r8;

            goto LAB_03023736;

          }

        }

        return 0;

      }

      USBMSCHost_Read(uVar6,1);

      unaff_r4 = unaff_r6 + uVar6;

    } while (unaff_r4 <= uVar19);

  }

  return 0;

LAB_03023736:

  func_0x03004d9c();

  while (uVar19 != 0 && uVar6 != 0) {

    uVar8 = uVar19;

    if (0x1ff < uVar19) {

      uVar8 = 0x200;

    }

    if (uVar9 == 0) {

      if (uVar6 < uVar8) {

        uVar8 = uVar6;

      }

      uVar11 = memset(&stack0x00000200,uVar8,unaff_r10);

      uVar13 = uVar8;

      if (uVar11 < uVar8) {

        return 0;

      }

    }

    else {

      uVar13 = uVar8 - uVar9;

      bVar20 = uVar13 < 0x200 - uVar9;

      if (bVar20) {

        uVar13 = 0x200 - uVar9;

      }

      if (bVar20 && uVar8 < uVar13) {

        uVar13 = uVar8;

      }

      if (uVar6 < uVar13) {

        uVar13 = uVar6;

      }

      uVar8 = memset(&stack0x00000200 + uVar9,uVar13,unaff_r10);

      if (uVar8 < uVar13) {

        return 0;

      }

      uVar8 = uVar9 + uVar13;

      uVar9 = 0;

    }

    uVar6 = uVar6 - uVar13;

    uVar19 = uVar19 - uVar13;

    if (((uVar8 != 0x200) && (uVar21 == 0xffffffff)) && (uVar6 != 0)) goto LAB_030239ac;

    uVar13 = 0;

    uVar11 = 0;

    pbVar14 = &stack0x00000200;

    iVar7 = 0;

    if (uVar8 == 0) {

LAB_030238a4:

      unaff_r8 = 0;

LAB_0302392a:

      iVar7 = 0;

    }

    else {

      do {

        iVar15 = 0;

        for (; (uVar13 < uVar8 && (pbVar14[uVar13] == 0x20)); uVar13 = uVar13 + 1) {

          iVar15 = iVar15 + 1;

        }

        if (uVar13 == uVar8) break;

        bVar20 = uVar8 - uVar13 == 2;

        if (1 < uVar8 - uVar13) {

          bVar20 = pbVar14[uVar13] == 0xd;

        }

        if (bVar20) {

          bVar2 = (&stack0x00000201)[uVar13];

        }

        else {

          bVar2 = pbVar14[uVar13];

        }

        if (bVar2 != 10) {

          if (iVar15 == 0) {

            uVar16 = (uint)pbVar14[uVar13];

            if (uVar16 == 0x3d) {

              uVar11 = uVar11 + 1;

              if (2 < uVar11) {

                iVar7 = -2;

                goto LAB_0302392c;

              }

LAB_03023848:

              if (*(byte *)(DAT_03023ae8 + uVar16) != 0x7f) {

                if ((0x3f < *(byte *)(DAT_03023ae8 + uVar16)) || (uVar11 == 0)) {

                  iVar7 = iVar7 + 1;

                  goto LAB_03023876;

                }

                iVar7 = -4;

                goto LAB_0302392c;

              }

            }

            else if (uVar16 < 0x80) goto LAB_03023848;

            iVar7 = -3;

            goto LAB_0302392c;

          }

          iVar7 = -1;

          goto LAB_0302392c;

        }

LAB_03023876:

        uVar13 = uVar13 + 1;

      } while (uVar13 < uVar8);

      if (iVar7 == 0) goto LAB_030238a4;

      unaff_r8 = (iVar7 * 6 + 7U >> 3) - uVar11;

      if ((&stack0x00000400 != (undefined1 *)0x0) && (unaff_r8 < 0x201)) {

        uVar8 = 0;

        uVar11 = 3;

        iVar7 = 0;

        puVar3 = &stack0x00000400;

        for (; uVar13 != 0; uVar13 = uVar13 - 1) {

          uVar16 = (uint)*pbVar14;

          puVar12 = puVar3;

          if ((uVar16 != 0xd && uVar16 != 10) && uVar16 != 0x20) {

            uVar11 = uVar11 - (*(byte *)(DAT_03023ae8 + uVar16) == 0x40);

            uVar1 = uVar8 << 6;

            uVar8 = *(byte *)(DAT_03023ae8 + uVar16) & 0x3f | uVar1;

            iVar7 = iVar7 + 1;

            if ((iVar7 == 4) && (iVar7 = 0, uVar11 != 0)) {

              *puVar3 = (char)(uVar1 >> 0x10);

              puVar12 = puVar3 + 1;

              if (1 < uVar11) {

                puVar3[1] = (char)(uVar1 >> 8);

                puVar12 = puVar3 + 2;

                if (2 < uVar11) {

                  puVar12 = puVar3 + 3;

                  puVar3[2] = (char)uVar8;

                }

              }

            }

          }

          pbVar14 = pbVar14 + 1;

          puVar3 = puVar12;

        }

        unaff_r8 = (int)puVar3 - (int)&stack0x00000400;

        goto LAB_0302392a;

      }

      iVar7 = -5;

    }

LAB_0302392c:

    if (iVar7 != 0) {

      thunk_FUN_03011c1c(s_base64_decode_error__ret___d_03023aec);

    }

    uVar8 = unaff_r8;

    if (0x200 - uVar18 < unaff_r8) {

      uVar8 = 0x200 - uVar18;

    }

    FUN_02ff890e(&stack0x00000000 + uVar18,&stack0x00000400,uVar8);

    uVar18 = uVar18 + uVar8;

    if (uVar18 == 0x200) {

      uVar18 = func_0x03000448(&stack0x00000000,iVar17,0x200,(int)*DAT_03023ad4);

      if (uVar18 < 0x200) goto LAB_03023970;

      uVar18 = unaff_r8 - uVar8;

      iVar17 = iVar17 + 0x200;

      FUN_02ff890e(&stack0x00000000,&stack0x00000400 + uVar8);

    }

  }

  if ((uVar21 != 0xffffffff) || (uVar8 = uVar9, uVar6 == 0)) {

    if ((uVar18 == 0) ||

       (uVar19 = func_0x03000448(&stack0x00000000,iVar17,uVar18,(int)*DAT_03023ad4),

       uVar18 <= uVar19)) {

      pcVar5 = DAT_03023ad4;

      USBMSCHost_Read(0,0,(int)*DAT_03023ad4);

      uVar19 = memset(&stack0x00000000,0x200,(int)*pcVar5);

      if (0x1ff < uVar19) {

        uVar10 = rkos_memory_malloc(&stack0x00000000,s_image__03023674,6);

        return uVar10;

      }

      thunk_FUN_03011c1c(s_MetaBlockPicHandle_Read_Error_03023f24);

    }

    else {

LAB_03023970:

      thunk_FUN_03011c1c(s_FileWrite_error_03023b0c);

    }

    return 0;

  }

LAB_030239ac:

  uVar9 = memset(&stack0x00000738,4,unaff_r10);

  if (uVar9 < 4) {

    return 0;

  }

  iVar7 = FUN_02ff89b4(&stack0x00000738,&DAT_03023b20,4);

  if (iVar7 != 0) {

    return 0;

  }

  iVar7 = memset(&stack0x0000074c,1,unaff_r10);

  if (iVar7 == 0) {

    return 0;

  }

  if (in_stack_0000074c != '\0') {

    return 0;

  }

  iVar7 = memset(&stack0x00000754,1,unaff_r10);

  if (iVar7 == 0) {

    return 0;

  }

  uVar9 = memset(&stack0x00000738,8,unaff_r10);

  if (uVar9 < 8) {

    return 0;

  }

  USBMSCHost_Read(0xc,1,unaff_r10);

  iVar7 = memset(&stack0x00000748,1,unaff_r10);

  if (iVar7 == 0) {

    return 0;

  }

  uVar9 = memset(&stack0x00000600,in_stack_00000748,unaff_r10);

  uVar13 = (uint)in_stack_00000748;

  if (uVar9 < uVar13) {

    return 0;

  }

  iVar7 = 0;

  uVar9 = uVar8;

  uVar21 = in_stack_00000738;

  if (in_stack_00000738 != 0xffffffff) {

    if (uVar13 != 0) {

      do {

        if ((&stack0x00000600)[iVar7] != 0xff) {

          uVar19 = uVar19 + (byte)(&stack0x00000600)[iVar7];

          break;

        }

        uVar19 = uVar19 + 0xff;

        iVar7 = iVar7 + 1;

      } while (iVar7 < (int)uVar13);

    }

    goto LAB_03023736;

  }

  if (uVar13 != 0) {

    do {

      pbVar14 = &stack0x00000600 + iVar7;

      iVar7 = iVar7 + 1;

      uVar19 = uVar19 + *pbVar14;

    } while (iVar7 < (int)uVar13);

  }

  goto LAB_03023736;

}
