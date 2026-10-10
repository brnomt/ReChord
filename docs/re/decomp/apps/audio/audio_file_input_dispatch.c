/**
 * audio_file_input_dispatch @ 0x0300d142
 * Named via call-graph propagation from named cluster neighbors.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void audio_file_input_dispatch(short param_1)



{

  undefined4 *puVar1;

  int iVar2;

  ushort uVar3;

  undefined4 uVar4;

  ushort uVar5;

  

  puVar1 = DAT_0300d340;

  uVar4 = audio_file_input_helper(*DAT_0300d340,(int)*DAT_0300d344);

  puVar1[8] = uVar4;

  *puVar1 = uVar4;

  puVar1[1] = uVar4;

  iVar2 = DAT_0300d34c;

  uVar5 = 0x18U - param_1 & 0xff;

  uVar3 = 0;

  if (uVar5 != 0) {

    do {

      **(undefined2 **)(iVar2 + 0xc) = 0;

      FUN_0300c65a();

      uVar3 = uVar3 + 1;

    } while (uVar3 < uVar5);

    return;

  }

  return;

}
