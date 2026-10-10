/**
 * record_shell @ 0x0304CCE8
 * Structural-matcher sweep (callee+module validated).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void record_shell(undefined4 param_1,undefined4 param_2,undefined4 param_3)



{

  int iVar1;

  int iVar2;

  

  iVar1 = DAT_0304cddc;

  iVar2 = 9;

  do {

    if (*(char *)(iVar1 + iVar2) == '\0') {

      record_shell_help(param_1,param_2,iVar2,param_3);

    }

    iVar2 = iVar2 + -1;

  } while (-1 < iVar2);

  return;

}
