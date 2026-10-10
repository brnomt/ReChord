/************************************************************************
 *       Smart Jpeg Decoder
 *
 * File:
 *  jpgdec_init.c
************************************************************************/
#include "image_main.h"
#ifdef JPG_DEC_INCLUDE

#pragma arm section code = "JpgDecCode", rodata = "JpgDecCode", rwdata = "JpgDecData", zidata = "JpgDecBss"

#include "stdio.h"
#include "string.h"
#include "pJPG.h"
#include "jpgdec_decompress.h"
#include "jpgdec_globalvardeclare.h"

//*************************************************************
extern IMAGE_DEC_INFO *gJpgProOutput;
extern int eob_run;
extern int McuOrgReverse[JPGD_MAXBLOCKSPERMCU];
extern int isyuvDataNotReady;
extern int currentMacroRow;
extern uint32 ValidW;
extern uint32 ValidH;

// Reset everything to default/uninitialized state.
void JpgDecInitDecode(void)
{
    isyuvDataNotReady = 1;

    currentMacroRow = 0;
    TranverseStep = 0;

    Jpg_gImageXSize = Jpg_gImageYSize = 0;
    memset(Jpg_pHuffNum, 0, sizeof(Jpg_pHuffNum));

    ImageMemSet(Jpg_pHuffVal, 0, sizeof(Jpg_pHuffVal)/(sizeof(uint32) * 8));
    ImageMemSet(Jpg_pQuant, 0, sizeof(Jpg_pQuant)/(sizeof(uint32) * 8));

    Jpg_gScanType = 0;
    Jpg_gComponentsInFrame = 0;

    memset(Jpg_gCompHoriSamp, 0, sizeof(Jpg_gCompHoriSamp));
    memset(Jpg_gCompVertSamp, 0, sizeof(Jpg_gCompVertSamp));
    memset(Jpg_gCompQuant, 0, sizeof(Jpg_gCompQuant));
    memset(Jpg_gCompIdent, 0, sizeof(Jpg_gCompIdent));
    memset(Jpg_gCompHoriBlocks, 0, sizeof(Jpg_gCompHoriBlocks));
    memset(Jpg_gCompVertBlocks, 0, sizeof(Jpg_gCompVertBlocks));

    Jpg_gCompInScan = 0;
    memset(Jpg_gCompList, 0, sizeof(Jpg_gCompList));
    memset(Jpg_gCompDCTable, 0, sizeof(Jpg_gCompDCTable));
    memset(Jpg_gCompACTable, 0, sizeof(Jpg_gCompACTable));

    Jpg_gSpectralStart = 0;
    Jpg_gSpectralEnd = 0;
    Jpg_gSuccessiveLow = 0;
    Jpg_gSuccessiveHigh = 0;

    Jpg_gMaxMcuXSize = 0;
    Jpg_gMaxMcuYSize = 0;

    Jpg_gBlocksPerMcu = 0;

    Jpg_gMcusPerRow = 0;
    Jpg_gMcusPerCol = 0;

    Jpg_gMaxBlocksPerRow = 0;
    Jpg_gMaxMcusPerCol = 0;

    memset(Jpg_gMcuOrg, 0, sizeof(Jpg_gMcuOrg));

    memset(Jpg_gHuffman, 0, sizeof(Jpg_gHuffman));


    //Jpg_gInBufLeft = 0;
    Jpg_gEofFlag = FALSE;
    //Jpg_gTemFlag = 0;

    ImageMemSet(JpgProInBuf, 0, sizeof(JpgProInBuf)/(sizeof(uint32) * 8));

    Jpg_pInBuf = &JpgProInBuf[32];
    Jpg_pPinBufOfs = Jpg_pInBuf;//Jpg_pInBuf;

    Jpg_gRestartInterval = 0;
    Jpg_gRestartLeft    = 0;
    Jpg_gNextRestartNum = 0;

    Jpg_gMaxMcusPerRow = 0;
    Jpg_gMaxBlocksPerMcu = 0;
    Jpg_gMcusPerCol = 0;

    memset(Jpg_pComponent, 0, sizeof(Jpg_pComponent));
    memset(Jpg_gLastDCValue, 0, sizeof(Jpg_gLastDCValue));
    memset(Jpg_gDCHuffSeg, 0, sizeof(Jpg_gDCHuffSeg));
    memset(Jpg_gACHuffSeg, 0, sizeof(Jpg_gACHuffSeg));

    //Jpg_gTotalBytesRead = 0;

    // Ready the input buffer.
    JpgDecPrepInBuffer();

    // Prime the bit buffer.
    Jpg_gBitsLeft = 16;

    JpgDecSkipBits_1(16);
    JpgDecSkipBits_1(16);
}

