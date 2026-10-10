/**
 * theme_color_helper @ 0x03005b54
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void theme_color_helper(uint param_1,uint param_2,uint param_3,uint param_4)



{

  char cVar1;

  int iVar2;

  undefined4 uVar3;

  uint uVar4;

  

  iVar2 = DAT_03005e8c;

  do {

    FUN_02fef416(param_1 & 0xffff,param_2 & 0xffff,param_3 & 0xffff,param_4 & 0xffff);

    for (uVar4 = param_1; (int)uVar4 <= (int)param_3; uVar4 = uVar4 + 1) {

      cVar1 = *(char *)(iVar2 + 0x34f);

      if (cVar1 == '\0') {

        FUN_02fef364(0x632c);

      }

      else {

        if (cVar1 == '\x01') {

          uVar3 = 0x10a2;

        }

        else if (cVar1 == '\x02') {

          uVar3 = 0xfd55;

        }

        else if (cVar1 == '\x03') {

          uVar3 = 0x2b6f;

        }

        else {

          if (cVar1 != '\x04') goto LAB_03005bc2;

          uVar3 = 0xb2a0;

        }

        FUN_02fef364(uVar3);

      }

LAB_03005bc2:

    }

    param_2 = param_2 + 1;

    if ((int)param_4 < (int)param_2) {

      return;

    }

  } while( true );

}
