#ifndef JPGDEC_GLOBALVARDECLARE_H
#define JPGDEC_GLOBALVARDECLARE_H
#include "jpgdec_decompress.h"
/******************************************************************************************
    BEGIN of Global Veri
*******************************************************************************************/

//*******post process********************************************
//int16  do_scale;                    // need scale befour YUV->RGB
//int16  ScaledX;        // length after scale
//int16  ScaledY;

//int16  hor_blank;
//  int16  ver_blank;
//  int16  number;
//int16  up_para;
//int16  down_para;

//*******post process********************************************
extern  int16    Jpg_gIdctSize;
extern  int16    Jpg_gImageXSize;
extern  int16    Jpg_gImageYSize;
extern  int16    Jpg_gDstXSizeExpand;
extern  int16    Jpg_gDstYSizeExpand;

extern  int16    Jpg_gDstXSize;
extern  int16    Jpg_gDstYSize;
extern  int16    Jpg_gNumCoffs;


extern  uint8  Jpg_pHuffNum[JPGD_MAXHUFFTABLES][17];  /* pointer to number of Huffman codes per bit size */
extern  uint8  Jpg_pHuffVal[JPGD_MAXHUFFTABLES][256];  /* pointer to Huffman codes per bit size */

extern  QUANT_TYPE Jpg_pQuant[JPGD_MAXQUANTTABLES][64];;    /* pointer to quantization tables */

extern  int      Jpg_gScanType;                      /* Grey, Yh1v1, Yh1v2, Yh2v1, Yh2v2,CMYK111, CMYK4114 */
extern  int      Jpg_gComponentsInFrame;                 /* # of components in frame */
extern  int      Jpg_gCompHoriSamp[JPGD_MAXCOMPONENTS];     /* component's horizontal sampling factor */
extern  int      Jpg_gCompVertSamp[JPGD_MAXCOMPONENTS];     /* component's vertical sampling factor */
extern  int      Jpg_gCompQuant[JPGD_MAXCOMPONENTS];      /* component's quantization table selector */
extern  int      Jpg_gCompIdent[JPGD_MAXCOMPONENTS];      /* component's ID */
extern  int      Jpg_gCompHoriBlocks[JPGD_MAXCOMPONENTS];
extern  int      Jpg_gCompVertBlocks[JPGD_MAXCOMPONENTS];
extern  int      Jpg_gCompInScan;                  /* # of components in scan */
extern  int      Jpg_gCompList[JPGD_MAXCOMPSINSCAN];      /* components in this scan */
extern  int      Jpg_gCompDCTable[JPGD_MAXCOMPONENTS];     /* component's DC Huffman coding table selector */
extern  int      Jpg_gCompACTable[JPGD_MAXCOMPONENTS];     /* component's AC Huffman coding table selector */

extern  int      Jpg_gSpectralStart;                 /* spectral selection start */
extern  int      Jpg_gSpectralEnd;                   /* spectral selection end   */
extern  int      Jpg_gSuccessiveLow;                 /* successive approximation low */
extern  int      Jpg_gSuccessiveHigh;                /* successive approximation high */

extern  int      Jpg_gMaxMcuXSize;                 /* MCU's max. X size in pixels */
extern  int      Jpg_gMaxMcuYSize;                 /* MCU's max. Y size in pixels */
extern  int      Jpg_gBlockSize;

extern  int      Jpg_gMaxMcuXSize2;                 /* MCU's max. X size in pixels */
extern  int      Jpg_gMaxMcuYSize2;

extern  int      Jpg_gMaxBlocksPerRow;
extern  int      Jpg_gMaxMcusPerCol;

extern  int      Jpg_gMcusPerRow;
extern  int      Jpg_gMcusPerCol;
extern  int      Jpg_gBlocksPerMcu;

extern  int      Jpg_gMcuOrg[JPGD_MAXBLOCKSPERMCU];

extern unsigned char  gJpg_gHuffman[(256*4 + 256 + 512*4)*JPGD_MAXHUFFTABLES];
extern  Phuff_tables_t Jpg_gHuffman[JPGD_MAXHUFFTABLES];
//  Pcoeff_buf_t dc_coeffs[JPGD_MAXCOMPONENTS];
//  Pcoeff_buf_t ac_coeffs[JPGD_MAXCOMPONENTS];


//extern  int block_y_mcu[JPGD_MAXCOMPONENTS];

extern  uint8*   Jpg_pPinBufOfs;
//extern  int      Jpg_gInBufLeft;
//extern  int      Jpg_gTemFlag;
extern  uint8    Jpg_gEofFlag;
extern  uint8    JpgProInBuf[JPGD_INBUFSIZE + 128 + 32];
extern  uint8*   Jpg_pInBuf;
extern  int      Jpg_gBitsLeft;
extern  uint32   Jpg_gBitBuf;
extern  int      Jpg_gRestartInterval;
extern  int      Jpg_gRestartLeft;
extern  int      Jpg_gNextRestartNum;
extern  int      Jpg_gMaxMcusPerRow;
extern  int      Jpg_gMaxBlocksPerMcu;
extern  int      Jpg_gMcusPerCol;
extern  int32*   Jpg_pComponent[JPGD_MAXBLOCKSPERMCU];   /* points into the lastdcvals table */
extern  int32    Jpg_gLastDCValue[JPGD_MAXCOMPONENTS];
extern  Phuff_tables_t  Jpg_gDCHuffSeg[JPGD_MAXBLOCKSPERMCU];
extern  Phuff_tables_t  Jpg_gACHuffSeg[JPGD_MAXBLOCKSPERMCU];
//extern  void*    Jpg_pBlocks[JPGD_MAXBLOCKS];         /* list of all dynamically allocated blocks */
//extern  int      Jpg_gBlockMaxZagSet[JPGD_MAXBLOCKSPERROW];
extern  int8   Jpg_pPsampleBuf[];
extern  int16    Jpg_gTempBlock[80];
extern  int16    Jpg_gTempMcuBlock[64*6];
//extern  int      Jpg_gTotalBytesRead;
#ifdef Y_WORD
extern  int16*   Jpg_pPSampleY;
#else
extern  int8*   Jpg_pPSampleY;
#endif
extern  int16*   Jpg_pPSampleU;
//extern  int16*   Jpg_pPSampleV;
#if 0
extern  int16*   Jpg_pLcdBufY;
extern  int16*   Jpg_pLcdBufU;
extern  int16*   Jpg_pLcdBufV;
#endif
//extern  rgbtype  Jpg_gRGBOutput[SCR_LENGTH * SCR_HEIGHT];//YUV缩小到屏幕尺寸大小的RGB数据
//extern  int16    Jpg_gZoomInWidth;
//extern  int16    Jpg_gZoomInHeight;
//extern  int16    Jpg_gYUVRawIndex;
#endif