// Determines the component order inside each MCU.
// Also calcs how many MCU's are on each row, etc.
void JpgDecCalcMcuBlockOrder(void)
{
    int component_num, component_id;
    int max_h_samp = 0, max_v_samp = 0;

    for (component_id = 0; component_id < Jpg_gComponentsInFrame; component_id++)
    {
        if (Jpg_gCompHoriSamp[component_id] > max_h_samp)
            max_h_samp = Jpg_gCompHoriSamp[component_id];

        if (Jpg_gCompVertSamp[component_id] > max_v_samp)
            max_v_samp = Jpg_gCompVertSamp[component_id];
    }

    for (component_id = 0; component_id < Jpg_gComponentsInFrame; component_id++)
    {
        Jpg_gCompHoriBlocks[component_id] = ((((Jpg_gImageXSize * Jpg_gCompHoriSamp[component_id]) + (max_h_samp - 1)) / max_h_samp) + 7) / 8;
        Jpg_gCompVertBlocks[component_id] = ((((Jpg_gImageYSize * Jpg_gCompVertSamp[component_id]) + (max_v_samp - 1)) / max_v_samp) + 7) / 8;
    }

    if (Jpg_gCompInScan == 1)
    {
        Jpg_gMcusPerRow = Jpg_gCompHoriBlocks[Jpg_gCompList[0]];
        Jpg_gMcusPerCol = Jpg_gCompVertBlocks[Jpg_gCompList[0]];
    }
    else
    {
        Jpg_gMcusPerRow = (((Jpg_gImageXSize + 7) / 8) + (max_h_samp - 1)) / max_h_samp;
        Jpg_gMcusPerCol = (((Jpg_gImageYSize + 7) / 8) + (max_v_samp - 1)) / max_v_samp;
    }

    if (Jpg_gCompInScan == 1)
    {
        Jpg_gMcuOrg[0] = Jpg_gCompList[0];

        Jpg_gBlocksPerMcu = 1;
    }
    else
    {
        int blocksNum;
        int *pMcuOrg;
        Jpg_gBlocksPerMcu = 0;

        for (component_num = 0; component_num < Jpg_gCompInScan; component_num++)
        {
            int num_blocks;

            component_id = Jpg_gCompList[component_num];

            num_blocks = Jpg_gCompHoriSamp[component_id] * Jpg_gCompVertSamp[component_id];

            while (num_blocks--)
                Jpg_gMcuOrg[Jpg_gBlocksPerMcu++] = component_id;
        }
        blocksNum = Jpg_gBlocksPerMcu;
        pMcuOrg = Jpg_gMcuOrg;
        do
        {
            McuOrgReverse[--blocksNum] = *pMcuOrg++;
        }
        while (blocksNum);
    }
}

