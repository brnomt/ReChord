/************************************************************************
 *       Smart Jpeg Decoder
 *
 * File:
 *  jpgdec.c
 *
 ************************************************************************/

#ifndef JPGDEC_DECOMPRESS_H
#define JPGDEC_DECOMPRESS_H

#include "jpgdec_global.h"
#define HUFF_EXTEND_TBL(x,s) ((x) < extend_test[s] ? (x) + extend_offset[s] : (x))

//------------------------------------------------------------------------------
#define JPGD_INBUFSIZE   4096
//------------------------------------------------------------------------------
// May need to be adjusted if support for other colorspaces/sampling factors is added
#define JPGD_MAXBLOCKSPERMCU 6
//------------------------------------------------------------------------------
#define JPGD_MAXHUFFTABLES  8
#define JPGD_MAXQUANTTABLES  4
#define JPGD_MAXCOMPONENTS  3
#define JPGD_MAXCOMPSINSCAN  4
//------------------------------------------------------------------------------
#define EIGHTEIGHT      8
#define FOURFOUR      4
#define TWOTWO      2
#define ONEONE      1

// Max. allocated blocks
#define JPGD_MAXBLOCKS    100
//------------------------------------------------------------------------------
//#define JPGD_MAX_HEIGHT   5120
//#define JPGD_MAX_WIDTH   5120
//------------------------------------------------------------------------------
/* JPEG specific errors */
#define JPGD_BAD_DHT_COUNTS              -200
#define JPGD_BAD_DHT_INDEX               -201
#define JPGD_BAD_DHT_MARKER              -202
#define JPGD_BAD_DQT_MARKER              -203
#define JPGD_BAD_DQT_TABLE               -204
#define JPGD_BAD_PRECISION               -205
#define JPGD_BAD_HEIGHT                  -206
#define JPGD_BAD_WIDTH                   -207
#define JPGD_TOO_MANY_COMPONENTS         -208
#define JPGD_BAD_SOF_LENGTH              -209
#define JPGD_BAD_VARIABLE_MARKER         -210
#define JPGD_BAD_DRI_LENGTH              -211
#define JPGD_BAD_SOS_LENGTH              -212
#define JPGD_BAD_SOS_COMP_ID             -213
#define JPGD_W_EXTRA_BYTES_BEFORE_MARKER -214
#define JPGD_NO_ARITHMITIC_SUPPORT       -215
#define JPGD_UNEXPECTED_MARKER           -216
#define JPGD_NOT_JPEG                    -217
#define JPGD_UNSUPPORTED_MARKER          -218
#define JPGD_BAD_DQT_LENGTH              -219
#define JPGD_TOO_MANY_BLOCKS             -221
#define JPGD_UNDEFINED_QUANT_TABLE       -222
#define JPGD_UNDEFINED_HUFF_TABLE        -223
#define JPGD_NOT_SINGLE_SCAN             -224
#define JPGD_UNSUPPORTED_COLORSPACE      -225
#define JPGD_UNSUPPORTED_SAMP_FACTORS    -226
#define JPGD_DECODE_ERROR                -227
#define JPGD_BAD_RESTART_MARKER          -228
#define JPGD_ASSERTION_ERROR             -229
#define JPGD_BAD_SOS_SPECTRAL            -230
#define JPGD_BAD_SOS_SUCCESSIVE          -231
#define JPGD_STREAM_READ                 -232
#define JPGD_NOTENOUGHMEM                -233
#define JPG_PROGRESSIVE                  -234
//------------------------------------------------------------------------------

#define JPGD_YH1V1     1
#define JPGD_YH2V1     2
#define JPGD_YH2V2     3
#define JPGD_YH1V2     4
#define JPGD_YH4V1     5
//------------------------------------------------------------------------------
#define JPGD_FAILED  -1
#define JPGD_DONE  1
#define JPGD_OKAY  0
//------------------------------------------------------------------------------
typedef enum
{
    M_SOF0  = 0xC0,
    M_SOF1  = 0xC1,
    M_SOF2  = 0xC2,
    M_SOF3  = 0xC3,

    M_SOF5  = 0xC5,
    M_SOF6  = 0xC6,
    M_SOF7  = 0xC7,

    M_JPG   = 0xC8,
    M_SOF9  = 0xC9,
    M_SOF10 = 0xCA,
    M_SOF11 = 0xCB,

    M_SOF13 = 0xCD,
    M_SOF14 = 0xCE,
    M_SOF15 = 0xCF,

    M_DHT   = 0xC4,

    M_DAC   = 0xCC,

    M_RST0  = 0xD0,
    M_RST1  = 0xD1,
    M_RST2  = 0xD2,
    M_RST3  = 0xD3,
    M_RST4  = 0xD4,
    M_RST5  = 0xD5,
    M_RST6  = 0xD6,
    M_RST7  = 0xD7,

    M_SOI   = 0xD8,
    M_EOI   = 0xD9,
    M_SOS   = 0xDA,
    M_DQT   = 0xDB,
    M_DNL   = 0xDC,
    M_DRI   = 0xDD,
    M_DHP   = 0xDE,
    M_EXP   = 0xDF,

    M_APP0  = 0xE0,
    M_APP15 = 0xEF,

    M_JPG0  = 0xF0,
    M_JPG13 = 0xFD,
    M_COM   = 0xFE,

    M_TEM   = 0x01,

    M_ERROR = 0x100
} JPEG_MARKER;


