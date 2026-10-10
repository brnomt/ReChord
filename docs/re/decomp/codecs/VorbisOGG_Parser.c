/**
 * VorbisOGG_Parser @ 0x03023be8
 * Tags: codec, ogg, media
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint VorbisOGG_Parser(undefined4 param_1,int *param_2)



{

  byte *pbVar1;

  undefined4 *puVar2;

  undefined2 uVar3;

  uint uVar4;

  int iVar5;

  uint uVar6;

  int iVar7;

  uint uVar8;

  int iVar9;

  uint uVar10;

  bool bVar11;

  bool bVar12;

  byte local_240 [12];

  int local_234;

  undefined1 auStack_230 [240];

  undefined1 local_140 [8];

  byte local_138 [4];

  char local_134 [4];

  undefined1 auStack_130 [8];

  char local_128 [4];

  char local_124 [228];

  undefined4 local_40;

  uint local_3c;

  uint local_38;

  undefined1 auStack_34 [4];

  undefined1 auStack_30 [8];

  uint local_28;

  undefined1 auStack_24 [8];

  

  uVar8 = 0;

  iVar7 = (int)(char)param_1;

  USBMSCHost_Read(0,0);

  cooee_set_key_qqcon(auStack_34,param_1);

  uVar3 = FUN_0300f088(auStack_34,DAT_03023f4c);

  switch(uVar3) {

  case 3:

    *DAT_03023f50 = 0;

    uVar8 = FUN_03023426(param_1,param_2);

    break;

  case 4:

  case 8:

  case 9:

    thunk_FUN_03025648(param_1);

    MediaGetFilePathByCurNum();

    iVar5 = thunk_FUN_03025652();

    uVar8 = thunk_FUN_03025658();

    iVar9 = thunk_FUN_0302565e();

    if ((-1 < iVar5 && -1 < (int)uVar8) && -1 < iVar9) {

      *param_2 = iVar9;

      USBMSCHost_Read(iVar5,0,iVar7);

      return uVar8;

    }

    goto LAB_03023fbc;

  case 5:

    iVar5 = FUN_02ff80c0(iVar7);

    if (0 < iVar5) {

      uVar8 = memset(local_240,0xc,iVar7);

      bVar11 = uVar8 == 0xc;

      if (0xb < uVar8) {

        bVar11 = local_240[0] == 0x52;

      }

      if (bVar11) {

        bVar11 = local_240[1] == 'I';

        if (bVar11) {

          local_240[1] = local_240[2];

        }

        if (bVar11 && local_240[1] == 'F') {

          bVar11 = local_240[3] == 'F';

          if (bVar11) {

            local_240[3] = local_240[8];

          }

          if (bVar11 && local_240[3] == 'W') {

            bVar11 = local_240[9] == 'A';

            if (bVar11) {

              local_240[9] = local_240[10];

            }

            bVar12 = bVar11 && local_240[9] == 'V';

            if (bVar11 && local_240[9] == 'V') {

              bVar12 = local_240[0xb] == 'E';

            }

            if (bVar12) {

              iVar5 = iVar5 + -0xc;

              while ((((0 < iVar5 && (uVar8 = memset(auStack_230,4,iVar7), 3 < uVar8)) &&

                      (uVar8 = memset(&local_234,4,iVar7), 3 < uVar8)) && (-1 < local_234))) {

                iVar9 = FUN_02ff89b4(auStack_230,&DAT_03023f70,4);

                if (iVar9 == 0) {

                  uVar8 = FUN_03023426(iVar7,param_2);

                  return uVar8;

                }

                USBMSCHost_Read(local_234,1,iVar7);

                iVar5 = (iVar5 + -8) - local_234;

              }

            }

          }

        }

      }

    }

    goto LAB_030241f4;

  case 6:

    uVar6 = 0;

    uVar8 = FUN_02ff80c0(iVar7);

    if (uVar8 != 0) {

      USBMSCHost_Read(uVar8 - 0x20,0,iVar7);

      uVar4 = memset(auStack_30,8,iVar7);

      if (((((7 < uVar4) && (iVar5 = FUN_02ff89b4(auStack_30,s_APETAGEX_03023f78,8), iVar5 == 0)) &&

           (uVar4 = memset(auStack_24,4,iVar7), 3 < uVar4)) &&

          (((uVar4 = memset(&local_40,4,iVar7), 3 < uVar4 &&

            (uVar4 = memset(&local_38,4,iVar7), 3 < uVar4)) &&

           ((uVar4 = memset(&local_28,4,iVar7), 3 < uVar4 &&

            ((local_38 < 0x10001 && ((local_28 & 0x20000000) == 0)))))))) && (local_40 <= uVar8)) {

        USBMSCHost_Read(uVar8 - local_40,0,iVar7);

        uVar8 = 0;

        if (local_38 != 0) {

          while (uVar4 = memset(&local_3c,4,iVar7), 3 < uVar4) {

            USBMSCHost_Read(4,1,iVar7);

            uVar6 = uVar6 + 8;

            if (local_40 < uVar6) break;

            iVar5 = 0;

            do {

              iVar9 = memset(local_240 + iVar5,1,iVar7);

              if (iVar9 == 0) goto LAB_03023fbc;

              uVar6 = uVar6 + 1;

              if (local_40 < uVar6) break;

              pbVar1 = local_240 + iVar5;

              if (*pbVar1 != 0) {

                iVar5 = iVar5 + 1;

              }

            } while (*pbVar1 != 0 && iVar5 < 0x200);

            if (iVar5 == 0x200) break;

            iVar5 = FUN_02ff89ec(local_240,s_Cover_Art__front__03023f84);

            if (iVar5 == 0) {

              uVar8 = memset(local_240,0x200,iVar7);

              if (0x1ff < uVar8) {

                iVar5 = 0;

                goto LAB_03023ee0;

              }

              break;

            }

            USBMSCHost_Read(local_3c,1);

            uVar6 = uVar6 + local_3c;

            if ((local_40 < uVar6) || (uVar8 = uVar8 + 1, local_38 <= uVar8)) break;

          }

        }

      }

    }

    goto LAB_03023fbc;

  case 7:

    uVar8 = FUN_02ff80c0(iVar7);

    if ((uVar8 != 0) && (uVar6 = memset(&local_3c,4,iVar7), 3 < uVar6)) {

      iVar5 = FUN_02ff89b4(&local_3c,&DAT_030243bc,3);

      if (iVar5 == 0) {

LAB_03024026:

        uVar8 = memset(local_240,0x200,iVar7);

        if (0x1ff < uVar8) {

          uVar8 = rkos_memory_malloc(local_240,s_image__03023674,6);

          return uVar8;

        }

      }

      else {

        iVar5 = FUN_02ff89b4(&local_3c,&DAT_030243c0,4);

        if ((iVar5 == 0) && (uVar6 = 4, 4 < uVar8)) {

          do {

            uVar4 = memset(&local_40,4,iVar7);

            if (uVar4 < 4) break;

            uVar10 = local_40 >> 7 & 1;

            uVar4 = (local_40._2_2_ & 0xff) << 8 | (uint)(local_40._2_2_ >> 8) |

                    (local_40 >> 8 & 0xff) << 0x10;

            if ((local_40 & 0x7f) == 6) goto LAB_03024026;

            USBMSCHost_Read(uVar4,1,iVar7);

            uVar6 = uVar6 + 4 + uVar4;

            bVar11 = uVar10 != 0;

            if (uVar10 != 1) {

              bVar11 = uVar8 <= uVar6;

            }

          } while (!bVar11);

        }

      }

    }

LAB_03023fbc:

    uVar8 = 0;

    break;

  case 10:

    do {

      func_0x03004d9c();

      uVar8 = memset(local_140,4,iVar7);

      if ((uVar8 < 4) || (iVar5 = FUN_02ff89b4(local_140,&DAT_03023b20,4), iVar5 != 0)) break;

      iVar5 = memset(local_128,1,iVar7);

      if (iVar5 == 0) {

        return 0;

      }

      if (local_128[0] != '\0') break;

      iVar5 = memset(local_134,1,iVar7);

      if (iVar5 == 0) {

        return 0;

      }

      uVar8 = memset(local_140,8,iVar7);

      if (uVar8 < 8) break;

      USBMSCHost_Read(0xc,1,iVar7);

      iVar5 = memset(local_138,1,iVar7);

      if (iVar5 == 0) {

        return 0;

      }

      uVar8 = memset(local_240,local_138[0],iVar7);

      uVar6 = (uint)local_138[0];

      if (uVar8 < uVar6) break;

      if (local_134[0] == '\x01') {

        iVar9 = 0;

        iVar5 = 0;

        if (uVar6 != 0) {

          do {

            pbVar1 = local_240 + iVar5;

            iVar5 = iVar5 + 1;

            iVar9 = iVar9 + (uint)*pbVar1;

          } while (iVar5 < (int)uVar6);

        }

      }

      else {

        iVar5 = memset(local_124,1,iVar7);

        if (iVar5 == 0) {

          return 0;

        }

        uVar8 = memset(auStack_130,6,iVar7);

        if (uVar8 < 6) break;

        if (local_124[0] == '\x03') {

          uVar8 = rkos_memory_malloc(auStack_130,s_vorbis_030243d0,6);

          return uVar8;

        }

        iVar9 = 0;

        USBMSCHost_Read(0xfffffff9,1,iVar7);

        iVar5 = 0;

        if (local_138[0] != 0) {

          do {

            pbVar1 = local_240 + iVar5;

            iVar5 = iVar5 + 1;

            iVar9 = iVar9 + (uint)*pbVar1;

          } while (iVar5 < (int)(uint)local_138[0]);

        }

      }

      USBMSCHost_Read(iVar9,1,iVar7);

    } while (local_134[0] != '\x04');

LAB_030241f4:

    uVar8 = 0;

    break;

  case 0xb:

  case 0xc:

  case 0xd:

    break;

  case 0xe:

    USBMSCHost_Read(0,0,iVar7);

    uVar8 = memset(local_240,0x14,iVar7);

    bVar11 = uVar8 == 0x14;

    if (0x13 < uVar8) {

      bVar11 = local_240[0] == 0x44;

    }

    if (bVar11) {

      bVar11 = local_240[1] == 'S';

      if (bVar11) {

        local_240[1] = local_240[2];

      }

      if ((bVar11 && local_240[1] == 'D') &&

         (uVar8 = memset(DAT_03023f50,4,iVar7), puVar2 = DAT_03023f50, 3 < uVar8)) {

        thunk_FUN_03011c1c(s______ChunkSize__0x_x____03023f54,*DAT_03023f50);

        USBMSCHost_Read(*puVar2,0,iVar7);

        uVar8 = FUN_03023426(param_1,param_2);

        *puVar2 = 0;

        return uVar8;

      }

    }

    return 0xffffffff;

  }

  return uVar8;

LAB_03023ee0:

  if (local_240[iVar5] == 0xff) {

    if (local_240[iVar5 + 1] == 0xd8) {

      iVar5 = 0;

      goto LAB_03023f9e;

    }

  }

  else if ((local_240[iVar5] == 0x42) && (local_240[iVar5 + 1] == 0x4d)) {

    iVar5 = 3;

LAB_03023f9e:

    *param_2 = iVar5;

    goto LAB_03023f08;

  }

  iVar5 = iVar5 + 1;

  if (0x1ff < iVar5) {

LAB_03023f08:

    USBMSCHost_Read(DAT_03023f98,1,iVar7);

    if (local_3c <= local_40) {

      return local_3c;

    }

    return 0;

  }

  goto LAB_03023ee0;

}
