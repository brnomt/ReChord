/**
 * audio_codec_cleanup @ 0x0302ca40
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void audio_codec_cleanup(undefined4 param_1,undefined4 param_2)



{

  if (*DAT_0302ca9c != 0xff) {

                    /* WARNING: Could not recover jumptable at 0x0302ca5c. Too many branches */

                    /* WARNING: Treating indirect jump as call */

    (**(code **)(DAT_0302caa0 + *DAT_0302ca9c * 4))(10,param_1,param_2,0);

    return;

  }

  return;

}
