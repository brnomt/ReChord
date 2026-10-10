/**
 * FileDev_GetVolumeInfByNum @ 0x0304fe5c
 * Structural-matcher sweep (post-boundary-repair, module-validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


short * FileDev_GetVolumeInfByNum(short *param_1,int param_2,int *param_3)



{

  short *psVar1;

  char *pcVar2;

  

  if (((param_1 != (short *)0x0) && (0 < *(int *)(param_1 + 4))) &&

     (param_2 != -1 && param_3 != (int *)0x0)) {

    if (*param_3 != 0) {

      return (short *)param_3[1];

    }

    pcVar2 = (char *)param_3[1];

    if (pcVar2 != (char *)0x0) {

      if (*param_1 == 0x75 || *param_1 == 0x76) {

        return param_1;

      }

      if (((*pcVar2 == 'g' && pcVar2[1] == 'r') && (pcVar2[2] == 'e' && pcVar2[3] == 'e')) &&

         (pcVar2[4] == 'n' && pcVar2[5] == '\0')) {

        return (short *)0x4470;

      }

      psVar1 = (short *)FileDev_FileSeek2(param_1);

      return psVar1;

    }

  }

  return (short *)&DAT_7fffffff;

}