//------------------------------------------------------------------------------
int JpgDecCalcuOutput(void)
{
    float  zoomrate;
    //if( (Jpg_gImageXSize>= SCR_LENGTH*16) || (Jpg_gImageYSize>= SCR_HEIGHT*16))
    if ((Jpg_gImageXSize > MAX_IMAGE_X_SIZE) || (Jpg_gImageYSize > MAX_IMAGE_Y_SIZE))
    {
        return(JPGD_BAD_HEIGHT);
    }
    else if ((Jpg_gImageXSize >= SCR_LENGTH*8) || (Jpg_gImageYSize >= SCR_HEIGHT*8))
    {
        Jpg_gDstXSize = (Jpg_gImageXSize + 7) / 8 ;//dst_x_size表示缩小之后的图像实际有效像素宽度(不包含填充列),这是jpeg解码后所取的有效数据的行宽,用于YUV转RGB用
        Jpg_gDstYSize = (Jpg_gImageYSize + 7) / 8 ;//dst_y_size表示缩小之后的图像实际有效像素高度(不包含填充行)
        Jpg_gMaxMcuXSize2 = Jpg_gMaxMcuXSize / 8;
        Jpg_gMaxMcuYSize2 = Jpg_gMaxMcuYSize / 8;
        Jpg_gDstXSizeExpand = Jpg_gMaxMcusPerRow * Jpg_gMaxMcuXSize2;//Jpg_gDstXSizeExpand表示缩小之后的图像像素宽度(含填充列,凑足宏块的整数倍),这是jpeg解码的行宽长度
        Jpg_gDstYSizeExpand = Jpg_gMcusPerCol * Jpg_gMaxMcuYSize2;//Jpg_gDstXSizeExpand表示缩小之后的图像像素高度(含填充行,凑足宏块的整数倍)
        Jpg_gBlockSize = 1;
        Jpg_gIdctSize = ONEONE;
        Jpg_gNumCoffs = 1;
    }
    else if ((Jpg_gImageXSize >= SCR_LENGTH*4) || (Jpg_gImageYSize >= SCR_HEIGHT*4))
    {
        Jpg_gDstXSize = (Jpg_gImageXSize + 3) / 4 ;
        Jpg_gDstYSize = (Jpg_gImageYSize + 3) / 4 ;
        Jpg_gMaxMcuXSize2 = Jpg_gMaxMcuXSize / 4;
        Jpg_gMaxMcuYSize2 = Jpg_gMaxMcuYSize / 4;
        Jpg_gDstXSizeExpand = Jpg_gMaxMcusPerRow * Jpg_gMaxMcuXSize2;
        Jpg_gDstYSizeExpand = Jpg_gMcusPerCol * Jpg_gMaxMcuYSize2;
        Jpg_gBlockSize = 4;
        Jpg_gIdctSize = TWOTWO;
        Jpg_gNumCoffs = 5;
    }
    else if ((Jpg_gImageXSize >= SCR_LENGTH*2) || (Jpg_gImageYSize >= SCR_HEIGHT*2))
    {
        Jpg_gDstXSize = (Jpg_gImageXSize + 1) / 2 ;
        Jpg_gDstYSize = (Jpg_gImageYSize + 1) / 2 ;
        Jpg_gMaxMcuXSize2 = Jpg_gMaxMcuXSize / 2;
        Jpg_gMaxMcuYSize2 = Jpg_gMaxMcuYSize / 2;
        Jpg_gDstXSizeExpand = Jpg_gMaxMcusPerRow * Jpg_gMaxMcuXSize2;
        Jpg_gDstYSizeExpand = Jpg_gMcusPerCol * Jpg_gMaxMcuYSize2;
        Jpg_gBlockSize = 16;
        Jpg_gIdctSize = FOURFOUR;
        Jpg_gNumCoffs = 25;
    }
    else
    {
        Jpg_gDstXSize = Jpg_gImageXSize;
        Jpg_gDstYSize = Jpg_gImageYSize;
        Jpg_gMaxMcuXSize2 = Jpg_gMaxMcuXSize;
        Jpg_gMaxMcuYSize2 = Jpg_gMaxMcuYSize;
        Jpg_gDstXSizeExpand = Jpg_gMaxMcusPerRow * Jpg_gMaxMcuXSize2;
        Jpg_gDstYSizeExpand = Jpg_gMcusPerCol * Jpg_gMaxMcuYSize2;
        Jpg_gBlockSize = 64;
        Jpg_gIdctSize = EIGHTEIGHT;
        Jpg_gNumCoffs = 64;
    }
#if 0
    DEBUG("\n");
    switch (Jpg_gIdctSize)
    {
        case ONEONE:
            DEBUG("  Jpg_gIdctSize          = %s ;\n", "ONEONE");
            break;
        case TWOTWO:
            DEBUG("  Jpg_gIdctSize          = %s ;\n", "TWOTWO");
            break;
        case FOURFOUR:
            DEBUG("  Jpg_gIdctSize          = %s ;\n", "FOURFOUR");
            break;
        case EIGHTEIGHT:
            DEBUG("  Jpg_gIdctSize          = %s ;\n", "EIGHTEIGHT");
            break;
        default:
            break;
    }

    DEBUG("  Jpg_gDstXSize          = %d ;\n", Jpg_gDstXSize);
    DEBUG("  Jpg_gDstYSize          = %d ;\n", Jpg_gDstYSize);
    DEBUG("  Jpg_gMaxMcuXSize2      = %d ;\n", Jpg_gMaxMcuXSize2);
    DEBUG("  Jpg_gMaxMcuYSize2      = %d ;\n", Jpg_gMaxMcuYSize2);
    DEBUG("  Jpg_gBlockSize         = %d ;\n", Jpg_gBlockSize);
#endif
    //Jpg_pPsampleBuf = (int16 *)JpgDecAlloc((Jpg_gMaxBlocksPerRow * Jpg_gBlockSize) * sizeof(int16));//动态分配最小运算空间
    //if (Jpg_pPsampleBuf == NULL)
        //return JPGD_NOTENOUGHMEM;

    Jpg_pPSampleY = (int8*)Jpg_pPsampleBuf;//所有的Y分量放在最开始的位置
    Jpg_pPSampleU = (int16*)(Jpg_pPsampleBuf + (Jpg_gMaxBlocksPerMcu - 2) * Jpg_gBlockSize * Jpg_gMaxMcusPerRow);//每个宏块行内UV各有1各block,共2各blocks,所有U分量存储在Y块
#if 0


    DEBUG("\n");
    DEBUG("  Original  Length = %4d  ,  Height = %4d \n", Jpg_gImageXSize, Jpg_gImageYSize);
    DEBUG("  Expanded  Length = %4d  ,  Height = %4d \n", Jpg_gDstXSizeExpand, Jpg_gDstYSizeExpand);
    DEBUG("  Unscaled  Length = %4d  ,  Height = %4d \n\n", Jpg_gDstXSize, Jpg_gDstYSize);
    DEBUG("  Y  Sample :  H%dV%d    \n\n\n\n", Jpg_gCompHoriSamp[0], Jpg_gCompVertSamp[0]);
#endif
//    支持放大功能
//    if ((Jpg_gDstXSize <= SCR_LENGTH) && (Jpg_gDstYSize <= SCR_HEIGHT))
//    {
//        zoomrate = 1.0f;
//    }
//    else
//    {
        zoomrate = (float)min(1.0f * SCR_LENGTH / Jpg_gDstXSize, 1.0f * SCR_HEIGHT / Jpg_gDstYSize);
//    }
    gJpgProOutput->ValidW = (short)(Jpg_gDstXSize * zoomrate);  //缩放后的宽度
    gJpgProOutput->ValidH  = (short)(Jpg_gDstYSize * zoomrate); //缩放后的高度

    ValidW = (Jpg_gDstXSize+1)&(~1);
    ValidH = Jpg_gDstYSize;

    return 0;
}

