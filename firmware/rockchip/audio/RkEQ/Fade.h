#ifndef _H_AUDIO_FADE
#define _H_AUDIO_FADE

typedef long    fade_short;
typedef long long   fade_long;
#define fade_max_coef   16777215
#define fade_scale   24 

#define FADE_IN     0
#define FADE_OUT    1
#define FADE_NULL   -1

//initialization.
//begin:the frist specimen serial number,len:length  type: 0-fade in 1-fade out.
void FadeInit(long begin,long len,int type);
void DC_filter(short *pwBuffer, unsigned short frameLen);
long FadeDoOnce();
void FadeProcess(fade_short *pwBuffer, unsigned short frameLen);

//#endif

#endif
