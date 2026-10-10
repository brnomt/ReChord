/**
 * audio_file_input_read @ 0x0300cfd4
 * Named via cross-version lineage (v3.4.0/v3.0.0 clusters); decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint audio_file_input_read(uint param_1,char *param_2,uint param_3)



{

  char *pcVar1;

  char cVar2;

  char *pcVar3;

  int *piVar4;

  uint uVar5;

  char *pcVar6;

  uint uVar7;

  uint uVar8;

  uint uVar9;

  bool bVar10;

  

  piVar4 = DAT_0300d34c;

  pcVar3 = DAT_0300d344;

  uVar9 = param_1;

  do {

    if (uVar9 < 0x201) {

      return param_3;

    }

    uVar9 = uVar9 - 0x200;

    FUN_02febace(uVar9,0,(int)*pcVar3);

    uVar5 = FUN_02feb5d6(param_2,0x200,(int)*pcVar3);

    uVar5 = uVar5 & 0xffff;

    (param_2 + uVar5 * 2)[0] = '\0';

    (param_2 + uVar5 * 2)[1] = '\0';

    if (uVar5 == 0) {

      return param_3;

    }

    uVar7 = 0;

    uVar5 = uVar5 - 2;

    uVar8 = uVar9;

    if (0 < (int)uVar5) {

      pcVar6 = param_2;

      do {

        if (param_1 - uVar8 < 0x201) goto LAB_0300d044;

        cVar2 = *pcVar6;

        if (*piVar4 != 1) {

          if (cVar2 == '\r') {

            if (pcVar6[1] == '\n') goto LAB_0300d082;

          }

          else if (cVar2 != '\n') goto LAB_0300d044;

          uVar8 = uVar8 + 1;

          break;

        }

        if (cVar2 == '\r') {

          if (pcVar6[1] == '\0') {

            pcVar1 = pcVar6 + 2;

            if (*pcVar1 == '\n') {

              pcVar6 = (char *)(uint)(byte)pcVar6[3];

            }

            if (*pcVar1 != '\n' || pcVar6 != (char *)0x0) goto LAB_0300d082;

            uVar8 = uVar8 + 4;

            break;

          }

        }

        else {

          bVar10 = cVar2 == '\n';

          if (bVar10) {

            cVar2 = pcVar6[1];

          }

          if (bVar10 && cVar2 == '\0') {

LAB_0300d082:

            uVar8 = uVar8 + 2;

            break;

          }

        }

LAB_0300d044:

        pcVar6 = pcVar6 + 1;

        uVar8 = uVar8 + 1;

        uVar7 = uVar7 + 1 & 0xffff;

      } while ((int)uVar7 < (int)uVar5);

    }

    if (uVar5 != uVar7) {

      return uVar8;

    }

  } while( true );

}