//------------------------------------------------------------------------------
#define RST0 0xD0
//------------------------------------------------------------------------------

#define QUANT_TYPE int16
#define BLOCK_TYPE int16

typedef struct huff_tables_tag
{
	uint32	look_up[256];
	uint8	code_size[256];
	uint32	tree[512];		// FIXME: Is 512 tree entries really enough to handle _all_ possible
							// code sets? I think so but not 100% positive.
} huff_tables_t, *Phuff_tables_t;

typedef enum
{
    GET_ONE_ROW_OK,
    GET_ONE_ROW_CONTINUE,
    GET_ONE_ROW_FAIL,
}getOneRowRGBState;

#ifndef max
#define max(a,b) (((a)>(b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a)<(b)) ? (a) : (b))
#endif

/******************************************************************************************
  END of Global Veri
*******************************************************************************************/
//int  JpgProDecRead(uchar *Pbuf, int max_bytes_to_read, uint8 *Peof_flag);
//void return(int status);
void JpgDecWordClear(void *p, uint16 c, uint n);
int JpgDecRead(void *Pbuf, int bytes);
void JpgDecPrepInBuffer(void);
int JpgDecReadDhtMarker(void);
int JpgDecReadDqtMarker(void);
int JpgDecReadSofMarker(void);
int JpgDecSkipVariableMarker(void);
int JpgDecReadDriMarker(void);
int JpgDecReadSosMarker(void);
int  JpgDecNextMarker(void);
int  JpgDecProcessMarkers(void);
int JpgDecLocateSoiMarker(void);
int JpgDecLocateSofMarker(void);
int  JpgDecLocateSosMarker(void);
int JpgDecCalcuOutput(void);
void JpgDecInitDecode(void);
void JpgDecFixInBuffer(void);
void JpgDecOutputOneMcu(void);
void JpgDecOutputOneMcu2(void);
int JpgDecodeNext_8x8(void);
int JpgDecodeNext_4x4(void);
int JpgDecodeNext_2x2(void);
int JpgDecodeNext_1x1(void);
void JpgDecMakeHuffTable(int index,  Phuff_tables_t hs);
int JpgDecCheckQuantTables(void);
int JpgDecCheckHuffTables(void);
void JpgDecCalcMcuBlockOrder(void);
int JpgDecInitScan(void);
int JpgDecInitFrame(void);
int JpgDecProcessRestart(void);
int JpgDecInitSequential(void);
int JpgDecodeStart(void);
void JpgDecFindEOI(void);
void JpgDecIdct(int16 *data, int16 *Pdst_ptr);
void JpgDecIdct4x4(int16 *data, int16 *Pdst_ptr);
void JpgDecIdct2x2(int16 *data, int16 *Pdst_ptr);
void JpgDecIdct1x1(int16 *data, int16 *Pdst_ptr);
//void JpgDecYuv2RgbScale2Lcd(void);
//void JpgDecMcuYuv2RgbScale2Lcd(rgbtype* rgbraw);
//void JpgDecMcuYuv2RgbScale2Lcd(rgbtype* rgbraw,int16 yuvlength);
void JpgZoom2YUV420(int16 SrcW, int16 SrcH, int macroRowIndex, uint8* Yptr, uint16* Uptr);
void JpgZoom2RGB888(int16 SrcW, int16 SrcH, int macroRowIndex, uint8* Yptr, uint16* Uptr);
int JpgDecode(IMAGE_DEC_INFO* pdec_info);
int JpgInit(IMAGE_DEC_INFO* pdec_info);
uint JpgDecRol(uint32 i, int j);
//uint JpgDecGetChar1(void);
//uint JpgDecGetChar(uint8 *Ppadding_flag);
uint JpgDecGetChar();
void JpgDecStuffChar(uchar q);
uchar JpgDecGetOctet(void);
uint JpgDecGetBits_1(int num_bits);
void JpgDecSkipBits_1(int numbits);
uint JpgDecGetBits_2(int numbits);
void JpgDecSkipBits_2(int numbits);
int  JpgDecHuffDecode(Phuff_tables_t Ph);


void JpgDecFreeAllBlocks(void);
void* JpgDecAlloc(int n);
void JpgProgressiveDecode(void);
int JpgDecCalcuBranch(void);

typedef struct coeff_buf_tag
{
    uchar *Pdata;

    int block_num_x, block_num_y;
    int block_len_x, block_len_y;

    int block_size;
    int macro_row_size;

} coeff_buf_t, *Pcoeff_buf_t;
#if 1//JPG_PROGRESSIVE

//extern int16 *block_seg[JPGD_MAXBLOCKSPERROW];
extern Pcoeff_buf_t dc_coeffs[JPGD_MAXCOMPONENTS];
extern Pcoeff_buf_t ac_coeffs[JPGD_MAXCOMPONENTS];

extern int block_y_mcu[JPGD_MAXCOMPONENTS];

extern void (*decode_block_func)();

//extern FILE* IDCTInFile;
//extern JPGPRO_OUTPUT *gJpgProOutput;
extern int TranverseStep;
extern void (*JpgZoom)(int16, int16, int, uint8*, uint16*);

int16* coeff_buf_getp(Pcoeff_buf_t cb, int block_x, int block_y);
#endif
#endif


