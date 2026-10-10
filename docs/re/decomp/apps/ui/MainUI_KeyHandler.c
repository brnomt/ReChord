/**
 * MainUI_KeyHandler @ 0x0301020c
 * Tags: ui, menu
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

/* Main UI physical-key handler (LADC keypad, not capacitive touch).

   Maps key events + on-screen soft-keyboard / list focus via rom_gui_check_area(x1,y1,x2,y2) LCD

   rectangles.

   Stages at +0x1bc: 0=filename edit (6x5 letter cells), 1=browse, 2=list.

   EQ apply loads WOOOOONXBIN/GOODEFGHMP3. Cursor char '|'=0x7c.

   Formerly misnamed DICTDECODER_OpenFile / MainUI_KeyTouch_Handler. */



bool MainUI_KeyHandler(void)



{

  char cVar1;

  byte bVar2;

  byte bVar3;

  byte *pbVar4;

  int iVar5;

  byte bVar6;

  int iVar7;

  int iVar8;

  undefined4 uVar9;

  int iVar10;

  int iVar11;

  uint uVar12;

  uint uVar13;

  int iVar14;

  bool bVar15;

  bool bVar16;

  undefined2 local_68;

  undefined2 local_66;

  undefined2 local_64;

  undefined1 local_48;

  undefined2 local_47;

  undefined2 local_45;

  

  iVar10 = rom_get_input_event();

  iVar5 = _DAT_0300fe2c;

  iVar8 = DAT_0300fe28;

  iVar11 = DAT_0300fe10;

  pbVar4 = DAT_0300fe0c;

  bVar15 = false;

  uVar12 = 0;

  iVar7 = 0;

  if (iVar10 != 0x40000080) {

    iVar7 = iVar10 + -0x50000000;

  }

  iVar14 = iVar10;

  if ((iVar10 == 0x40000080 || iVar7 == 0x80) || iVar10 == _DAT_0300fe2c) {

    if (*(char *)(DAT_0300fe10 + 0x1bc) == '\x02') {

      iVar7 = rom_gui_check_area(0,0,0x3c,0x32,0);

      iVar14 = h5_close;

      if (iVar7 == 0) {

        do {

          iVar7 = rom_gui_check_area(0,uVar12 * 0x18 + 0x1b,0x140,uVar12 * 0x18 + 0x33,0);

          if (iVar7 != 0) {

            bVar6 = *(byte *)(iVar11 + 0x1ba);

            iVar14 = 0;

            if (bVar6 != 0) {

              bVar2 = *pbVar4;

              iVar14 = iVar8;

              if (bVar2 != uVar12) {

                uVar12 = uVar12 + (bVar2 / 6) * 6;

                bVar3 = (byte)uVar12;

                *pbVar4 = bVar3;

                if ((int)(bVar6 - 1) < (int)(uVar12 & 0xff)) {

                  *pbVar4 = bVar2;

                  iVar14 = 0;

                }

                else {

                  pbVar4[4] = bVar3;

                  rom_post_event(0x33);

                  rom_post_event(0x1c3);

                }

              }

            }

            break;

          }

          uVar12 = uVar12 + 1 & 0xff;

          iVar14 = iVar10;

        } while (uVar12 < 6);

      }

    }

    else {

      iVar7 = rom_gui_check_area(0,0,0x3c,0x32,0);

      iVar14 = h5_close;

      if (iVar7 == 0) {

        iVar7 = rom_gui_check_area(0x46,0,0xa0,0x46,0);

        if (iVar7 == 0) {

          iVar7 = rom_gui_check_area(0xbc,0,0xf0,0x3c,0);

          if (iVar7 == 0) {

            iVar14 = iVar10;

            if ((iVar10 != iVar5) && (*(char *)(iVar11 + 0x1bc) == '\0')) {

              do {

                uVar13 = 0;

                do {

                  iVar7 = rom_gui_check_area(uVar12 * 0x26 + 8,uVar13 * 0x26 + 0x7e,

                                             uVar12 * 0x26 + 0x2e,uVar13 * 0x26 + 0xa4,0);

                  if (iVar7 != 0) {

                    pbVar4[2] = (byte)uVar12;

                    pbVar4[3] = (byte)uVar13;

                    iVar10 = DAT_0300fe28;

                    uVar12 = 9;

                    rom_post_event(0x33);

                    rom_post_event(0x1c3);

                    break;

                  }

                  uVar13 = uVar13 + 1 & 0xff;

                } while (uVar13 < 5);

                uVar12 = uVar12 + 1 & 0xff;

                iVar14 = iVar10;

              } while (uVar12 < 6);

            }

          }

          else {

            iVar14 = iVar10;

            if (*(byte *)(iVar11 + 0x1bc) < 2) {

              iVar14 = 0x40000002;

            }

          }

        }

        else {

          iVar14 = iVar10;

          if (iVar10 != iVar5) {

            iVar14 = 0x20000040;

          }

        }

      }

    }

  }

  iVar7 = rom_event_pending(0x158);

  if (iVar7 != 0) {

    rom_event_clear_id(0x1bb);

    iVar7 = iVar14 + -0x40000040;

    bVar15 = iVar7 != 0;

    bVar16 = iVar7 != 0x1c0;

    if (bVar15 && bVar16) {

      iVar7 = iVar14 + -0x50000040;

    }

    if ((!bVar15 || !bVar16) || iVar7 == 0x1c0) {

      local_68 = *(undefined2 *)(DAT_0300fe10 + 0x21d);

      rom_ui_cmd(8,&local_68);

    }

    bVar15 = (!bVar15 || !bVar16) || iVar7 == 0x1c0;

    if (iVar14 != 0x20000040) {

      return bVar15;

    }

  }

  iVar10 = DAT_03010254;

  iVar11 = DAT_0300fe08;

  iVar7 = DAT_0300fe04;

  iVar8 = iVar14 + -0x40000100;

  if (iVar14 == 0x40000100) {

LAB_030100d0:

    rom_event_clear_id(0x159);

    if (*(int *)(iVar7 + 4) != -1) {

      FUN_03020886();

    }

    local_68 = 6;

    rom_ui_cmd(10,&local_68);

    return true;

  }

  if (iVar14 < 0x40000101) {

    if (iVar14 == 0x20001000) {

      if (*(int *)(DAT_0300fe04 + 4) != -1) {

        FUN_03020886();

      }

      *(undefined1 *)(DAT_03010254 + 0x1bb) = 0;

      rom_ui_cmd(0,&local_68);

      return true;

    }

    if (iVar14 < 0x20001001) {

      if (iVar14 == 0x20000002) {

        if (*(char *)(DAT_03010254 + 0x1bc) != '\0') {

          return bVar15;

        }

        bVar6 = *pbVar4;

joined_r0x03010066:

        if (bVar6 == 0) {

          return bVar15;

        }

        local_48 = 1;

        local_47 = 0x38;

        local_45 = 0x75;

        FUN_02fed544(DAT_03010278,DAT_03010274,&local_48);

        return bVar15;

      }

      if (iVar14 == 0x20000040) {

        if (pbVar4[1] == 0) {

          return bVar15;

        }

        if (*pbVar4 == 0) {

          pbVar4[1] = 0;

          rom_post_event(0x32);

          return bVar15;

        }

        iVar10 = DSP_GOODEF_Init(s_WOOOOONXBIN_0300fe1c,1);

        if (iVar10 == 0) {

          return bVar15;

        }

        if (*(char *)(DAT_03010254 + 0x1bc) == '\0') {

          *(undefined1 *)(*(int *)(iVar11 + 8) + *pbVar4 + 1) = 0;

          *(undefined1 *)(*(int *)(iVar11 + 8) + (uint)*pbVar4) = 0;

        }

        iVar8 = ed25519_test(iVar10,*(undefined4 *)(iVar11 + 8),s_GOODEFGHMP3_03010258);

        if (*(char *)(DAT_03010254 + 0x1bc) == '\0') {

          *(undefined1 *)(*(int *)(iVar11 + 8) + (uint)*pbVar4) = 0x7c;

        }

        FUN_0300f8a2(iVar10);

        FUN_02fed690(DAT_03010264);

        if (iVar8 == 0) {

          FUN_02ffec94(0x3b,7);

          uVar9 = FUN_03020810(*(undefined4 *)(iVar11 + 4));

          *(undefined4 *)(iVar7 + 4) = uVar9;

          return bVar15;

        }

        if (*(char *)(DAT_03010254 + 0x1bc) == '\0') {

          bVar6 = *pbVar4;

          *DAT_03010268 = bVar6;

          *(undefined1 *)(*(int *)(iVar11 + 8) + (uint)bVar6) = 0;

        }

        iVar7 = DAT_0301026c;

        iVar11 = *(int *)(iVar11 + 8);

        uVar12 = 0;

        do {

          *(ushort *)(iVar7 + uVar12 * 2) = (ushort)*(byte *)(iVar11 + uVar12);

          uVar12 = uVar12 + 1 & 0xff;

        } while (uVar12 < 2);

        rom_post_event(0x159);

        local_68 = 1;

        rom_ui_cmd(3,&local_68);

        return true;

      }

      if (iVar14 != 0x20000400) {

        if (iVar14 != 0x20000800) {

          return bVar15;

        }

        goto LAB_0301017c;

      }

    }

    else {

      iVar8 = iVar14 + -0x20001000 + DAT_0301024c;

      if (iVar8 != 0) {

        if (iVar8 == 0x400) goto LAB_0301017c;

        if (iVar8 + DAT_03010250 == 0) goto LAB_030100d0;

        if (iVar8 + DAT_03010250 != 0x3e) {

          return bVar15;

        }

        cVar1 = *(char *)(DAT_03010254 + 0x1bc);

        if (cVar1 != '\0') {

          if (cVar1 == '\x01') {

            if (*(char *)(DAT_03010254 + 0x1ba) != '\0') {

              local_48 = 1;

              local_47 = 0x38;

              local_45 = 0x76;

              FUN_02fed544(DAT_03010278,DAT_03010274,&local_48);

              return bVar15;

            }

LAB_030100b8:

            local_68 = 0x38;

            local_66 = 0x77;

            local_64 = 1;

            FUN_02fed544(DAT_03010278,DAT_0301027c,&local_68);

            return bVar15;

          }

          if (cVar1 != '\x02') {

            return bVar15;

          }

          if (*(char *)(DAT_03010254 + 0x1ba) == '\0') goto LAB_030100b8;

          *(undefined1 *)(DAT_03010254 + 0x1bc) = 1;

          uVar9 = 0x32;

          goto LAB_030100b2;

        }

        uVar12 = (uint)pbVar4[2];

        if (uVar12 < 4) {

          if ((pbVar4[3] == 4) && (uVar12 == 2 || uVar12 == 3)) {

            bVar6 = *pbVar4;

            goto joined_r0x03010066;

          }

LAB_0301002c:

          uVar12 = uVar12 + (uint)pbVar4[3] * 6;

          if (0x1a < uVar12) {

            return bVar15;

          }

          *(char *)(*(int *)(DAT_0300fe08 + 8) + (uint)*pbVar4) = (char)uVar12 + 'a';

          if (*pbVar4 < 0xf) {

            bVar6 = *pbVar4 + 1;

LAB_0300ffe8:

            *pbVar4 = bVar6;

          }

          else {

            *pbVar4 = 0;

          }

        }

        else {

          if (pbVar4[3] != 4) goto LAB_0301002c;

          if (*pbVar4 != 0) {

            bVar6 = *pbVar4 - 1;

            goto LAB_0300ffe8;

          }

        }

        *(undefined1 *)(*(int *)(iVar11 + 8) + *pbVar4 + 1) = 0;

        *(undefined1 *)(*(int *)(iVar11 + 8) + (uint)*pbVar4) = 0;

        if (*(char *)(iVar10 + 0x356) != '\0') {

          MediaGotoNextDir(DAT_03010270);

        }

        *(undefined1 *)(*(int *)(iVar11 + 8) + (uint)*pbVar4) = 0x7c;

        DSP_GOODEF_Process();

        uVar9 = 0x33;

        goto LAB_030100b2;

      }

    }

LAB_030101c4:

    if (*(char *)(DAT_03010254 + 0x1bc) != '\0') {

      if (*(char *)(DAT_03010254 + 0x1bc) == '\x01') {

LAB_03010200:

        pbVar4[8] = 1;

        pbVar4[9] = 0;

        return bVar15;

      }

      bVar6 = *(byte *)(DAT_03010254 + 0x1ba);

      if (bVar6 < 2) {

        return bVar15;

      }

      bVar2 = *pbVar4;

      pbVar4[4] = bVar2;

      if (bVar2 == 0) {

        bVar2 = bVar6;

      }

      *pbVar4 = bVar2 - 1;

LAB_030101a8:

      uVar9 = 0x33;

      goto LAB_030100b2;

    }

    if (pbVar4[3] == 0) {

      bVar6 = 3;

    }

    else {

      bVar6 = pbVar4[3] - 1;

    }

    pbVar4[3] = bVar6;

  }

  else {

    if (iVar8 == 0x3f00) goto LAB_030101c4;

    if (iVar8 < 0x3f01) {

      if (iVar8 == 0x100) goto LAB_030100f4;

      if (iVar8 == 0x300) goto LAB_030101c4;

      if (iVar8 != 0x700) {

        if (iVar8 != 0xf00) {

          return bVar15;

        }

        goto LAB_030100f4;

      }

    }

    else if (iVar8 != 0x7f00) {

      if (iVar8 == 0x10000000) goto LAB_030100d0;

      if (iVar8 != 0x10000100) {

        return bVar15;

      }

LAB_030100f4:

      if (*(char *)(DAT_03010254 + 0x1bc) == '\x01') {

        pbVar4[4] = *pbVar4;

        *(undefined1 *)(iVar10 + 0x1bc) = 2;

      }

      else if (*(char *)(DAT_03010254 + 0x1bc) == '\x02') {

        *pbVar4 = 0;

        pbVar4[2] = 0;

        pbVar4[3] = 0;

        *(undefined1 *)(*(int *)(iVar11 + 8) + 1) = 0;

        **(undefined1 **)(iVar11 + 8) = 0x7c;

        *(undefined1 *)(iVar10 + 0x1bc) = 0;

      }

      else {

        if (*(int *)(DAT_0300fe04 + 4) != -1) {

          FUN_03020886();

        }

        *(undefined1 *)(iVar10 + 0x1bb) = 0;

        rom_event_clear_id(0x159);

        local_68 = 6;

        rom_ui_cmd(10,&local_68);

        bVar15 = true;

      }

      rom_post_event(0x32);

      return bVar15;

    }

LAB_0301017c:

    if (*(char *)(DAT_03010254 + 0x1bc) != '\0') {

      if (*(char *)(DAT_03010254 + 0x1bc) == '\x01') goto LAB_03010200;

      bVar6 = *(byte *)(DAT_03010254 + 0x1ba);

      if (bVar6 < 2) {

        return bVar15;

      }

      bVar2 = *pbVar4;

      pbVar4[4] = bVar2;

      if ((int)(uint)bVar2 < (int)(bVar6 - 1)) {

        *pbVar4 = bVar2 + 1;

      }

      else {

        *pbVar4 = 0;

      }

      goto LAB_030101a8;

    }

    if (pbVar4[3] < 3) {

      pbVar4[3] = pbVar4[3] + 1;

    }

    else {

      pbVar4[3] = 0;

    }

  }

  uVar9 = 0x1c3;

LAB_030100b2:

  rom_post_event(uVar9);

                    /* WARNING: Read-only address (ram,0x0300fe30) is written */

  return bVar15;

}
