//****************************************************************************
//#include "../buffer/buffer.h"
#include "SysConfig.h"
//****************************************************************************

//****************************************************************************
#define NUMTAPS 13

//****************************************************************************
//
// The perisistent state of the sample rate converter.
//
//****************************************************************************
#ifndef _SRC_H_
#define _SRC_H_

typedef struct
{
    //
    // The internal, persistent state of the sample rate converter.
    //
    unsigned char ucInternal[12];
    short           *psFilter;
	short         lFilterIncr;
	short         lFilterOffset;
	short			lFilterSize;

	short Left_right[NUMTAPS*2];
    #ifdef _A2DP_SINK_
    short Last_sample[180*2]; // BY WP 20140603
    #endif
    int   last_sample_num;
    int   process_num;

} SRCState;

//static SRCState pSRC_st;

//****************************************************************************
//
// Function prototypes.
//
//****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern int SRCInit(SRCState *pSRC, unsigned long ulInputRate,
                   unsigned long ulOutputRate);
//extern BufferState *SRCGetInputBuffer(SRCState *pSRC);
extern long SRCFilter(SRCState *pSRC, short *psLeft, short *psRight,
                      long lNumSamples);
//extern long SRCNumSamplesAvailable(SRCState *pSRC);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif
