#include <stdio.h>
#ifndef FLACDEC_H_
#define FLACDEC_H_

#include "../hifi_get_bits.h"
#include "codec.h"
#define HIFI_ACC
#define FLAC_STREAMINFO_SIZE   34
#define FLAC_MAX_CHANNELS       2//8
#define FLAC_MIN_BLOCKSIZE     16
#define FLAC_MAX_BLOCKSIZE   4608//65535
#define FLAC_MIN_FRAME_SIZE    11

#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((a) << 24))
#define AVERROR_INVALIDDATA        (-MKTAG( 'I','N','D','A')) ///< Invalid data found when processing input
//int my_bui_clz(unsigned int a);

#define av_log2(x) (31 - my_bui_clz((x)|1))



#define GET_UTF8(val, GET_BYTE, ERROR)\
    val= GET_BYTE;\
    {\
        int ones= 7 - av_log2(val ^ 255);\
        if(ones==1)\
            ERROR\
        val&= 127>>ones;\
        while(--ones > 0){\
            int tmp= GET_BYTE - 128;\
            if(tmp>>6)\
                ERROR\
            val= (val<<6) + tmp;\
        }\
    }




#define FLACCOMMONINFO \
    int samplerate;         /**< sample rate                             */\
    int channels;           /**< number of channels                      */\
    int bps;                /**< bits-per-sample                         */\

/**
 * Data needed from the Streaminfo header for use by the raw FLAC demuxer
 * and/or the FLAC decoder.
 */
#define FLACSTREAMINFO \
    FLACCOMMONINFO \
    int max_blocksize;      /**< maximum block size, in samples          */\
    int min_framesize;      /**< minimum frame size, in bytes    */\
    int max_framesize;      /**< maximum frame size, in bytes            */\
    long long  samples;        /**< total number of samples                 */\

typedef struct FLACStreaminfo {
    FLACSTREAMINFO
    int bitrate;
    long long samples_decoded;
} FLACStreaminfo;

typedef struct FLACFrameInfo {
    FLACCOMMONINFO
    int blocksize;          /**< block size of the frame                 */
    int ch_mode;            /**< channel decorrelation mode              */
    long long  frame_or_sample_num;    /**< frame number or sample number Ö¡ÐòÁÐºÅ  */
    int is_var_size;
} FLACFrameInfo;

typedef struct FLACContext {
    FLACSTREAMINFO

    GetBitContext gb;                       ///< GetBitContext initialized to start at the current frame

    int blocksize;                          ///< number of samples in the current frame
    int curr_bps;                           ///< bps for current subframe, adjusted for channel correlation and wasted bits
    int sample_shift;                       ///< shift required to make output samples 16-bit or 32-bit
    int is32;                               ///< flag to indicate if output should be 32-bit instead of 16-bit
    int ch_mode;                            ///< channel decorrelation type in the current frame
    int got_streaminfo;                     ///< indicates if the STREAMINFO has been read

    int decoded[FLAC_MAX_CHANNELS][FLAC_MAX_BLOCKSIZE];    ///< decoded samples
} FLACContext;


enum {
    FLAC_METADATA_TYPE_STREAMINFO = 0,

};

enum {
    FLAC_CHMODE_INDEPENDENT =  0,
    FLAC_CHMODE_LEFT_SIDE   =  8,
    FLAC_CHMODE_RIGHT_SIDE  =  9,
    FLAC_CHMODE_MID_SIDE    = 10,
};



int flac_decode_frame(void *data, int *data_size,AVPacket *avpkt);
int get_metadata_size( unsigned char *buf, int buf_size);
int parse_streaminfo(FLACContext *s,  unsigned char *buf, int buf_size);
extern int FLAC_decode_init();
extern int FLAC_frame_decode(unsigned char*outbuf,int *out_size);

#endif

