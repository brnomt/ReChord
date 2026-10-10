/**
 * DICTDECODER_InitStream @ 0x0304fb44
 * Tags: codec, parser
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* Parse DICTDECODER data-stream header (magic 'VaT '/'VAT ' @ g_dict_magic_VaT/VAT).

   Args: pHeader = 0x28-byte header struct (out), hStream = stream handle.

   Reads magic, format code (0x70/0x71/0xf1 + subtype), sample params, sizes; then

   dict_build_frame_index for seek table.

   Returns 1 on success, 0 on failure. Emits 'DICTDECODER_InitDataStream 0/1/2' on error paths. */



int DICTDECODER_InitStream(short *pHeader,int hStream)



{

  short sVar1;

  int iVar2;

  uint uVar3;

  bool bVar4;

  undefined4 local_60;

  undefined4 local_5c;

  undefined4 local_58;

  undefined4 local_54;

  uint *local_50;

  undefined4 local_4c;

  undefined4 local_48;

  undefined4 local_44;

  undefined4 local_40;

  undefined4 local_3c;

  undefined4 local_38;

  undefined4 local_34;

  uint local_30;

  undefined1 auStack_2c [4];

  undefined1 local_28 [20];

  

  if (pHeader == (short *)0x0 || hStream == -1) {

    FUN_03033618(s_DICTDECODER_InitDataStream_0_0304fd0c);

    return 0;

  }

  pHeader[0] = 0;

  pHeader[1] = 0;

  pHeader[2] = 0;

  pHeader[3] = 0;

  pHeader[4] = 0;

  pHeader[5] = 0;

  pHeader[6] = 0;

  pHeader[7] = 0;

  pHeader[8] = 0;

  pHeader[9] = 0;

  pHeader[10] = 0;

  pHeader[0xb] = 0;

  pHeader[0xc] = 0;

  pHeader[0xd] = 0;

  pHeader[0xe] = 0;

  pHeader[0xf] = 0;

  pHeader[0x10] = 0;

  pHeader[0x11] = 0;

  pHeader[0x12] = 0;

  pHeader[0x13] = 0;

  local_28[0] = 0;

  iVar2 = FUN_0304f550(hStream,local_28);

  if (iVar2 == 0) {

    FUN_03033618(s_DICTDECODER_InitDataStream_1_0304fd2c);

    return 0;

  }

  iVar2 = FUN_0304ed3e(hStream,0);

  if (iVar2 != 0) {

    FUN_03033618(s_DICTDECODER_InitDataStream_2_0304fcec);

    return 0;

  }

  local_30 = 0;

  iVar2 = dict_stream_read(hStream,4,&local_30);

  if (iVar2 != 4) {

    return 0;

  }

  uVar3 = g_dwDictMagicVaT;

  if (local_30 != g_dwDictMagicVaT) {

    uVar3 = g_dwDictMagicVAT;

  }

  if (local_30 != g_dwDictMagicVaT && local_30 != uVar3) {

    return 0;

  }

  iVar2 = dict_stream_read(hStream,2,pHeader);

  if (iVar2 != 2) {

    return 0;

  }

  iVar2 = dict_stream_read(hStream,2,pHeader + 1);

  if (iVar2 != 2) {

    return 0;

  }

  iVar2 = dict_stream_read(hStream,1,pHeader + 2);

  if (iVar2 != 1) {

    return 0;

  }

  iVar2 = dict_stream_read(hStream,1,auStack_2c);

  if (iVar2 != 1) {

    return 0;

  }

  *(undefined1 *)((int)pHeader + 5) = 2;

  iVar2 = dict_stream_read(hStream,4,&local_30);

  if (iVar2 != 4) {

    return 0;

  }

  iVar2 = dict_stream_read(hStream,4,&local_30);

  if (iVar2 != 4) {

    return 0;

  }

  iVar2 = dict_stream_read(hStream,4,&local_30);

  if (iVar2 != 4) {

    return 0;

  }

  local_30 = 0;

  iVar2 = dict_stream_read(hStream,4,&local_30);

  if (iVar2 != 4) {

    return 0;

  }

  *(uint *)(pHeader + 4) = local_30;

  if (*pHeader == 0x70 || *pHeader == 0x71) {

    if (pHeader[1] != 0x62) goto LAB_0304fc62;

    pHeader[7] = 0;

    pHeader[6] = 0;

  }

  else {

LAB_0304fc62:

    iVar2 = dict_stream_read(hStream,2,pHeader + 6);

    if (iVar2 != 2) {

      return 0;

    }

    uVar3 = dict_stream_read(hStream,2,pHeader + 7);

    bVar4 = uVar3 == 2;

    if (bVar4) {

      uVar3 = (uint)(ushort)pHeader[7];

    }

    if (!bVar4 || uVar3 != 0x2a4) {

      return 0;

    }

  }

  iVar2 = dict_stream_read(hStream,2,pHeader + 8);

  if (iVar2 != 2) {

    return 0;

  }

  iVar2 = dict_stream_read(hStream,2,pHeader + 9);

  if (iVar2 != 2) {

    return 0;

  }

  iVar2 = dict_stream_read(hStream,2,pHeader + 10);

  if (iVar2 != 2) {

    return 0;

  }

  iVar2 = dict_stream_read(hStream,4,&local_30);

  if (iVar2 != 4) {

    return 0;

  }

  iVar2 = dict_stream_read(hStream,2,pHeader + 0xb);

  if (iVar2 != 2) {

    return 0;

  }

  sVar1 = *pHeader;

  if (sVar1 == 0x70) {

    if (pHeader[1] == 0x42) {

      sVar1 = pHeader[0xb];

      goto joined_r0x0304fd6e;

    }

  }

  else {

    if (sVar1 == 0xf1) {

      if (pHeader[1] != 0x12) goto LAB_0304fd86;

      sVar1 = pHeader[0xb];

joined_r0x0304fd6e:

      if (sVar1 != 0x32) {

        return 0;

      }

      goto LAB_0304fd86;

    }

    if (sVar1 != 0x70 && sVar1 != 0x71) goto LAB_0304fd86;

  }

  if ((pHeader[1] == 0x62) && (pHeader[0xb] != 0x2e)) {

    return 0;

  }

LAB_0304fd86:

  iVar2 = dict_stream_read(hStream,4,pHeader + 0xc);

  if ((iVar2 == 4) && (iVar2 = dict_stream_read(hStream,4,pHeader + 0xe), iVar2 == 4)) {

    pHeader[0x10] = 4;

    if (*pHeader != 0x75 && *pHeader != 0x76) {

      local_4c = 0;

      local_38 = 0;

      local_60 = 1;

      local_5c = 0;

      local_3c = 0;

      local_50 = &local_30;

      local_58 = 0;

      local_54 = 0;

      local_48 = 0;

      local_44 = 0;

      local_40 = 0;

      local_34 = 0;

      iVar2 = dict_build_frame_index(pHeader,hStream,&local_60);

      if (iVar2 == 0) {

        return 0;

      }

      *(uint *)(pHeader + 0x12) = *local_50 - (uint)(ushort)pHeader[10];

    }

    return 1;

  }

  return 0;

}
