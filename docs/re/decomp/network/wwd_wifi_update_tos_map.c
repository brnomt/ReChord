/**
 * wwd_wifi_update_tos_map @ 0x0300ba94
 * Structural-match v2 (validated by same-module callees).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 wwd_wifi_update_tos_map(void)



{

  undefined4 *puVar1;

  undefined4 *puVar2;

  int *piVar3;

  uint *puVar4;

  undefined4 *puVar5;

  undefined1 *puVar6;

  uint *puVar7;

  int iVar8;

  int iVar9;

  undefined4 uVar10;

  uint uVar11;

  uint uVar12;

  int unaff_r5;

  char *unaff_r6;

  undefined4 unaff_r7;

  bool bVar13;

  char cStack00000000;

  char cStack00000001;

  char cStack00000002;

  undefined4 in_stack_00000204;

  undefined4 in_stack_00000208;

  uint in_stack_00000210;

  undefined4 in_stack_00000214;

  int in_stack_00000218;

  undefined4 in_stack_00000286;

  undefined4 in_stack_0000028a;

  undefined4 in_stack_0000028e;

  undefined2 in_stack_000002ba;

  

  FUN_02feb5d6();

  iVar8 = FUN_0300cb04();

  puVar6 = DAT_0300bc58;

  puVar1 = DAT_0300bc2c;

  if (iVar8 == 1) {

    *DAT_0300bc2c = 3;

    *puVar6 = 0;

  }

  else {

    if (cStack00000000 == -1) {

      if (cStack00000001 == -2) {

        *DAT_0300bc2c = unaff_r7;

        goto LAB_0300bae8;

      }

    }

    else {

      bVar13 = cStack00000000 == -0x11;

      if (bVar13) {

        cStack00000000 = cStack00000001;

      }

      if ((bVar13 && cStack00000000 == -0x45) && (cStack00000002 == -0x41)) {

        *DAT_0300bc2c = 3;

        *puVar6 = 3;

        goto LAB_0300bae8;

      }

    }

    *DAT_0300bc2c = 2;

  }

LAB_0300bae8:

  *(undefined2 *)(DAT_0300bc4c + 0x21d) = in_stack_000002ba;

  RKBT_FRead(puVar1 + 4,unaff_r5 + 0x20,0x66);

  *(undefined4 *)((int)puVar1 + 0x76) = in_stack_00000286;

  *(undefined4 *)((int)puVar1 + 0x7a) = in_stack_0000028a;

  *(undefined4 *)((int)puVar1 + 0x7e) = in_stack_0000028e;

  RKBT_FRead((int)puVar1 + 0x82,unaff_r5 + 0x92,0x28);

  FUN_02febace(0,0,(int)*unaff_r6);

  puVar2 = DAT_0300bc34;

  puVar1[1] = in_stack_00000204;

  puVar5 = DAT_0300bc48;

  puVar1 = DAT_0300bc44;

  *puVar2 = in_stack_00000208;

  *puVar5 = in_stack_00000208;

  *puVar1 = in_stack_00000208;

  iVar8 = FUN_02ffa6f4(200);

  puVar7 = DAT_0300bc5c;

  if (iVar8 == 0) {

    uVar12 = 0;

    do {

      wwd_bus_init();

      uVar12 = uVar12 + 1;

    } while (uVar12 < 0xc);

  }

  else {

    if (*DAT_0300bc5c == 0) {

      LOGE(200);

      uVar12 = 0;

      do {

        wwd_bus_init();

        uVar12 = uVar12 + 1;

      } while (uVar12 < 0xc);

    }

    if (in_stack_00000210 < *puVar7) {

      LOGE(200);

      uVar12 = 0;

      do {

        wwd_bus_init();

        uVar12 = uVar12 + 1;

      } while (uVar12 < 0xc);

    }

  }

  piVar3 = DAT_0300bc38;

  *puVar2 = in_stack_00000208;

  puVar4 = DAT_0300bc3c;

  *piVar3 = in_stack_00000218;

  puVar1 = DAT_0300bc40;

  *puVar4 = in_stack_00000210;

  *puVar1 = in_stack_00000214;

  iVar8 = DAT_0300bc30;

  uVar12 = 0;

  do {

    *(undefined4 *)(iVar8 + uVar12 * 4) = *(undefined4 *)(unaff_r5 + uVar12 * 4 + 0xbc);

    uVar12 = uVar12 + 1;

  } while (uVar12 < 0x32);

  iVar9 = FUN_02ffa6f4(200);

  if (iVar9 != 0) {

    uVar12 = *puVar7;

    if ((uVar12 == 0) || (*puVar4 < uVar12)) {

      LOGE(200);

    }

    else {

      uVar11 = (uVar12 - 1) / 400;

      *puVar7 = uVar12 - 1;

      uVar10 = *(undefined4 *)(iVar8 + uVar11 * 4);

      *puVar5 = uVar10;

      *puVar2 = uVar10;

      *piVar3 = uVar11 * 400;

      uVar12 = 0;

      do {

        wwd_bus_init();

        uVar12 = uVar12 + 1;

      } while (uVar12 < 0xc);

    }

  }

  return 1;

}
