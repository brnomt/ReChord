/**
 * i2c_receive_helper @ 0x03070b9a
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint i2c_receive_helper(undefined4 param_1,undefined4 param_2,uint param_3,uint param_4)



{

  uint uVar1;

  uint uVar2;

  int iVar3;

  bool bVar4;

  undefined4 uStack_20;

  undefined4 uStack_1c;

  uint uStack_18;

  uint uStack_14;

  

  iVar3 = DAT_03070c74;

  uStack_14 = param_4;

  uStack_18 = param_3;

  uStack_1c = param_2;

  uStack_20 = param_1;

  for (; 0x1f < param_3; param_3 = param_3 - 0x20) {

    uVar1 = bitstream_getbits_be(&uStack_20,0x20);

    uVar2 = (uint)*(ushort *)(iVar3 + ((param_4 >> 8 ^ uVar1 >> 0x18) & 0xff) * 2) ^ param_4 << 8;

    uVar2 = (uint)*(ushort *)(iVar3 + ((uVar2 >> 8 ^ uVar1 >> 0x10) & 0xff) * 2) ^ uVar2 << 8;

    uVar2 = (uint)*(ushort *)(iVar3 + (((uVar2 ^ uVar1) & 0xffff) >> 8) * 2) ^ uVar2 << 8;

    param_4 = (uint)*(ushort *)(iVar3 + ((uVar1 ^ uVar2 >> 8) & 0xff) * 2) ^ uVar2 << 8;

  }

  uVar1 = param_3 >> 3;

  if (uVar1 != 0) {

    if (uVar1 != 1) {

      if (uVar1 != 2) {

        if (uVar1 != 3) goto LAB_03070c48;

        uVar1 = bitstream_getbits_be(&uStack_20,8);

        param_4 = (uint)*(ushort *)(iVar3 + ((uVar1 ^ param_4 >> 8) & 0xff) * 2) ^ param_4 << 8;

      }

      uVar1 = bitstream_getbits_be(&uStack_20,8);

      param_4 = (uint)*(ushort *)(iVar3 + ((uVar1 ^ param_4 >> 8) & 0xff) * 2) ^ param_4 << 8;

    }

    uVar1 = bitstream_getbits_be(&uStack_20,8);

    param_3 = param_3 & 7;

    param_4 = (uint)*(ushort *)(iVar3 + ((uVar1 ^ param_4 >> 8) & 0xff) * 2) ^ param_4 << 8;

  }

LAB_03070c48:

  iVar3 = param_3 - 1;

  if (iVar3 != -1) {

    do {

      uVar2 = bitstream_getbits_be(&uStack_20,1);

      uVar1 = param_4 >> 0xf;

      param_4 = param_4 << 1;

      if (((uVar2 ^ uVar1) & 1) != 0) {

        param_4 = param_4 ^ 0x8005;

      }

      bVar4 = iVar3 != 0;

      iVar3 = iVar3 + -1;

    } while (bVar4);

  }

  return param_4 & 0xffff;

}
