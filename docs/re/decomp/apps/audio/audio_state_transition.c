/**
 * audio_state_transition @ 0x0302b414
 * Named via call-graph propagation from named cluster neighbors.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


void audio_state_transition(undefined4 param_1)



{

  AudioStop(1);

  FUN_030059bc();

  AudioStateHandler(param_1);

  FUN_030059bc();

  shared_frame_epilogue_c6b4(0x110);

  return;

}
