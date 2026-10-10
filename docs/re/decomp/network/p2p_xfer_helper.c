/**
 * p2p_xfer_helper @ 0x03086bf4
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


int p2p_xfer_helper(int *param_1,undefined4 param_2,undefined4 param_3,uint param_4)



{

  short sVar1;

  short sVar2;

  int iVar3;

  char cVar4;

  int iVar5;

  int iVar6;

  uint uVar7;

  uint uVar8;

  int iVar9;

  uint uVar10;

  int iVar11;

  int iVar12;

  int iVar13;

  int iVar14;

  bool bVar15;

  uint local_20;

  

  iVar3 = DAT_03086ffc;

  iVar11 = *param_1;

  iVar14 = 0;

  if (((*(int *)(iVar11 + 0x74) != 0) && (iVar13 = DAT_03086ffc, *(int *)(iVar11 + 0x80) < 2)) ||

     (cVar4 = *(char *)((int)param_1 + 0x23), iVar13 = iVar14, local_20 = param_4, cVar4 == '\0')) {

    return iVar13;

  }

LAB_03086c1e:

                    /* WARNING: Could not find normalized switch variable to match jumptable */

  switch(cVar4) {

  case '\x01':

    *(undefined1 *)((int)param_1 + 0x23) = 3;

  case '\x03':

    *(undefined1 *)(param_1 + 0x50) = 0;

    *(undefined1 *)((int)param_1 + 0x23) = 4;

  case '\x04':

  case '\x05':

    cVar4 = '\x06';

    break;

  case '\x02':

    if ((*(int *)(iVar11 + 0x74) != 0) && (*(int *)(iVar11 + 0x40) < 3)) {

      uVar7 = 0;

      uVar10 = *(uint *)(iVar11 + 0x80);

      while (1 < uVar10) {

        uVar7 = uVar7 + 1;

        uVar10 = *(uint *)(iVar11 + 0x80) >> (uVar7 & 0xff);

      }

      uVar10 = 0;

      uVar8 = uVar7;

      while (1 < uVar8) {

        uVar10 = uVar10 + 1;

        uVar8 = uVar7 >> (uVar10 & 0xff);

      }

      iVar13 = uVar10 + 1;

      iVar14 = wma_bitreader_getbits_copy_copy(param_1 + 0x21,iVar13 * 2);

      if (iVar14 < 0) {

        return iVar14;

      }

      iVar14 = wma_bitreader_getbits(param_1 + 0x21,iVar13,&local_20);

      if (iVar14 < 0) {

        return iVar14;

      }

      sVar1 = (short)(*(int *)(iVar11 + 0x9c) / (1 << (local_20 & 0xff)));

      iVar12 = (int)sVar1;

      iVar14 = wma_bitreader_getbits(param_1 + 0x21,iVar13,&local_20);

      if (iVar14 < 0) {

        return iVar14;

      }

      iVar5 = *(int *)(iVar11 + 0x9c);

      sVar2 = (short)(iVar5 / (1 << (local_20 & 0xff)));

      iVar9 = (int)sVar2;

      iVar6 = *(int *)(iVar11 + 0x88);

      bVar15 = SBORROW4(iVar12,iVar6);

      iVar13 = iVar12 - iVar6;

      if (iVar6 <= iVar12) {

        bVar15 = SBORROW4(iVar5,iVar12);

        iVar13 = iVar5 - iVar12;

      }

      if (iVar13 < 0 != bVar15) {

        return iVar3;

      }

      bVar15 = SBORROW4(iVar9,iVar6);

      iVar13 = iVar9 - iVar6;

      if (iVar6 <= iVar9) {

        bVar15 = SBORROW4(iVar5,iVar9);

        iVar13 = iVar5 - iVar9;

      }

      if (iVar13 < 0 != bVar15) {

        return iVar3;

      }

      iVar13 = 0;

      if (*(short *)(iVar11 + 0x26) != 0) {

        do {

          iVar12 = *(int *)(iVar11 + 0xd8) + iVar13 * 0xfc;

          *(short *)(*(int *)(*(int *)(iVar12 + 0xc0) + 8) + -2) = sVar1;

          **(undefined2 **)(iVar12 + 0xc0) = 0;

          **(short **)(*(int *)(iVar12 + 0xc0) + 8) = sVar2;

          **(undefined2 **)(*(int *)(iVar12 + 0xc0) + 0xc) = 0;

          **(short **)(iVar12 + 0xc0) = **(short **)(iVar12 + 0xc0) + 1;

          iVar13 = (int)(short)((short)iVar13 + 1);

        } while (iVar13 < (int)(uint)*(ushort *)(iVar11 + 0x26));

      }

    }

LAB_03086d42:

    cVar4 = '\v';

    break;

  case '\x06':

    local_20 = 0;

    cVar4 = '\v';

    break;

  case '\a':

    iVar14 = wma_bitreader_getbits(param_1 + 0x21,1,&local_20);

    if (iVar14 < 0) {

      return iVar14;

    }

    *(uint *)(iVar11 + 0x108) = local_20;

    if (local_20 == 0) {

      cVar4 = '\t';

    }

    else {

      cVar4 = '\b';

    }

    break;

  case '\b':

    uVar7 = 0;

    uVar10 = *(uint *)(iVar11 + 0x98);

    while (1 < uVar10) {

      uVar7 = uVar7 + 1;

      uVar10 = *(uint *)(iVar11 + 0x98) >> (uVar7 & 0xff);

    }

    local_20 = 0;

    iVar14 = wma_bitreader_getbits(param_1 + 0x21,uVar7,&local_20);

    if (iVar14 < 0) {

      return iVar14;

    }

    *(uint *)(iVar11 + 0x10c) = local_20;

    cVar4 = '\t';

    break;

  case '\t':

    iVar14 = wma_bitreader_getbits(param_1 + 0x21,1,&local_20);

    if (iVar14 < 0) {

      return iVar14;

    }

    if (local_20 == 0) goto LAB_03086d42;

    cVar4 = '\n';

    break;

  case '\n':

    uVar7 = 0;

    uVar10 = *(uint *)(iVar11 + 0x98);

    while (1 < uVar10) {

      uVar7 = uVar7 + 1;

      uVar10 = *(uint *)(iVar11 + 0x98) >> (uVar7 & 0xff);

    }

    local_20 = 0;

    iVar14 = wma_bitreader_getbits(param_1 + 0x21,uVar7,&local_20);

    if (iVar14 < 0) {

      return iVar14;

    }

    *(uint *)(iVar11 + 0x110) = local_20;

    cVar4 = '\v';

    *(undefined1 *)((int)param_1 + 0x23) = 0xb;

    goto LAB_03086c1e;

  case '\v':

    *(undefined1 *)((int)param_1 + 0x23) = 0;

    return iVar14;

  default:

    goto switchD_03086c26_default;

  }

  *(char *)((int)param_1 + 0x23) = cVar4;

  goto LAB_03086c1e;

switchD_03086c26_default:

  if (cVar4 == '\0') {

    return iVar14;

  }

  goto LAB_03086c1e;

}
