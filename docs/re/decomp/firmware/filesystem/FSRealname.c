/**
 * FSRealname @ 0x030b1328
 * Tags: fs, utility
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"




undefined4 FUN_030b1328(void)



{

  int iVar1;

  undefined4 *puVar2;

  int *piVar3;

  int iVar4;

  

  puVar2 = DAT_030b1520;

  iVar4 = DAT_030b151c;

  if (*(int *)(DAT_030b151c + 0x204) != 0) {

    (*(code *)*DAT_030b1520)();

    *(undefined4 *)(iVar4 + 0x204) = 0;

  }

  iVar1 = DAT_030b1518;

  *(undefined2 *)(iVar4 + 0x200) = 0;

  *(undefined2 *)(iVar4 + 0x202) = 0;

  piVar3 = *(int **)(iVar1 + 0x2c);

  if (piVar3 != (int *)0x0) {

    iVar4 = *piVar3;

  }

  if (piVar3 != (int *)0x0 && iVar4 != 0) {

    if (*(int *)(iVar4 + 0x204) != 0) {

      (*(code *)*puVar2)();

      *(undefined4 *)(iVar4 + 0x204) = 0;

    }

    *(undefined2 *)(iVar4 + 0x200) = 0;

    *(undefined2 *)(iVar4 + 0x202) = 0;

  }

  piVar3 = *(int **)(iVar1 + 0x3c);

  if (piVar3 != (int *)0x0) {

    iVar4 = *piVar3;

  }

  if (piVar3 != (int *)0x0 && iVar4 != 0) {

    if (*(int *)(iVar4 + 0x204) != 0) {

      (*(code *)*puVar2)();

      *(undefined4 *)(iVar4 + 0x204) = 0;

    }

    *(undefined2 *)(iVar4 + 0x200) = 0;

    *(undefined2 *)(iVar4 + 0x202) = 0;

  }

  return 0;

}




