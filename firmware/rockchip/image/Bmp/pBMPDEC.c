/******************************************************
Copyright(C)2007,Rockchip Co., Ltd. All Rights Reserved.
File:  bmpdecode.c
Desc:  BMP image decode
Note:
Author:  evan wu
$Log:
Revision  1.2 2007/01/09 evan wu

******************************************************/

//*********************includes*****************************************************
#include "../ImageInclude/image_main.h"
#include "../ImageInclude/image_globals.h"
#ifdef BMP_DEC_INCLUDE
#pragma arm section code = "BmpDecCode", rodata = "BmpDecCode", rwdata = "BmpDecData", zidata = "BmpDecBss"
#include "Driverconfig.h"
#include "../ImageInclude/image_file_access.h"
#include "pBMPDEC.h"


/*************************************************************************
GLOBAL VARIABLES
**************************************************************************/
static BMP_CALLBACKS_DECODE BMP_callbacks_decode;
static BMP bmp;
//FILE *pImageFile;
//unsigned char* ImageBufAddr;
static IMAGE_PIXEL  gbmpOutbuf[IMAGE_MAX_OUTPUT_WIDTH];
static BMPOUTFACTOR BmpOutFactor;
//static int row_len; 
//static int start_offset;
static int ScreenWidth;//lcd width
static int ScreenHigh;//lcd height
static int gFirstOffset = 0;
/******************************************************
Name: BMPFILE_Read
Desc: read size*cnt data to pBuff from nHandle
Param:pBuff:a pointer to save data
Return:read byte counter
Global:NULL
Note:
Author:evan wu
Log:
******************************************************/
int BMPFILE_Read(void * pBuff,  int size, int cnt, void *nHandle)
{
#ifdef bmp_rk_nano_bord	
    return (IMAGE_RKFIO_FRead(pBuff, size*cnt,(FILE *)nHandle)) ;    
#else
    return (int)(fread(pBuff, size,cnt, nHandle));
#endif
}

/******************************************************
Name: BMP_Seek
Desc: read file
Param: iHandle:file handle, iOffset:file offset iOrigin:file start postion
Return: ture return 0, false return -1
Global:NULL
Note:
Author:evan wu
Log:
******************************************************/
int BMP_Seek(void* iHandle, long iOffset, int iOrigin)
{
    int Ret;
#ifdef bmp_rk_nano_bord	
    Ret = IMAGE_RKFIO_FSeek(iOffset, iOrigin , (FILE *) iHandle);   
#else
    Ret = fseek(iHandle, iOffset, iOrigin);

#endif

    return Ret;
}

/******************************************************
Name: BMPFunction
Desc: 
Param: NULL
Return: 
Global:
Note:
Author: evan wu
Log:
******************************************************/
unsigned long g_bufferheight,g_bufferwidth;
unsigned long BMPFunction(unsigned long ulSubFn, unsigned long ulParam1,
                          unsigned long ulParam2, unsigned long ulParam3)
{

    switch (ulSubFn)
    {
        case IMAGE_SUBFN_CODEC_DECODE:
            {
				INT32S ret;
                //int ScreenWidth = LCD_WIDTH;
                //int ScreenHigh = LCD_HEIGHT;


                ret = BMP_ThumbNail_Decode(BMP_callbacks_decode, &BmpOutFactor, &bmp, ScreenWidth, ScreenHigh);
                //BmpOutFactor.dsty++;
                return ret;
            }
        case IMAGE_SUBFN_CODEC_OPEN_DEC:
            {				
				//ImageBufAddr = (unsigned char*)(&gbmpOutbuf[0]);
                BMP_callbacks_decode.read_func = (unsigned int (*)(void *, unsigned int, unsigned int,  void *))BMPFILE_Read;
                BMP_callbacks_decode.seek_func = (unsigned int (*)(void *, long , int))BMP_Seek;
#ifdef OUT_PUT_RGB888
                BMP_callbacks_decode.write_func = BMP_CustomOutput_32BitRGB888;
#else
                BMP_callbacks_decode.write_func = BMP_CustomOutput_16BitRGB565;
#endif


                BmpOutFactor.BmpFile = pImageFileHandle;
                BmpOutFactor.ImageBufAddr = (unsigned char*)gbmpOutbuf;
                BmpOutFactor.dsty = 0;


                if (!BMP_InitHead(BMP_callbacks_decode,&bmp,&BmpOutFactor,ScreenWidth, ScreenHigh))
                    return FALSE;

                if(bmp.bmih.biWidth > 8000 || abs(bmp.bmih.biHeight) > 8000){   //����ͼƬ��С
    			    return FALSE;
    			}
				
				get_output_wh(&ScreenWidth,&ScreenHigh,bmp.bmih.biWidth,abs(bmp.bmih.biHeight));
				if(ScreenWidth > IMAGE_MAX_OUTPUT_WIDTH)
					return FALSE;				

				if(!BMP_calc_output(BMP_callbacks_decode,&bmp,&BmpOutFactor,ScreenWidth, ScreenHigh))
					return FALSE;

                gFirstOffset = (ScreenHigh - BmpOutFactor.ImageHInBuf)>>1;
                return TRUE;
            }
		case IMAGE_SUBFN_CODEC_GETBUFFER:
			{
				*((unsigned long*)ulParam1) = (unsigned long)BmpOutFactor.ImageBufAddr;
				*((unsigned long*)ulParam2) = (unsigned long)BmpOutFactor.dsty + ((ScreenHigh - BmpOutFactor.ImageHInBuf)>>1);

                BmpOutFactor.dsty++;
				return 1;
            }	
		case IMAGE_SUBFN_CODEC_GETLENGTH:
		{
			*((unsigned long*)ulParam1) = ScreenWidth;
			return 1;
		}
		case IMAGE_SUBFN_CODEC_GETRESOLUTION:
		{
			*((unsigned long*)ulParam1) = bmp.bmih.biWidth;
			*((unsigned long*)ulParam2) = abs(bmp.bmih.biHeight);
			return 1;		    
		}
		case IMAGE_SUBFN_CODEC_GETOUTPUTWH:
		{
            *((unsigned long*)ulParam1) = BmpOutFactor.ImageWInBuf;
			*((unsigned long*)ulParam2) = BmpOutFactor.ImageHInBuf;
			*((unsigned long*)ulParam3) = gFirstOffset;
			return 1;
		}
        case IMAGE_SUBFN_CODEC_ZOOM:
            {
                    return TRUE;
            }
        default:
            return FALSE;
    }
}
#if 0
//----------------test main---------------------
void main()
{
	long cnt_loop = 0;
	FILE* fouthandle = fopen("c:\\rgb888.dat","wb");
	if(NULL == fouthandle)
		return;
		
	pImageFile = fopen("c:\\pic\\128x128.BMP","rb");
	if(NULL == pImageFile)
		return;
	
	
	if(0 == BMPFunction(IMAGE_CODEC_OPEN_DEC,128,128,0,0))
		return;
	
//	fseek(128*((128 - BmpOutFactor.ImageHInBuf)>>1)*2);
	while(TRUE == BMPFunction(IMAGE_CODEC_DECODE,128,128,0,0))
	{
		cnt_loop++;
		fwrite(ImageBufAddr,1,128*sizeof(targbmodel),fouthandle);
	}
}
#endif

#pragma arm section code
#endif