int JpgDecCalcuBranch(void)
{
    //if( (Jpg_gImageXSize>= SCR_LENGTH*16) || (Jpg_gImageYSize>= SCR_HEIGHT*16))
    if ((Jpg_gImageXSize > MAX_IMAGE_X_SIZE) || (Jpg_gImageYSize > MAX_IMAGE_Y_SIZE))
    {
        return(JPGD_BAD_HEIGHT);
    }
    else if ((Jpg_gImageXSize >= SCR_LENGTH*8) || (Jpg_gImageYSize >= SCR_HEIGHT*8))
    {
        Jpg_gIdctSize = ONEONE;
    }
    else if ((Jpg_gImageXSize >= SCR_LENGTH*4) || (Jpg_gImageYSize >= SCR_HEIGHT*4))
    {
        Jpg_gIdctSize = TWOTWO;
    }
    else if ((Jpg_gImageXSize >= SCR_LENGTH*2) || (Jpg_gImageYSize >= SCR_HEIGHT*2))
    {
        Jpg_gIdctSize = FOURFOUR;
    }
    else
    {
        Jpg_gIdctSize = EIGHTEIGHT;
    }
    return 0;

}

// Refill the input buffer.
// This method will sit in a loop until (A) the buffer is full or (B)
// the stream's read() method reports and end of file condition.
int JpgDecRead(void *Pbuf, int bytes)
{
#ifdef jpg_rk_nano_bord
    return (IMAGE_RKFIO_FRead(Pbuf, 1*bytes,(FILE *)gJpgProOutput->fhandle)) ;
#else
    return (int)(fread(Pbuf,1,bytes,gJpgProOutput->fhandle));
#endif
    //return(fread(Pbuf,1,bytes,gJpgProOutput->fhandle));
}

