/**
 * wwd_wifi_test_credentials @ 0x0300bada
 * Structural-match v2 (validated by same-module callees).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 wwd_wifi_test_credentials(int param_1,undefined4 param_2,undefined1 *param_3)



{

  undefined4 *puVar1;

  int *piVar2;

  uint *puVar3;

  undefined4 *puVar4;

  undefined4 *puVar5;

  uint *puVar6;

  int iVar7;

  int iVar8;

  undefined4 uVar9;

  uint uVar10;

  undefined4 *unaff_r4;

  uint uVar11;

  int unaff_r5;

  char *unaff_r6;

  undefined4 in_stack_00000204;

  undefined4 in_stack_00000208;

  uint in_stack_00000210;

  undefined4 in_stack_00000214;

  int in_stack_00000218;

  undefined4 in_stack_00000286;

  undefined4 in_stack_0000028a;

  undefined4 in_stack_0000028e;

  undefined2 in_stack_000002ba;

  

  if (param_1 == 0xbf) {

    *unaff_r4 = param_2;

    *param_3 = (char)param_2;

  }

  else {

    *unaff_r4 = 2;

  }

  *(undefined2 *)(DAT_0300bc4c + 0x21d) = in_stack_000002ba;

  RKBT_FRead(unaff_r4 + 4,unaff_r5 + 0x20,0x66);

  *(undefined4 *)((int)unaff_r4 + 0x76) = in_stack_00000286;

  *(undefined4 *)((int)unaff_r4 + 0x7a) = in_stack_0000028a;

  *(undefined4 *)((int)unaff_r4 + 0x7e) = in_stack_0000028e;

  RKBT_FRead((int)unaff_r4 + 0x82,unaff_r5 + 0x92,0x28);

  FUN_02febace(0,0,(int)*unaff_r6);

  puVar1 = DAT_0300bc34;

  unaff_r4[1] = in_stack_00000204;

  puVar5 = DAT_0300bc48;

  puVar4 = DAT_0300bc44;

  *puVar1 = in_stack_00000208;

  *puVar5 = in_stack_00000208;

  *puVar4 = in_stack_00000208;

  iVar7 = FUN_02ffa6f4(200);

  puVar6 = DAT_0300bc5c;

  if (iVar7 == 0) {

    uVar11 = 0;

    do {

      wwd_bus_init();

      uVar11 = uVar11 + 1;

    } while (uVar11 < 0xc);

  }

  else {

    if (*DAT_0300bc5c == 0) {

      LOGE(200);

      uVar11 = 0;

      do {

        wwd_bus_init();

        uVar11 = uVar11 + 1;

      } while (uVar11 < 0xc);

    }

    if (in_stack_00000210 < *puVar6) {

      LOGE(200);

      uVar11 = 0;

      do {

        wwd_bus_init();

        uVar11 = uVar11 + 1;

      } while (uVar11 < 0xc);

    }

  }

  piVar2 = DAT_0300bc38;

  *puVar1 = in_stack_00000208;

  puVar3 = DAT_0300bc3c;

  *piVar2 = in_stack_00000218;

  puVar4 = DAT_0300bc40;

  *puVar3 = in_stack_00000210;

  *puVar4 = in_stack_00000214;

  iVar7 = DAT_0300bc30;

  uVar11 = 0;

  do {

    *(undefined4 *)(iVar7 + uVar11 * 4) = *(undefined4 *)(unaff_r5 + uVar11 * 4 + 0xbc);

    uVar11 = uVar11 + 1;

  } while (uVar11 < 0x32);

  iVar8 = FUN_02ffa6f4(200);

  if (iVar8 != 0) {

    uVar11 = *puVar6;

    if ((uVar11 == 0) || (*puVar3 < uVar11)) {

      LOGE(200);

    }

    else {

      uVar10 = (uVar11 - 1) / 400;

      *puVar6 = uVar11 - 1;

      uVar9 = *(undefined4 *)(iVar7 + uVar10 * 4);

      *puVar5 = uVar9;

      *puVar1 = uVar9;

      *piVar2 = uVar10 * 400;

      uVar11 = 0;

      do {

        wwd_bus_init();

        uVar11 = uVar11 + 1;

      } while (uVar11 < 0xc);

    }

  }

  return 1;

}
