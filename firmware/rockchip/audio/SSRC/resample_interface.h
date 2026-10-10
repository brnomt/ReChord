
#ifndef _RESAMPLE_INTERFACE_H
#define _RESAMPLE_INTERFACE_H

//#define SSRC
#include "Src.h"

SRCState *resample_init(int nb_channels, int in_rate, int out_rate);
int resampler_process(SRCState *st, short *in, int *in_len, short *out, int *out_len);

#endif 