// Starts a new scan.
int JpgDecInitScan(void)
{
    if (!JpgDecLocateSosMarker())
        return FALSE;

    JpgDecCalcMcuBlockOrder();

    JpgDecCheckHuffTables();

    if (JpgDecCheckQuantTables())
        return FALSE;

    memset(Jpg_gLastDCValue, 0, Jpg_gComponentsInFrame * sizeof(uint));

    eob_run = 0;

    if (Jpg_gRestartInterval)
    {
        Jpg_gRestartLeft = Jpg_gRestartInterval;
        Jpg_gNextRestartNum = 0;
    }

    JpgDecFixInBuffer();
    #if 0
    {
        int a, b, c;
        a = Jpg_gCompDCTable[0];
        b = Jpg_gCompDCTable[1];
        c = Jpg_gCompDCTable[2];

        DC_PhuffTab[0] = Jpg_gHuffman[a];
        DC_PhuffTab[1] = Jpg_gHuffman[b];
        DC_PhuffTab[2] = Jpg_gHuffman[c];

        a = Jpg_gCompACTable[0];
        b = Jpg_gCompACTable[1];
        c = Jpg_gCompACTable[2];

        AC_PhuffTab[0] = Jpg_gHuffman[a];
        AC_PhuffTab[1] = Jpg_gHuffman[b];
        AC_PhuffTab[2] = Jpg_gHuffman[c];
    }
    #endif
    return TRUE;
}

//------------------------------------------------------------------------------
int JpgDecInitSequential(void)
{
    if (!JpgDecInitScan())
    {
        return(JPGD_UNEXPECTED_MARKER);
    }
    return 0;
}

