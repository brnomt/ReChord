/**
 * dac_gain_curve_apply @ 0x030098e4
 * Tags: audio, volume (**was** `os_delay_ms`)
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void dac_gain_curve_apply(int *param_1,int param_2,int param_3)



{

  char in_ZR;

  

  if (in_ZR == '\0') {

    in_ZR = param_2 == 1;

  }

  if (in_ZR == '\0') {

    in_ZR = param_2 == 3;

  }

  if (in_ZR == '\0') {

    *(uint *)(param_3 + 0xc) =

         (uint)(DAT_03009ce0 * (*(int *)(*param_1 + 0xc) + -0x1db0)) / 0x578 & 0xffff;

    return;

  }

  *(uint *)(param_3 + 0xc) = (uint)((*(int *)(*param_1 + 0xc) + -0x21fc) * 200) / 0x834 & 0xffff;

  return;

}
