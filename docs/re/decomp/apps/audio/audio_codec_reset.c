/**
 * audio_codec_reset @ 0x0302c932
 * Named via changelog-anchored lineage cluster (intro version).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void audio_codec_reset(void)



{

  if (*DAT_0302ca9c != 0xff) {

                    /* WARNING: Could not recover jumptable at 0x0302c94e. Too many branches */

                    /* WARNING: Treating indirect jump as call */

    (**(code **)(DAT_0302caa0 + *DAT_0302ca9c * 4))(0xc,0,0);

    return;

  }

  return;

}
