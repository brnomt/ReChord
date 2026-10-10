
#include <stdio.h>

#define HIFI_ACC

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef signed char  int8_t;
typedef signed short int16_t;
typedef signed int   int32_t;
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

typedef unsigned int size_t;
typedef signed long long   int64_t;
typedef unsigned long long uint64_t;

#define int64_t_C(c)     (int64_t)(c)
#define uint64_t_C(c)    (uint64_t)(c)

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))

#define INT_MAX       2147483647    /* maximum (signed) int value */

#define av_log(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#define UINT_MAX  (2800*36)
//#define		av_malloc	rkos_memory_malloc
//#define     av_free     rkos_memory_free


//#define BLOCKS_PER_LOOP     2048  //左\右声道数
#define BLOCKS_PER_LOOP     4096  //左\右声道数

#define MAX_CHANNELS        2
#define MAX_BYTESPERSAMPLE  3
#define APE_FRAMECODE_MONO_SILENCE    1
#define APE_FRAMECODE_STEREO_SILENCE  3
#define APE_FRAMECODE_PSEUDO_STEREO   4
#define HISTORY_SIZE 512
#define PREDICTOR_SIZE 50
#define APE_FILTER_LEVELS 3

#define ENABLE_DEBUG 0

/* The earliest and latest file formats supported by this library */
#define APE_MIN_VERSION 3950
#define APE_MAX_VERSION 4170    //3 3990 chad.ma modified 20160713

#define MAC_FORMAT_FLAG_8_BIT                 1 // is 8-bit [OBSOLETE]
#define MAC_FORMAT_FLAG_CRC                   2 // uses the new CRC32 error detection [OBSOLETE]
#define MAC_FORMAT_FLAG_HAS_PEAK_LEVEL        4 // uint32 nPeakLevel after the header [OBSOLETE]
#define MAC_FORMAT_FLAG_24_BIT                8 // is 24-bit [OBSOLETE]
#define MAC_FORMAT_FLAG_HAS_SEEK_ELEMENTS    16 // has the number of seek elements after the peak level
#define MAC_FORMAT_FLAG_CREATE_WAV_HEADER    32 // create the wave header on decompression (not stored)

#define MAC_SUBFRAME_SIZE 4608

#define APE_EXTRADATA_SIZE 6

typedef struct {
    int64_t pos;
    int nblocks;
    int size;
    int skip;
    int64_t pts;
} APEFrame;

typedef struct {
    /* Derived fields */
    uint32_t junklength;
    uint32_t firstframe;
    uint32_t totalsamples;
    int currentframe;
    APEFrame *frames;
    uint32_t APE_Frm_NUM;
    uint32_t frm_left_sample;
    /* Info from Descriptor Block */
    char magic[4];
    int16_t fileversion;
    int16_t padding1;
    uint32_t descriptorlength;
    uint32_t headerlength;
    uint32_t seektablelength;
    uint32_t wavheaderlength;
    uint32_t audiodatalength;
    uint32_t audiodatalength_high;
    uint32_t wavtaillength;
    uint8_t md5[16];

    /* Info from Header Block */
    uint16_t compressiontype;
    uint16_t formatflags;
    uint32_t blocksperframe;
    uint32_t finalframeblocks;
    uint32_t totalframes;
    uint16_t bps;
    uint16_t channels;
    uint32_t samplerate;

    uint32_t file_size;//文件总大小
    uint32_t total_blocks;//总数据个数
    uint32_t file_time ;//文件时间
    uint32_t bitrate;
    uint32_t TimePos;
    /* Seektable */
    uint32_t *seektable;
} APEContext;

typedef struct APEFilter {
    int16_t *coeffs;        ///< actual coefficients used in filtering
    int16_t *adaptcoeffs;   ///< adaptive filter coefficients used for correcting of actual filter coefficients
    int16_t *historybuffer; ///< filter memory
    int16_t *delay;         ///< filtered values
    int avg;
} APEFilter;
typedef struct APERice {
    uint32_t k;
    uint32_t ksum;
} APERice;
typedef struct APERangecoder {
    uint32_t low;           ///< low end of interval 48
    uint32_t range;         ///< length of interval  52
    uint32_t help;          ///< bytes_to_follow resp. intermediate value 56
    unsigned int buffer;    ///< buffer for input/output 60
} APERangecoder;
typedef struct APEPredictor {
    int32_t *buf;
    int32_t lastA[2];
    int32_t filterA[2];
    int32_t filterB[2];
    int32_t coeffsA[2][4];  ///< adaption coefficients
    int32_t coeffsB[2][5];  ///< adaption coefficients
    int32_t historybuffer[HISTORY_SIZE + PREDICTOR_SIZE];
} APEPredictor;
typedef struct APEContextdec {
    int channels;
    int samples;                             ///< samples left to decode in current frame 0
    int fileversion;                         ///< codec version, very important in decoding process 4
    int compression_level;                   ///< compression levels 8
    int fset;                                ///< which filter set to use (calculated from compression level) 12
    int flags;                               ///< global decoder flags 16
    uint32_t CRC;                            ///< frame CRC 20
    int frameflags;                          ///< frame flags 24
    int currentframeblocks;                  ///< samples (per channel) in current frame 28
    int blocksdecoded;                       ///< count of decoded samples in current frame 32
    APEPredictor predictor;                  ///< predictor used for final reconstruction  2380
    int32_t decoded0[BLOCKS_PER_LOOP];       ///< decoded data for the first channel
    int32_t decoded1[BLOCKS_PER_LOOP];       ///< decoded data for the second channel
    int16_t* filterbuf[APE_FILTER_LEVELS];   ///< filter memory
    APERangecoder rc;                        ///< rangecoder used to decode actual values
    APERice riceX;                           ///< rice code parameters for the second channel
    APERice riceY;                           ///< rice code parameters for the first channel
    APEFilter filters[APE_FILTER_LEVELS][2]; ///< filters used for reconstruction
    uint32_t data_end;                       ///< frame data end
     uint32_t ptr;                      ///< current position in frame data
    int error;
} APEContextdec;
extern int ape_read_header(void);
extern void init_ape(void);
#ifdef APE_TAG
extern void ff_ape_parse_tag(ByteIOContext *pb);
#endif
extern void ape_decode(uint8_t *APE_out_Buffer,u32* OutLength);
extern void APE_free();