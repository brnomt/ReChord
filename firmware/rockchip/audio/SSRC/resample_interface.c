
#include "SysConfig.h" 
#include "stdio.h"
#include "resample_interface.h"
#include "audio_main.h"
#include "SysInclude.h"


#define _ATTR_AUDIO_TEXT_     
#define _ATTR_AUDIO_DATA_     
#define _ATTR_AUDIO_BSS_      

//#define _ATTR_AUDIO_SBC_ENCODE_TEXT_     
//#define _ATTR_AUDIO_SBC_ENCODE_DATA_     
//#define _ATTR_AUDIO_SBC_ENCODE_BSS_      

#ifdef SSRC

//#pragma arm section code ="SSRCCode", rodata = "SSRCData", rwdata = "SSRCData", zidata = "SSRCData"
#include "Src.h"

_ATTR_AUDIO_BSS_ static SRCState pSRC_st;
_ATTR_AUDIO_BSS_ static SRCState pSRC_st_44100;
_ATTR_AUDIO_TEXT_
SRCState *resample_init(int nb_channels, int in_rate, int out_rate)
{
	SRCState  *resampler;	
	int err = 0;	
    ModuleOverlay(MODULE_ID_SRC, MODULE_OVERLAY_ALL);
    switch(in_rate)
    {
        case 48000:
        if(out_rate == 44120)
        {
            ModuleOverlay(MODULE_ID_SRC_TABLE_48_44120, MODULE_OVERLAY_CODE);
        }
        else
        {
            ModuleOverlay(MODULE_ID_SRC_TABLE_48_44, MODULE_OVERLAY_CODE);
        }
        
        break;
        case 32000:
        ModuleOverlay(MODULE_ID_SRC_TABLE_32_44, MODULE_OVERLAY_CODE);
        break;

        case 24000:
        ModuleOverlay(MODULE_ID_SRC_TABLE_24_44, MODULE_OVERLAY_CODE);
        break;
        case 22050:
        ModuleOverlay(MODULE_ID_SRC_TABLE_22_44, MODULE_OVERLAY_CODE);
        break;
        case 16000:
        ModuleOverlay(MODULE_ID_SRC_TABLE_16_44, MODULE_OVERLAY_CODE);
        break;

        case 12000:
        ModuleOverlay(MODULE_ID_SRC_TABLE_12_44, MODULE_OVERLAY_CODE);
        break;
        case  11025:
        ModuleOverlay(MODULE_ID_SRC_TABLE_11_44, MODULE_OVERLAY_CODE);   
        break;

        case  8000:
        ModuleOverlay(MODULE_ID_SRC_TABLE_8_44, MODULE_OVERLAY_CODE);   
        break;

        case  44100:
        if(out_rate == 44120)
        {
            ModuleOverlay(MODULE_ID_SRC_TABLE_44_44120, MODULE_OVERLAY_CODE);
            SRCInit(&pSRC_st,in_rate,48000);
            SRCInit(&pSRC_st_44100,48000,out_rate+1);
	        return &pSRC_st_44100;
        }

        break;
    }
	SRCInit(&pSRC_st,in_rate,out_rate);
   
	return &pSRC_st;
}
_ATTR_AUDIO_TEXT_
int resampler_process(SRCState *st, short *in, int *in_len, short *out, int *out_len)//���롢�����˫����short�ܳ���
{
	int err;
	int len;
    if(st == NULL)
    {
       st =  &pSRC_st;
    }

    #ifdef _A2DP_SOUCRE_
	len =  *in_len;
    memcpy((char*)&out[len*5],(char*)in,(len<<1) );
	*out_len = SRCFilter(st, &out[len*5],out,len);
	#else
    len =  *in_len;
    *out_len = SRCFilter(st, in,out,len);
    #endif
	
	return 0;
}

#pragma arm section code

#endif