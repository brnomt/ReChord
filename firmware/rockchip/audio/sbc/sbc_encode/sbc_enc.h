
#ifndef __SBC_ENC_H
#define __SBC_ENC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SysConfig.h"
//#include <stdint.h>
#include "typedef.h"

#ifdef _SBC_ENCODE_



#define _ATTR_AUDIO_SBC_ENCODE_TEXT_     
#define _ATTR_AUDIO_SBC_ENCODE_DATA_     
#define _ATTR_AUDIO_SBC_ENCODE_BSS_      

#define _ATTR_AUDIO_TEXT_     
#define _ATTR_AUDIO_DATA_     
#define _ATTR_AUDIO_BSS_      

/* sampling frequency */
#define SBC_FREQ_16000      0x00
#define SBC_FREQ_32000      0x01
#define SBC_FREQ_44100      0x02
#define SBC_FREQ_48000      0x03

/* blocks */
#define SBC_BLK_4       0x00
#define SBC_BLK_8       0x01
#define SBC_BLK_12      0x02
#define SBC_BLK_16      0x03

/* channel mode */
#define SBC_MODE_MONO       0x00
#define SBC_MODE_DUAL_CHANNEL   0x01
#define SBC_MODE_STEREO     0x02
#define SBC_MODE_JOINT_STEREO   0x03

/* allocation method */
#define SBC_AM_LOUDNESS     0x00
#define SBC_AM_SNR      0x01

/* subbands */
#define SBC_SB_4        0x00
#define SBC_SB_8        0x01

/* Data endianess */
#define SBC_LE          0x00
#define SBC_BE          0x01

struct sbc_struct {
    unsigned long flags;

    uint8 frequency;
    uint8 blocks;
    uint8 subbands;
    uint8 mode;
    uint8 allocation;
    uint8 bitpool;
    uint8 endian;

    void *priv;
};

typedef struct sbc_struct sbc_t;

//void *  sbc_init(uint32 nSampleRate, uint32 nBitRate, uint32 nChannels);
void *  sbc_init();
int sbc_reinit(sbc_t *sbc, unsigned long flags);
int sbc_parse(sbc_t *sbc, void *input, int input_len);
int sbc_decode(sbc_t *sbc, void *input, int input_len, void *output,
        int output_len, int *len);
int sbc_encode_frame(void * hEnc, uint32* pnFrameSize, uint8 * pFrame, uint32 nRawLen, short *pRawData);
int sbc_get_frameLen(sbc_t *sbc);
int sbc_get_frame_duration(sbc_t *sbc);
int sbc_get_encode_size(sbc_t *sbc);
//void sbc_finish(sbc_t *sbc);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __SBC_H */
