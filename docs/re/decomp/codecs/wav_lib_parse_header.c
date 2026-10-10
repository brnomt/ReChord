/**
 * wav_lib_parse_header @ 0x030a46c8
 * Tags: codec, wav
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4

wav_lib_parse_header(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)



{

  char cVar1;

  undefined4 *puVar2;

  undefined4 *puVar3;

  undefined4 *puVar4;

  undefined4 *puVar5;

  uint uVar6;

  int iVar7;

  uint uVar8;

  undefined4 uVar9;

  int iVar10;

  char *pcVar11;

  int iVar12;

  bool bVar13;

  bool bVar14;

  undefined4 local_28;

  

  puVar4 = DAT_030a4a04;

  puVar2 = DAT_030a49fc;

  local_28 = param_4;

  (*(code *)*DAT_030a4a04)(0,0,*DAT_030a49fc);

  puVar3 = DAT_030a4a00;

  uVar6 = (*(code *)*DAT_030a4a00)(param_1 + 8,0xc,*puVar2);

  bVar13 = uVar6 == 0xc;

  *(uint *)(param_1 + 0x8218) = uVar6;

  if (0xb < uVar6) {

    bVar13 = *(char *)(param_1 + 8) == 'R';

  }

  if (!bVar13) {

LAB_030a472c:

    hifi_debug_printf_ovl_09e0

              (s_FILE___s__LINE___d__030a4584,s_______Common_Codec_Audio_Wav_WAV_030a4554,0x207);

    return 0;

  }

  cVar1 = *(char *)(param_1 + 9);

  bVar13 = cVar1 == 'I';

  if (bVar13) {

    cVar1 = *(char *)(param_1 + 10);

  }

  if (!bVar13 || cVar1 != 'F') goto LAB_030a472c;

  cVar1 = *(char *)(param_1 + 0xb);

  bVar13 = cVar1 == 'F';

  if (bVar13) {

    cVar1 = *(char *)(param_1 + 0x10);

  }

  if (!bVar13 || cVar1 != 'W') goto LAB_030a472c;

  cVar1 = *(char *)(param_1 + 0x11);

  bVar13 = cVar1 == 'A';

  if (bVar13) {

    cVar1 = *(char *)(param_1 + 0x12);

  }

  bVar14 = bVar13 && cVar1 == 'V';

  if (bVar13 && cVar1 == 'V') {

    bVar14 = *(char *)(param_1 + 0x13) == 'E';

  }

  if (!bVar14) goto LAB_030a472c;

  iVar7 = 0xc;

  *(undefined4 *)(param_1 + 0x822c) = 0;

  while( true ) {

    *(int *)(param_1 + 0x8214) = iVar7;

    while( true ) {

      while( true ) {

        (*(code *)*puVar4)(*(undefined4 *)(param_1 + 0x8214),0,*puVar2);

        iVar7 = (*(code *)*puVar3)(param_1 + 8,8,*puVar2);

        if (iVar7 != 8) {

          hifi_debug_printf_ovl_09e0

                    (s_FILE___s__LINE___d__030a4584,s_______Common_Codec_Audio_Wav_WAV_030a4554,

                     0x21d);

          return 0;

        }

        local_28 = *(undefined4 *)(param_1 + 8);

        uVar6 = (uint)*(byte *)(param_1 + 0xc) | *(int *)(param_1 + 0xd) << 8;

        *(int *)(param_1 + 0x8214) = *(int *)(param_1 + 0x8214) + 8;

        iVar7 = FUN_0309f6e8(DAT_030a4a08,&local_28,4);

        if (iVar7 != 0) break;

        *(uint *)(param_1 + 0x8214) = *(int *)(param_1 + 0x8214) + uVar6;

      }

      iVar7 = FUN_0309f6e8(DAT_030a4a0c,&local_28,4);

      if (iVar7 != 0) break;

      iVar7 = wav_lib_decode(param_1,uVar6);

      if (iVar7 != 1) {

        hifi_debug_printf_ovl_09e0

                  (s_FILE___s__LINE___d__030a4584,s_______Common_Codec_Audio_Wav_WAV_030a4554,0x234)

        ;

        return 0;

      }

    }

    iVar7 = FUN_0309f6e8(DAT_030a4a10,&local_28,4);

    puVar5 = DAT_030a4a14;

    if (iVar7 == 0) break;

    iVar7 = *(int *)(param_1 + 0x8214) + uVar6;

  }

  *(int *)(param_1 + 0x821c) = *(int *)(param_1 + 0x8214);

  uVar8 = (*(code *)*puVar5)(*puVar2);

  if (uVar8 < uVar6) {

    uVar6 = (*(code *)*puVar5)(*puVar2);

  }

  *(uint *)(param_1 + 0x8220) = uVar6;

  iVar12 = 0;

  uVar9 = (*(code *)*puVar3)(param_1 + 8,0x1000,*puVar2);

  iVar7 = 1;

  *(undefined4 *)(param_1 + 0x8218) = uVar9;

  do {

    iVar10 = param_1 + iVar12;

    iVar12 = iVar12 + 1;

    pcVar11 = (char *)(iVar10 + 8);

    cVar1 = *pcVar11;

    if (cVar1 == -1) {

      cVar1 = *(char *)(iVar10 + 9);

      bVar13 = cVar1 == '\x1f';

      if (bVar13) {

        cVar1 = *(char *)(iVar10 + 10);

      }

      bVar14 = bVar13 && cVar1 == '\0';

      if (bVar13 && cVar1 == '\0') {

        bVar14 = *(char *)(iVar10 + 0xb) == -0x18;

      }

      if (bVar14) {

        bVar13 = (~*(byte *)(iVar10 + 0xc) & 0xf0) == 0;

        if (bVar13) {

          pcVar11 = (char *)(uint)*(byte *)(iVar10 + 0xd);

        }

        if (bVar13 && pcVar11 == &DAT_00000007) goto LAB_030a492c;

      }

    }

    else if (cVar1 == '\x1f') {

      cVar1 = *(char *)(iVar10 + 9);

      bVar13 = cVar1 == -1;

      if (bVar13) {

        cVar1 = *(char *)(iVar10 + 10);

      }

      if (bVar13 && cVar1 == -0x18) {

        cVar1 = *(char *)(iVar10 + 0xb);

        bVar13 = cVar1 == '\0';

        if (bVar13) {

          cVar1 = *(char *)(iVar10 + 0xc);

        }

        if ((bVar13 && cVar1 == '\a') && ((~*(byte *)(iVar10 + 0xd) & 0xf0) == 0))

        goto LAB_030a492c;

      }

    }

    else if (cVar1 == -2) {

      cVar1 = *(char *)(iVar10 + 9);

      bVar13 = cVar1 == '\x7f';

      if (bVar13) {

        cVar1 = *(char *)(iVar10 + 10);

      }

      bVar14 = bVar13 && cVar1 == '\x01';

      if (bVar13 && cVar1 == '\x01') {

        bVar14 = *(char *)(iVar10 + 0xb) == -0x80;

      }

      if (bVar14) {

LAB_030a492c:

        hifi_debug_printf_ovl_09e0(&LAB_030a4a38);

        return 0;

      }

    }

    else {

      bVar13 = cVar1 == '\x7f';

      if (bVar13) {

        cVar1 = *(char *)(iVar10 + 9);

      }

      if (bVar13 && cVar1 == -2) {

        bVar13 = *(char *)(iVar10 + 10) == -0x80;

        if (bVar13) {

          pcVar11 = (char *)(uint)*(byte *)(iVar10 + 0xb);

        }

        if (bVar13 && pcVar11 == (char *)0x1) goto LAB_030a492c;

      }

    }

    iVar7 = iVar7 + 1;

  } while (iVar7 < 0xffb);

  (*(code *)*puVar4)(*(undefined4 *)(param_1 + 0x821c),0,*puVar2);

  iVar12 = *(int *)(param_1 + 0x822c);

  iVar7 = iVar12 + -0xac44;

  if (iVar12 == 0xac44) goto LAB_030a498c;

  if (iVar12 < 0xac45) {

    if (iVar12 == 16000) goto LAB_030a498c;

    if (iVar12 < 0x3e81) {

      if (iVar12 != 6000 && iVar12 != 8000) {

        iVar7 = 0;

        if (iVar12 != 0x2b11) {

          iVar7 = iVar12 + -0x2e11;

        }

        if (iVar12 != 0x2b11 && iVar7 != 0xcf) goto LAB_030a491c;

      }

      goto LAB_030a498c;

    }

    bVar13 = iVar12 == 0x5622;

    iVar7 = 0;

    if (!bVar13) {

      iVar7 = iVar12 + -24000;

      bVar13 = iVar7 == 0;

    }

    if (!bVar13) {

      bVar13 = iVar7 == 8000;

    }

  }

  else {

    if (iVar7 == 0xcabc) goto LAB_030a498c;

    if (iVar7 < 0xcabd) {

      iVar7 = iVar12 + -48000;

      bVar13 = iVar7 != 0;

      bVar14 = iVar7 != 16000;

      if (bVar13 && bVar14) {

        iVar7 = iVar12 + -0x15380;

      }

      if ((!bVar13 || !bVar14) || iVar7 == 0x508) goto LAB_030a498c;

      goto LAB_030a491c;

    }

    bVar13 = iVar12 == 0x1f400;

    if (!bVar13) {

      iVar7 = iVar12 + -0x2b110;

      bVar13 = iVar7 == 0;

    }

    if (!bVar13) {

      bVar13 = iVar7 == 0x3cf0;

    }

  }

  if (!bVar13) {

LAB_030a491c:

    hifi_debug_printf_ovl_09e0(&DAT_030a4a18,s_______Common_Codec_Audio_Wav_WAV_030a4554,0x26b);

    return 0;

  }

LAB_030a498c:

  uVar8 = *(uint *)(param_1 + 0x8220);

  uVar6 = *(uint *)(param_1 + 0x8228);

  *(uint *)(param_1 + 0x8224) = uVar8;

  *(uint *)(param_1 + 0x823c) =

       ((uVar8 - uVar6 * (uVar8 / uVar6)) * 1000) / uVar6 + (uVar8 / uVar6) * 1000;

  *(undefined4 *)(param_1 + 0x8240) = 0;

  (*(code *)*puVar4)(*(undefined4 *)(param_1 + 0x821c),0,*puVar2);

  *(undefined4 *)(param_1 + 0x8214) = 0;

  if (*(short *)(param_1 + 0x8238) == 1) {

    *(undefined2 *)(param_1 + 0x8234) = 0x1000;

  }

  else {

    uVar9 = (*(code *)*puVar3)(param_1 + 8,0x1000,*puVar2);

    *(undefined4 *)(param_1 + 0x8218) = uVar9;

  }

  return 1;

}
