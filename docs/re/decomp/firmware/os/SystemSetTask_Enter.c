/**
 * SystemSetTask_Enter @ 0x03016fa8
 * Structural-match v2 (validated by SDK-callee overlap).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int SystemSetTask_Enter(void)



{

  short sVar1;

  ulonglong uVar2;

  longlong lVar3;

  ushort uVar4;

  uint uVar5;

  int iVar6;

  int iVar7;

  int iVar8;

  int iVar9;

  undefined4 uVar10;

  int iVar11;

  undefined2 *puVar12;

  ushort *puVar13;

  int iVar14;

  undefined2 uStack_48;

  

  iVar11 = DAT_030173bc;

  iVar9 = DAT_030173b4;

  iVar14 = 0;

  puVar12 = &uStack_48;

  puVar13 = (ushort *)(DAT_030173b8 + *(int *)(DAT_030173b4 + 0x10) * 0x14);

  uVar4 = *(ushort *)(DAT_030173b4 + 2);

  uVar5 = uVar4 - 1;

  if (uVar5 < 0x13) {

    puVar13[5] = (ushort)uVar5;

    if (uVar4 == 4) {

      func_0x030037a8(0x11a);

    }

    if (*(short *)(iVar9 + 2) == 1) {

      func_0x030037a8(0x11a);

    }

    *(short *)(iVar9 + 2) = 0xff - *(short *)(iVar9 + 2);

    iVar6 = iVar11;

  }

  else {

    iVar6 = rkos_sleep();

    iVar7 = ui_refresh_and_redraw(0x1dc);

    if (iVar7 != 0) {

      iVar6 = DAT_030173c0;

    }

  }

  iVar8 = ui_refresh_and_redraw(0xc9);

  iVar7 = DAT_030173c4;

  if (iVar8 != 0) {

    return iVar8;

  }

  iVar8 = iVar6 + -0x40000040;

  if (iVar6 == iVar11) goto LAB_030175e8;

  if (iVar6 < iVar11) {

    iVar11 = iVar6 - DAT_030173c8;

    if (iVar6 == DAT_030173c8) {

LAB_030172a6:

      ui_post_redraw(0x11a);

      *DAT_030173d4 = 0;

      iVar11 = *(int *)(puVar13 + 8);

      if (*(ushort *)(iVar11 + 0xe) == 1) {

        return 0;

      }

      if ((int)(uint)puVar13[5] < (int)(*(ushort *)(iVar11 + 0xe) - 1)) {

        uVar4 = puVar13[2];

        if ((uint)uVar4 == *puVar13 - 1) {

          puVar13[2] = 0;

          uVar4 = puVar13[1] + 6;

          puVar13[1] = uVar4;

          if ((uint)*(ushort *)(iVar11 + 0xe) < uVar4 + 6) {

            *puVar13 = *(ushort *)(iVar11 + 0xe) - uVar4;

          }

          else {

            *puVar13 = 6;

          }

          puVar13[3] = 0;

          uVar10 = 0xcb;

        }

        else {

          puVar13[3] = uVar4;

          puVar13[2] = uVar4 + 1;

          uVar10 = 0xce;

        }

        func_0x030037a8(uVar10);

        puVar13[5] = puVar13[5] + 1;

      }

      else {

        puVar13[2] = 0;

        puVar13[5] = 0;

        puVar13[1] = 0;

        if (*(ushort *)(iVar11 + 0xe) < 7) {

          puVar13[3] = *(ushort *)(iVar11 + 0xe) - 1;

          uVar10 = 0xce;

        }

        else {

          *puVar13 = 6;

          puVar13[3] = 0;

          uVar10 = 0xcb;

        }

        func_0x030037a8(uVar10);

      }

      func_0x030037a8(0xcc);

      if (**(short **)(puVar13 + 8) == 0xf6) {

        if ((ushort)*(byte *)(iVar7 + 0x74) != puVar13[5]) {

          func_0x030037a8(0x102);

        }

        *(char *)(iVar7 + 0x74) = (char)puVar13[5];

      }

      else if (**(short **)(puVar13 + 8) == 0x138) {

        *(char *)(iVar7 + 0x17) = (char)puVar13[5];

        func_0x030009f8();

      }

      ui_post_redraw(0x154);

      if (*(int *)(iVar9 + 0x10) != 1) {

        return 0;

      }

      if (*(char *)(iVar7 + 0x357) == 'U') {

        return 0;

      }

      if (*(short *)(iVar9 + 2) == 0xfe) {

        if (puVar13[1] == 6) {

          func_0x030037a8(199);

          ui_post_redraw(0x1dc);

          func_0x030037a8(0x11a);

          *(uint *)(iVar9 + 0x14) = puVar13[2] + 6;

          return 0;

        }

        uVar4 = puVar13[2];

        if (uVar4 != 1) {

          if (uVar4 == 0 || uVar4 == 5) {

            func_0x030037a8(199);

          }

          mbedtls_oid_get_md_alg();

          func_0x030037a8(0x1dc);

          *(undefined1 *)(iVar7 + 0x357) = 0;

          iVar11 = *(int *)(iVar9 + 0x10);

          goto joined_r0x0301726c;

        }

      }

      else if ((uint)puVar13[2] + (uint)puVar13[1] < 6) {

        mbedtls_oid_get_md_alg();

        func_0x030037a8(0x1dc);

        *(uint *)(iVar9 + 0x14) = (uint)puVar13[2];

        iVar14 = FUN_03016c1a(*(undefined2 *)(*(int *)(puVar13 + 8) + (uint)puVar13[5] * 2 + 0x10));

        func_0x030037a8(0xcd);

        uVar10 = 0xcb;

        goto LAB_03017292;

      }

      uVar10 = 199;

      goto LAB_03017292;

    }

    if (iVar6 < DAT_030173c8) {

      if (iVar11 == -0x10000000) goto LAB_030172a6;

      if (iVar11 != -0xffffffc) {

        if (iVar11 == -0xffffff4) goto LAB_030170be;

        if (iVar11 != -0xfffffe4) {

          return 0;

        }

        goto LAB_030170c6;

      }

    }

    else if (iVar11 != 4) {

      if (iVar11 == 0xc) goto LAB_030170be;

      if (iVar11 != 0x1c) {

        return 0;

      }

LAB_030170c6:

      FUN_03016bd8();

      return 0;

    }

    *DAT_030173d4 = 0;

    ui_post_redraw(0x11a);

    iVar11 = *(int *)(puVar13 + 8);

    if (*(short *)(iVar11 + 0xe) == 1) {

      return 0;

    }

    if (puVar13[5] == 0) {

      uVar4 = *(short *)(iVar11 + 0xe) - 1;

      puVar13[5] = uVar4;

      uVar5 = (uint)*(ushort *)(iVar11 + 0xe);

      if (uVar5 < 7) {

        puVar13[3] = 0;

        *puVar13 = *(ushort *)(iVar11 + 0xe);

        uVar10 = 0xce;

      }

      else {

        lVar3 = (ulonglong)DAT_030173d8 * (ulonglong)(uint)uVar4;

        uVar5 = (uint)lVar3;

        *puVar13 = uVar4 + (short)(uint)((ulonglong)lVar3 >> 0x22) * -6 + 1;

        iVar11 = 0xffff;

        puVar13[3] = 0;

        puVar13[1] = (uVar4 / 6 + (short)(uVar4 / 6 << 1)) * 2;

        uVar10 = 0xcb;

      }

      func_0x030037a8(uVar10,iVar11,uVar5);

      puVar13[2] = *puVar13 - 1;

    }

    else {

      uVar4 = puVar13[2];

      if (uVar4 == 0) {

        if (puVar13[1] < 6) {

          puVar13[3] = 0;

          uVar4 = *puVar13 - 1;

        }

        else {

          puVar13[1] = puVar13[1] - 6;

          *puVar13 = 6;

          uVar4 = 5;

        }

        puVar13[2] = uVar4;

        uVar10 = 0xcb;

      }

      else {

        puVar13[3] = uVar4;

        puVar13[2] = uVar4 - 1;

        uVar10 = 0xce;

      }

      func_0x030037a8(uVar10);

      puVar13[5] = puVar13[5] - 1;

    }

    if (**(short **)(puVar13 + 8) == 0xf6) {

      if ((ushort)*(byte *)(iVar7 + 0x74) != puVar13[5]) {

        func_0x030037a8(0x102);

      }

      *(char *)(iVar7 + 0x74) = (char)puVar13[5];

    }

    else if (**(short **)(puVar13 + 8) == 0x138) {

      *(char *)(iVar7 + 0x17) = (char)puVar13[5];

      func_0x030009f8();

    }

    func_0x030037a8(0xcc);

    ui_post_redraw(0x154);

    if (*(int *)(iVar9 + 0x10) != 1) {

      return 0;

    }

    if (*(char *)(iVar7 + 0x357) == 'U') {

      return 0;

    }

    if (*(short *)(iVar9 + 2) == 0xfe) {

      uVar5 = (uint)puVar13[2];

      if (puVar13[1] == 6) {

        *(uint *)(iVar9 + 0x14) = uVar5 + 6;

        func_0x030037a8(199);

        ui_post_redraw(0x1dc);

        uVar10 = 0x11a;

        goto LAB_03017292;

      }

      if (uVar5 == 1) goto LAB_0301729a;

      if (uVar5 == 0 || uVar5 == 5) {

        func_0x030037a8(199);

      }

      mbedtls_oid_get_md_alg();

      func_0x030037a8(0x1dc);

      *(undefined1 *)(iVar7 + 0x357) = 0;

      iVar11 = *(int *)(iVar9 + 0x10);

    }

    else {

      if (5 < (uint)puVar13[2] + (uint)puVar13[1]) {

LAB_0301729a:

        func_0x030037a8(199);

        uVar10 = 0x1dc;

LAB_0301745c:

        ui_post_redraw(uVar10);

        return 0;

      }

      mbedtls_oid_get_md_alg();

      func_0x030037a8(0x1dc);

      iVar11 = *(int *)(iVar9 + 0x10);

    }

joined_r0x0301726c:

    if (iVar11 == 1) {

      *(uint *)(iVar9 + 0x14) = (uint)puVar13[2];

    }

    iVar14 = FUN_03016c1a(*(undefined2 *)(*(int *)(puVar13 + 8) + (uint)puVar13[5] * 2 + 0x10));

    uVar10 = 0xcd;

LAB_03017292:

    func_0x030037a8(uVar10);

    return iVar14;

  }

  if (iVar8 == 0x7fc0) {

    ui_post_redraw(0x11c);

    *DAT_030173d0 = *DAT_030173cc;

    iVar9 = ui_refresh_and_redraw(1);

    if (iVar9 == 0) {

      func_0x030037a8(1);

      return 0;

    }

LAB_030170be:

    FUN_03016b94();

    return 0;

  }

  if (iVar8 < 0x7fc1) {

    if (iVar8 == 0x40) goto LAB_030175e8;

    if (iVar8 != 0xfc0) {

      if (iVar8 != 0x3fc0) {

        return 0;

      }

      ui_post_redraw(0x11c);

      *DAT_030173d0 = *DAT_030173cc;

      iVar9 = ui_refresh_and_redraw(1);

      if (iVar9 != 0) goto LAB_030170c6;

      uVar10 = 1;

      goto LAB_03017292;

    }

    *(undefined2 *)(iVar9 + 4) = 0;

    ui_post_redraw(0x11a);

    ui_post_redraw(0x154);

    *DAT_030176c0 = 0;

    iVar11 = ui_refresh_and_redraw(1);

    if (iVar11 != 0) {

      func_0x030037a8(0x2f);

      uVar10 = 1;

      goto LAB_0301745c;

    }

    sVar1 = **(short **)(puVar13 + 8);

    *(undefined1 *)(iVar7 + 0x357) = 0;

    if (sVar1 == 0xf6) {

      if ((uint)*(byte *)(iVar7 + 0x74) != *(uint *)(iVar9 + 8)) {

        func_0x030037a8(0x102);

      }

      *(undefined1 *)(iVar7 + 0x74) = *(undefined1 *)(iVar9 + 8);

    }

    else if (sVar1 == 0x138) {

      *(undefined1 *)(iVar7 + 0x17) = *(undefined1 *)(iVar9 + 0xc);

      func_0x030009f8();

    }

    iVar11 = *(int *)(iVar9 + 0x10);

    if (iVar11 == 1) {

      sVar1 = *(short *)(iVar9 + 2);

      if (sVar1 != 0xfe) {

        if (sVar1 == 0xf9) goto LAB_03017562;

        if (sVar1 == 0xf5) {

          uStack_48 = CONCAT11(uStack_48._1_1_,6);

          goto LAB_03017562;

        }

        if (sVar1 == 0xf4) {

          iVar9 = FUN_02ff2834(*DAT_030176d4,DAT_030176d0);

          if ((iVar9 == 1) && (*DAT_030176d8 != '\0')) {

            *DAT_030176dc = 0;

            uStack_48 = 2;

            uVar10 = 1;

            puVar12 = &uStack_48;

          }

          else {

            uStack_48 = CONCAT11(uStack_48._1_1_,0xc);

            uVar10 = 0;

            puVar12 = &uStack_48;

          }

          goto LAB_0301767e;

        }

        if (sVar1 == 0xfb) goto LAB_03017686;

        if (sVar1 != 0xf6) goto LAB_030174d2;

        goto LAB_0301768c;

      }

      iVar9 = FUN_02ff2834(*DAT_030176d4,DAT_030176d0);

      if (iVar9 != 1) {

        uStack_48 = CONCAT11(uStack_48._1_1_,1);

        uVar10 = 0;

        puVar12 = &uStack_48;

        goto LAB_0301767e;

      }

      goto LAB_03017558;

    }

    if (iVar11 != 0) {

      if (iVar11 == 2) {

        iVar11 = ui_refresh_and_redraw(0x1dc);

        if (iVar11 == 0) {

          func_0x030037a8(0x1dc);

          func_0x030037a8(0xce);

          ui_post_redraw(0xcd);

          ui_post_redraw(199);

          ui_post_redraw(0x26);

          ui_post_redraw(0xcb);

          x509_get_hash_alg();

          ui_post_redraw(0x1dc);

          func_0x03000518();

        }

      }

      else {

LAB_030174d2:

        func_0x030037a8(199);

        func_0x030037a8(0xcd);

      }

      uVar5 = *(int *)(iVar9 + 0x10) - 1;

      iVar11 = DAT_030176c4 + uVar5 * 0x14;

      uVar4 = puVar13[7];

      uVar2 = (ulonglong)DAT_030176c8;

      *(uint *)(iVar9 + 0x10) = uVar5;

      *(uint *)(iVar11 + 0x10) = DAT_030176cc + (uVar5 + (uint)(uVar2 * uVar5 >> 0x21) * -3) * 0x3a;

      FUN_02ffefb6(uVar4);

      iVar9 = func_0x030037c4(0x1dc);

      if (iVar9 == 0) {

        return 0;

      }

      func_0x030037a8(0xce);

      ui_post_redraw(0xcd);

      ui_post_redraw(199);

      ui_post_redraw(0x26);

      uVar10 = 0xcb;

      goto LAB_0301745c;

    }

  }

  else {

    if (iVar6 == 0x41000000) {

      uStack_48 = 0;

      DisplayDev_SetWindow(DAT_030176e4,DAT_030176e0,&uStack_48);

      return 0;

    }

    if (iVar6 == 0x42000000) {

      uStack_48 = 1;

      DisplayDev_SetWindow(DAT_030176e4,DAT_030176e0,&uStack_48);

      return 0;

    }

    if (iVar8 != 0x10000040) {

      return 0;

    }

LAB_030175e8:

    iVar11 = ui_refresh_and_redraw(1);

    if (iVar11 != 0) {

      func_0x030037a8(0x2f);

      ui_post_redraw(1);

      return 0;

    }

    if (*(int *)(iVar9 + 0x10) != 0) {

      ui_post_redraw(0x11a);

      iVar11 = *(int *)(iVar9 + 0x10);

      if ((iVar11 != 0) && (*(undefined1 *)(iVar7 + 0x357) = 0, iVar11 == 1)) {

        *(uint *)(iVar9 + 0x14) = (uint)puVar13[5];

      }

    }

    iVar11 = FUN_03016c1a(*(undefined2 *)(*(int *)(puVar13 + 8) + (uint)puVar13[5] * 2 + 0x10));

    if (iVar11 != 0) {

      return iVar11;

    }

    iVar11 = ui_refresh_and_redraw(0x7d);

    if (iVar11 != 0) {

      return 0;

    }

    iVar11 = ui_refresh_and_redraw(0x6a);

    if (iVar11 != 0) {

      return 0;

    }

    func_0x030037a8(0xcd);

    if (*(int *)(iVar9 + 0x10) != 0) {

      return 0;

    }

    sVar1 = *(short *)(iVar9 + 2);

    if (sVar1 == 0xfe) {

      iVar9 = FUN_02ff2834(*DAT_030176d4,DAT_030176d0);

      if (iVar9 != 1) {

        uStack_48 = (ushort)uStack_48._1_1_ << 8;

        uVar10 = 0;

        goto LAB_0301767e;

      }

LAB_03017558:

      puVar12 = (undefined2 *)0x0;

      uStack_48 = (ushort)uStack_48._1_1_ << 8;

      uVar10 = 3;

LAB_0301767e:

      FUN_030039d2(uVar10,puVar12);

      return 1;

    }

    if (sVar1 != 0xfb) {

      if (sVar1 != 0xf6) {

        return 0;

      }

LAB_0301768c:

      uStack_48 = uStack_48 & 0xff00;

      goto LAB_03017562;

    }

  }

LAB_03017686:

  uStack_48 = CONCAT11(uStack_48._1_1_,2);

LAB_03017562:

  FUN_030039d2(0,&uStack_48);

  return 1;

}
