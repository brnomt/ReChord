/**
 * dict_stream_read @ 0x0304eec4
 * Tags: codec, parser
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int dict_stream_read(char *param_1,int param_2,int param_3)



{

  char cVar1;

  int iVar2;

  int iVar3;

  int iVar4;

  int iStack_3c;

  undefined1 auStack_38 [16];

  int iStack_28;

  

  if ((param_3 != 0) && (0 < param_2)) {

    if (param_1 == (char *)0x0) {

      FUN_03033618(s_sjlsjdlfjlsdf_2222_______________0304f024);

      return -2;

    }

    cVar1 = *param_1;

    switch(cVar1) {

    case '\0':

      return -3;

    case '\x01':

    case '\x02':

    case '\x03':

    case '\x04':

      goto switchD_0304eeec_caseD_1;

    default:

      return -4;

    }

  }

  return 0;

switchD_0304eeec_caseD_1:

  iVar3 = *(int *)(param_1 + 0x18);

  if (iVar3 < 0) {

    FUN_03033618(s_sjlsjdlfjlsdf_11111______________0304f050);

    return -5;

  }

  if (*(int *)(param_1 + 0x10) == 0) {

    iVar4 = *(int *)(param_1 + 0x14);

  }

  else {

    iVar4 = *(int *)(param_1 + 0xc) + *(int *)(param_1 + 0x10);

  }

  if (iVar4 <= iVar3) {

    FUN_03033618(DAT_0304f07c,iVar3,iVar4,*(undefined4 *)(param_1 + 0xc));

    return -1;

  }

  if (cVar1 == '\x01' || cVar1 == '\x02') {

    iVar3 = FUN_030198e4(*(undefined4 *)(param_1 + 0x1c),0,1);

    *(int *)(param_1 + 0x18) = iVar3;

    if (iVar4 < iVar3 + param_2) {

      param_2 = iVar4 - iVar3;

    }

    param_2 = FUN_030198b6(*(undefined4 *)(param_1 + 0x1c),param_3,param_2);

    if (param_2 < 0) {

      FUN_03033618(s_sjlsjdlfjlsdf___________________0304f080);

      return -5;

    }

    iVar4 = FUN_030198e4(*(undefined4 *)(param_1 + 0x1c),0,1);

    *(int *)(param_1 + 0x18) = iVar4;

    if (iVar4 < 0) {

      FUN_03033618(s_sjlsjdlfjlsdf__________________x_0304f0a8);

      return -5;

    }

  }

  else {

    if (cVar1 != '\x03' && cVar1 != '\x04') {

      return -2;

    }

    if (iVar4 < iVar3 + param_2) {

      param_2 = iVar4 - iVar3;

    }

    FUN_0301a30e(param_3,iVar3 + *(int *)(param_1 + 0x1c),param_2);

    *(int *)(param_1 + 0x18) = *(int *)(param_1 + 0x18) + param_2;

    if (param_2 < 0) {

      return param_2;

    }

  }

  iVar4 = param_2;

  if (*(int *)(param_1 + 0xc) < 0) {

    iVar3 = iVar3 - *(int *)(param_1 + 0xc);

  }

  while( true ) {

    if (iVar4 < 1) {

      return param_2;

    }

    iVar2 = FUN_0304f4ac(param_1,iVar3,auStack_38,&iStack_3c);

    if (iVar2 == 0) break;

    iVar2 = iVar4;

    if (iStack_3c + iStack_28 < iVar3 + iVar4) {

      iVar2 = iVar4 - ((iVar3 + iVar4) - (iStack_3c + iStack_28));

    }

    FUN_0304f104((param_2 - iVar4) + param_3,iVar2,auStack_38,iVar3);

    iVar3 = iVar3 + iVar2;

    iVar4 = iVar4 - iVar2;

  }

  return 0;

}
