

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef signed char  int8_t;
typedef signed short int16_t;
typedef signed int   int32_t;
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

//typedef unsigned int size_t;
typedef signed long long   int64_t;
typedef unsigned long long uint64_t;

#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))
#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((a) << 24))
enum CodecID 
{
	CODEC_ID_APE,
	CODEC_ID_ALAC,
	CODEC_ID_FLAC,

};
enum CodecID codec_id;

enum ChannelMode{
    CODEC_CHMODE_INDEPENDENT ,
    CODEC_CHMODE_LEFT_SIDE  ,
    CODEC_CHMODE_RIGHT_SIDE ,
    CODEC_CHMODE_MID_SIDE   ,
};

typedef struct Codec_info
{    
    uint16_t is32;                          //< flag to indicate if output should be 32-bit instead of 16-bit
    uint16_t bps;                           // bits per sample
    uint16_t channels;                      // channels
    uint32_t samplerate;                    // samplerate
    uint32_t average_bitrate;               // bits per second

    uint32_t finalframeblocks;
    uint32_t totalframes;
	
    int max_blocksize;            /**< maximum block size, in samples          */
    long long max_framesize;            /**< maximum frame size, in bytes            */
    long long int    samples;        /**< total number of samples                 */ 

	enum CodecID codec_type;                          //文件类型
	enum ChannelMode ch_mode;                            ///< channel decorrelation type in the current frame
}Codec_info;

#define AV_RB32(x)                                \
	(((unsigned int)((const unsigned char*)(x))[0] << 24) |    \
	(((const unsigned char*)(x))[1] << 16) |    \
	(((const unsigned char*)(x))[2] <<  8) |    \
	((const unsigned char*)(x))[3])
#define AV_RB24(x)                           \
	((((const unsigned char*)(x))[0] << 16) |         \
	(((const unsigned char*)(x))[1] <<  8) |         \
	((const unsigned char*)(x))[2])
#define AV_RL32(x)                                \
	(((unsigned int)((const unsigned char*)(x))[3] << 24) |    \
	(((const unsigned char*)(x))[2] << 16) |    \
	(((const unsigned char*)(x))[1] <<  8) |    \
	((const unsigned char*)(x))[0])



