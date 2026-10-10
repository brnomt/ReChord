/**
 * flac_decode_samplerate @ 0x030e097c
 * Named via call-graph propagation from named cluster neighbors.
 */

#include "decomp_support.h"
#include "decomp_globals.h"


uint flac_decode_samplerate(int param_1,uint param_2)



{

  return *(uint *)(param_1 * 0x3c + 0x1060008) & param_2;

}
