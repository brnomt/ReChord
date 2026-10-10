
#include "audio_globals.h"
#include "SysConfig.h"   /* _SBC_ENCODE_ feature gate */
#include "SysInclude.h"

#include "sbc_enc.h"

#include "sbc_enc_interface.h"

#ifdef _SBC_ENCODE_
_ATTR_AUDIO_SBC_ENCODE_DATA_ void * Psbc_pcb = (void *)0;

int sbc_bitpool_max = 53;
//_ATTR_AUDIO_DATA_ unsigned char  isSbcEncInited = 0;

_ATTR_AUDIO_TEXT_
void sbc_set_sbc_bitpool_max(uint32 bitpool)
{
    sbc_bitpool_max = bitpool;
}

_ATTR_AUDIO_TEXT_
int sbc_get_sbc_bitpool_max(void)
{
    int bitpool_max;
    int avdtp_bitpool;
    avdtp_bitpool = avdtp_get_sbc_bitpool();
    bitpool_max = sbc_bitpool_max > avdtp_bitpool ? avdtp_bitpool : sbc_bitpool_max;
    return bitpool_max;
}

_ATTR_AUDIO_TEXT_
void *  sbc_enc_init()
{
    //if(isSbcEncInited == 0)
    {
        ModuleOverlay(MODULE_ID_SBC_ENCODE, MODULE_OVERLAY_ALL);

    //    isSbcEncInited = 1;
    }
        Psbc_pcb = sbc_init();
        return  Psbc_pcb;


    return  (void *)0;
}


_ATTR_AUDIO_TEXT_
uint32 sbc_enc(void *hEnc,UINT8 *input,uint32 in_len,UINT8 *output,uint32 * Frame_size)
{
    int i =0;
    uint32 FrameSize;
    uint32 out_len;
    //sbc_enc_init();
    while(i*RAW_FRAME_LENGTH < in_len)
    {
        sbc_encode_frame((sbc_t *)Psbc_pcb,(uint32* )&FrameSize,output,RAW_FRAME_LENGTH,(short *)&input[i*RAW_FRAME_LENGTH]);
        i++;
        output += FrameSize;
    }
    *Frame_size = FrameSize;
    out_len = FrameSize *i;
    return out_len;
}

#endif
