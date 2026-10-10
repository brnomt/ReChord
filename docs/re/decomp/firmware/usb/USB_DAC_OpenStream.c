/**
 * USB_DAC_OpenStream @ 0x0302b80a
 * Tags: usb, audio
 * Auto-exported from Ghidra decompilation
 */

#include "decomp_support.h"
#include "decomp_globals.h"


/* USB DAC / storage mode open. Called from MusicInit when g_usb_state==99. Toggles mode 2<->4 via

   ROM, posts event 0xf5 on success. */



undefined4 USB_DAC_OpenStream(undefined4 param_1)



{

  int iVar1;

  undefined2 *puVar2;

  int iVar3;

  undefined4 uVar4;

  

  uVar4 = 0;

  iVar3 = FUN_02ffe3a0(DAT_0302b9c8,param_1);

  puVar2 = DAT_0302b9b8;

  *DAT_0302b9b8 = (short)iVar3;

  iVar1 = DAT_0302b9a4;

  if (iVar3 == 0) {

    if (*(char *)(DAT_0302b9a4 + 0xe) == '\x02') {

      if ((*(char *)(DAT_0302b9a4 + 0x1f) != '\0') && (iVar3 = FUN_02ff7890(), iVar3 == 1)) {

        *(undefined1 *)(iVar1 + 0xe) = 4;

        FUN_0300ac14();

        FUN_02ffbf72(*(undefined1 *)(iVar1 + 0xe));

      }

      iVar3 = FUN_02ffe3a0(DAT_0302b9c8,param_1);

      *puVar2 = (short)iVar3;

    }

    else {

      if (*(char *)(DAT_0302b9a4 + 0xe) != '\x04') {

        return 0;

      }

      if (*(char *)(DAT_0302b9a4 + 0x1f) == '\0') {

        return 0;

      }

      *(undefined1 *)(DAT_0302b9a4 + 0xe) = 2;

      FUN_02ffbf72(2);

      FUN_0300ac16();

      iVar3 = FUN_02ffe3a0(DAT_0302b9c8,param_1);

      *puVar2 = (short)iVar3;

    }

    if (iVar3 == 0) {

      *(undefined1 *)(iVar1 + 0xe) = 2;

      FUN_02ffbf72(2);

      FUN_0300ac16();

      uVar4 = 1;

      shared_frame_epilogue_c6b4(0xf5);

    }

  }

  return uVar4;

}
