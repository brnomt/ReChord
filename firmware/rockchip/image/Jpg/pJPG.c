#include "../ImageInclude/image_main.h"
#include "../ImageInclude/image_globals.h"

#ifdef JPG_DEC_INCLUDE
#pragma arm section code = "JpgDecCode", rodata = "JpgDecCode", rwdata = "JpgDecData", zidata = "JpgDecBss"

#include "Driverconfig.h"
#include "pJPG.h"

static int gFirstOffset = 0;
static unsigned char gJpgOutputBuf[1024 * 4];
static IMAGE_PIXEL gScreenBuffer[IMAGE_MAX_OUTPUT_WIDTH*2];
static IMAGE_DEC_INFO gJpgDecInfo;

extern int JpgInit(IMAGE_DEC_INFO* pdec_info);
extern int JpgDecode(IMAGE_DEC_INFO* pdec_info);
extern int JpgDecCalcuOutput(void);
//return 1 when success
unsigned long JPGFunction(unsigned long ulSubFn, unsigned long ulParam1, unsigned long ulParam2,
                          unsigned long ulParam3)
{
    //int  ret_val,width,height;

    switch (ulSubFn)
    {
            int  ret_val;

        case IMAGE_SUBFN_CODEC_DECODE:
            {
                gJpgDecInfo.ptr_output_buf = (unsigned long)gJpgOutputBuf;
                gJpgDecInfo.ScreenBuffer = gScreenBuffer;
                ret_val = JpgDecode(&gJpgDecInfo);
                return ret_val;
            }

            //initialize the decoder
        case IMAGE_SUBFN_CODEC_OPEN_DEC:
            {
                //gJpgDecInfo.fhandle = ulParam3;
                gJpgDecInfo.fhandle = pImageFileHandle;
                gJpgDecInfo.OutputFormat = IMAGE_RGB565;

                gJpgDecInfo.ptr_output_buf = (unsigned long)gJpgOutputBuf;
                gJpgDecInfo.CurrentDecLine = 0;

                if (-1 == (int)gJpgDecInfo.fhandle||0 == gJpgDecInfo.ptr_output_buf)
                    return 0;
                //if(0 == gJpgDecInfo.OutputW||0 == gJpgDecInfo.OutputH)
                //return 0;
                //gJpgDecInfo.OutputW = LCD_WIDTH;
                //gJpgDecInfo.OutputH = LCD_HEIGHT;

                if (0 != JpgInit(&gJpgDecInfo))
                {
                    return 0;
                }

                get_output_wh(&gJpgDecInfo.OutputW,&gJpgDecInfo.OutputH,gJpgDecInfo.ImageW,gJpgDecInfo.ImageH);

                if (gJpgDecInfo.OutputW > IMAGE_MAX_OUTPUT_WIDTH)
                    return 0;
                if (0 != JpgDecCalcuOutput())
                    return 0;
                gJpgDecInfo.dispOffset = (gJpgDecInfo.OutputH - gJpgDecInfo.ValidH)>>1;
                gFirstOffset = gJpgDecInfo.dispOffset;
                return 1;
            }
        case IMAGE_SUBFN_CODEC_GETBUFFER:
            {
                *((unsigned long*)ulParam1) = (unsigned long)gJpgDecInfo.ScreenBuffer;
                *((unsigned long*)ulParam2) = gJpgDecInfo.dispOffset;

                gJpgDecInfo.dispOffset++;
                return 1;
            }
        case IMAGE_SUBFN_CODEC_GETLENGTH:
            {
                *((unsigned long*)ulParam1) = gJpgDecInfo.OutputW;
                return 1;
            }
        case IMAGE_SUBFN_CODEC_GETRESOLUTION:
            {
                *((unsigned long*)ulParam1) = gJpgDecInfo.ImageW;
                *((unsigned long*)ulParam2) = gJpgDecInfo.ImageH;
                return 1;
            }
        case IMAGE_SUBFN_CODEC_GETOUTPUTWH:
            {
                *((unsigned long*)ulParam1) = gJpgDecInfo.ValidW;
                *((unsigned long*)ulParam2) = gJpgDecInfo.ValidH;
                *((unsigned long*)ulParam3) = gFirstOffset;
                return 1;
            }
            // Cleanup after the codec.
        case IMAGE_SUBFN_CODEC_CLOSE:
            {
                return(1);
            }

        default:
            {
                // Return a failure,just in case;
                return(0);
            }
    }
}
#pragma arm section code
#endif