/**
 * Mp3GenreParse @ 0x0302744c
 * Structural-match v2 (validated by same-module callees).
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void Mp3GenreParse(void)



{

  *DAT_03027464 = 0x10;

  mp3_bitstream_getbits();

  mp3_bitstream_getbits(0x10);

  FMControlTask_Enter();

  return;

}
