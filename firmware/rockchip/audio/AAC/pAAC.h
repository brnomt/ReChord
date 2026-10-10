

#include "audio_main.h"
#include <stdio.h>
#ifdef AAC_DEC_INCLUDE
#include "SysFindFile.h"

typedef struct {
	int rate;
	int channels;
	int frames;
	int blocks;
	
	unsigned int framelen;
	int curframe;
	long curtime;
	long totaltime;
	int bitrate;
}AAC_TIMEINFO;
typedef void *HAACDecoder;


extern 	long GetCurTime_AAC();

extern AAC_TIMEINFO *GetInfo_AAC();

//extern void InitTime(unsigned int len,FILE *pRawFileCache);
extern void InitAACDataTable(void);
extern HAACDecoder AACInitDecoder(void);


extern int  InitTimeForADTS(FILE *fp,int filelen,int offsethelun);


extern int AACFlushCodec(HAACDecoder hAACDecoder);

extern void AACFreeDecoder(HAACDecoder hAACDecoder);

extern int AACDecode(HAACDecoder hAACDecoder, unsigned char **inbuf, int *bytesLeft, short *outbuf);
extern int SeekTime_AAC(long offsettime,HAACDecoder hAACDecoder,FILE *fp);


extern unsigned long 
AACDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
               unsigned long ulParam2, unsigned long ulParam3);
#endif

