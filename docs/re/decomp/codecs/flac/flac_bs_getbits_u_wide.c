/**
 * flac_bs_getbits_u_wide @ 0x030df46c
 * Tags: codec, flac, bitstream
 *
 * Introduced in v3.6.0 (FLAC decoder rewrite per changelog + string diff)
 * Named via cross-version lineage analysis; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint flac_bs_getbits_u_wide(int *param_1,uint param_2,int param_3,int param_4)



{

  int iVar1;

  int iVar2;

  uint uVar3;

  int iVar4;

  uint uVar5;

  uint uVar6;

  bool bVar7;

  bool bVar8;

  

  uVar5 = param_1[2];

  iVar1 = flac_bs_bswap32(*param_1 + ((int)uVar5 >> 3));

  uVar6 = iVar1 << (uVar5 & 7);

  iVar2 = flac_clz32(uVar6 | 1);

  iVar2 = 0x1f - iVar2;

  uVar3 = iVar2 - param_2;

  bVar8 = SBORROW4(uVar3,7);

  iVar1 = uVar3 - 7;

  bVar7 = uVar3 == 7;

  if (6 < (int)uVar3) {

    iVar4 = 0x20 - iVar2;

    bVar8 = SBORROW4(param_3,iVar4);

    iVar1 = param_3 - iVar4;

    bVar7 = param_3 == iVar4;

  }

  if (bVar7 || iVar1 < 0 != bVar8) {

    iVar1 = 0;

    while ((iVar1 < param_3 && (-1 < (int)uVar6))) {

      uVar5 = uVar5 + 1;

      iVar2 = flac_bs_bswap32(*param_1 + ((int)uVar5 >> 3));

      iVar1 = iVar1 + 1;

      uVar6 = iVar2 << (uVar5 & 7);

    }

    uVar3 = uVar5 + 1;

    param_3 = param_3 + -1;

    if (iVar1 != param_3 && param_3 <= iVar1) {

      uVar3 = 0xffffffff;

    }

    if (iVar1 < param_3) {

      if (param_2 == 0) {

        uVar5 = 0;

      }

      else {

        uVar5 = (uVar6 << 1) >> (0x20 - param_2 & 0xff);

        uVar3 = uVar3 + param_2;

      }

      param_1[2] = uVar3;

      uVar3 = (iVar1 << (param_2 & 0xff)) + uVar5;

    }

    else if (iVar1 == param_3) {

      param_1[2] = uVar3 + param_4;

      uVar3 = ((uVar6 << 1) >> (0x20U - param_4 & 0xff)) + 1;

    }

  }

  else {

    uVar3 = (uVar6 >> (uVar3 & 0xff)) + (0x1e - iVar2 << (param_2 & 0xff));

    param_1[2] = (param_2 - iVar2) + uVar5 + 0x20;

  }

  return -(uVar3 & 1) ^ (int)uVar3 >> 1;

}