//------------------------------------------------------------------------------
// Starts a frame. Determines if the number of components or sampling factors
// are supported.
int JpgDecInitFrame(void)
{
    int i;
    uchar *q;

    if ((Jpg_gComponentsInFrame == 3) || (Jpg_gComponentsInFrame == 1))
    {
        if ((((Jpg_gCompHoriSamp[1] != 1) || (Jpg_gCompVertSamp[1] != 1)) ||
                ((Jpg_gCompHoriSamp[2] != 1) || (Jpg_gCompVertSamp[2] != 1))) && (Jpg_gComponentsInFrame == 3))
        {
            return(JPGD_UNSUPPORTED_SAMP_FACTORS);
        }

        if (Jpg_gComponentsInFrame == 1)//修正只有一个颜色分量并且不是h1v1的图片解码错误
        {
            Jpg_gCompHoriSamp[0] = Jpg_gCompVertSamp[0] = 1;
        }

        //为不同的YUV格式配置宏块的尺寸,每个宏块有多少块
        if ((Jpg_gCompHoriSamp[0] == 1) && (Jpg_gCompVertSamp[0] == 1))//4:4:4
        {
            Jpg_gScanType          = JPGD_YH1V1;

            Jpg_gMaxBlocksPerMcu = 3;//每个宏块含有多少块, YUV各一块

            Jpg_gMaxMcuXSize     = 8;//宏块的宽度
            Jpg_gMaxMcuYSize     = 8;//宏块的高度
        }
        else if ((Jpg_gCompHoriSamp[0] == 2) && (Jpg_gCompVertSamp[0] == 1))//4:2:2
        {
            Jpg_gScanType          = JPGD_YH2V1;

            Jpg_gMaxBlocksPerMcu = 4;//每个宏块含有多少块，Y占二块，UV各一块

            Jpg_gMaxMcuXSize     = 16;//宏块的宽度
            Jpg_gMaxMcuYSize     = 8;//宏块的高度
        }
        else if ((Jpg_gCompHoriSamp[0] == 2) && (Jpg_gCompVertSamp[0] == 2))//4:2:0和4:1:1
        {
            Jpg_gScanType          = JPGD_YH2V2;

            Jpg_gMaxBlocksPerMcu = 6;//每个宏块含有多少块, Y占四块, UV各一块

            Jpg_gMaxMcuXSize     = 16;//宏块的宽度
            Jpg_gMaxMcuYSize     = 16;//宏块的高度
        }
        else if ((Jpg_gCompHoriSamp[0] == 1) && (Jpg_gCompVertSamp[0] == 2))//4:4:0
        {
            //DEBUG("UNSUPPORTED_SAMP_FACTORS\n");
            //exit(1);
            Jpg_gScanType          = JPGD_YH1V2;

            Jpg_gMaxBlocksPerMcu = 4;//每个宏块含有多少块

            Jpg_gMaxMcuXSize     = 8;//宏块的宽度
            Jpg_gMaxMcuYSize     = 16;//宏块的高度

        }
        else if ((Jpg_gCompHoriSamp[0] == 4) && (Jpg_gCompVertSamp[0] == 1)) // 4:1:1
        {
            Jpg_gScanType          = JPGD_YH4V1;

            Jpg_gMaxBlocksPerMcu = 6;//每个宏块含有多少块, Y占四块, UV各一块

            Jpg_gMaxMcuXSize     = 32;//宏块的宽度
            Jpg_gMaxMcuYSize     = 8;//宏块的高度
        }
        else
        {
            return(JPGD_UNSUPPORTED_SAMP_FACTORS);
        }
    }
    else
    {
        return(JPGD_UNSUPPORTED_COLORSPACE);

    }

#if 0
    switch (Jpg_gScanType)
    {
        case JPGD_YH1V1:
            DEBUG("  Jpg_gScanType          = %s ;\n", "4:4:4");
            break;
        case JPGD_YH2V1:
            DEBUG("  Jpg_gScanType          = %s ;\n", "4:2:2");
            break;
        case JPGD_YH2V2:
            DEBUG("  Jpg_gScanType          = %s ;\n", "4:2:0");
            break;
        case JPGD_YH1V2:
            DEBUG("  Jpg_gScanType          = %s ;\n", "4:4:0");
            break;
        default:
            break;
    }

    DEBUG("  Jpg_gMaxBlocksPerMcu   = %d ;\n", Jpg_gMaxBlocksPerMcu);
    DEBUG("  Jpg_gMaxMcuXSize       = %d ;\n", Jpg_gMaxMcuXSize);
    DEBUG("  Jpg_gMaxMcuYSize       = %d ;\n", Jpg_gMaxMcuYSize);

#endif
    Jpg_gMaxMcusPerRow = (Jpg_gImageXSize + (Jpg_gMaxMcuXSize - 1)) / Jpg_gMaxMcuXSize;//jpeg图片按行可分割为多少宏块
    Jpg_gMaxMcusPerCol = (Jpg_gImageYSize + (Jpg_gMaxMcuYSize - 1)) / Jpg_gMaxMcuYSize;//jpeg图片按列可分割为多少宏块

    Jpg_gMaxBlocksPerRow = Jpg_gMaxMcusPerRow * Jpg_gMaxBlocksPerMcu;//计算jpeg图片按行可分割为多少个8×8块

    // Should never happen
    if (Jpg_gMaxBlocksPerRow > JPGD_MAXBLOCKSPERROW)
    {
        return(JPGD_ASSERTION_ERROR);
    }

#if 0//def JPG_PROGRESSIVE
    // Allocate the coefficient buffer, enough for one row's worth of MCU's
    q = (uchar *)JpgDecAlloc(Jpg_gMaxBlocksPerRow * 64 * sizeof(int16) + 8);
    if (q == NULL)
        return JPGD_NOTENOUGHMEM;

    // Align to 8-byte boundry, for MMX code
    q = (uchar *)(((uint)q + 7) & ~7);

    // The block_seg[] array's name dates back to the
    // 16-bit assembler implementation. "seg" stood for "segment".
    //for (i = 0; i < Jpg_gMaxBlocksPerRow; i++)
    //block_seg[i] = (int16 *)(q + i * 64 * sizeof(int16));
#endif

    //for (i = 0; i < Jpg_gMaxBlocksPerRow; i++)
        //Jpg_gBlockMaxZagSet[i] = 64;

    return 0;
}


//------------------------------------------------------------------------------
int JpgDecodeStart(void)
{
    int c;
    c = JpgDecInitFrame();
    if (c)
        return c;

    c = JpgDecCalcuBranch();
    if (c)
        return c;
    //if (progressive_flag)
        //return(JpgDecInitProgressive());
    //else
        return(JpgDecInitSequential());
}

int JpgInit(IMAGE_DEC_INFO* pdec_info)
{
  int ret = 0;
  gJpgProOutput = pdec_info;
  JpgDecInitDecode();
  if(JpgDecLocateSofMarker())
    ret = -1;
  if(JpgDecodeStart())
    ret = -1;
  //if(JpgDecCalcuOutput())
  //    ret = -1;

  gJpgProOutput->ImageW = Jpg_gImageXSize;
  gJpgProOutput->ImageH = Jpg_gImageYSize;

  if(ret)
    JpgDecFreeAllBlocks();

  return ret;
}

#pragma arm section code
#endif
