/**
 * flac_frame_header_parse @ 0x030df30e
 * Tags: codec, flac, parser
 *
 * Introduced in v3.6.0 (FLAC decoder rewrite per changelog + string diff)
 * Named via cross-version lineage analysis; decompiled from Ghidra v3.7.0.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


undefined4 flac_frame_header_parse(int param_1,int param_2)



{

  if ((*(byte *)(param_2 + 4) & 0x7f) == 0 &&

      ((uint)*(byte *)(param_2 + 5) << 0x10 | (uint)*(byte *)(param_2 + 6) << 8 |

      (uint)*(byte *)(param_2 + 7)) == 0x22) {

    ShellHelp(param_1,param_2 + 8);

    *(undefined4 *)(param_1 + 0x44) = 1;

    return 0;

  }

  return DAT_030df5a0;

}
