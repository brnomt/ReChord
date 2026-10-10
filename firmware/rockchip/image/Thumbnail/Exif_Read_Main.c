/* ************************************************ */
/*  Exif_Read_Main.c                                */
/*  2015/4/27                                       */
/*  by Yuki.KIshimoto                               */
/* ************************************************ */
#include "../ImageInclude/image_main.h"
#include "../ImageInclude/image_globals.h"

#ifdef THUMB_DEC_INCLUDE

#pragma arm section code = "ImageContrlCode", rodata = "ImageContrlCode", rwdata = "ImageContrlData", zidata = "ImageContrlBss"

/* ============================================================================================== */
/*  �w�b�_�t�@�C���̃C���N���[�h                                                                  */
/* ============================================================================================== */

#include <stdio.h>
#include "thumbnail_parse.h"
#include "Exif_Read_IF.h"
#include "Exif_Read_Proc.h"

/* ============================================================================================== */
/*  �����\���̒�`�F���̃��W���[���������Ŏg�p����\���̂̒�`                                    */
/* ============================================================================================== */
/*  [��`�Ȃ�] */

/* ============================================================================================== */
/*  �����}�N����`�F���̃��W���[�������Ŏg�p����}�N����`                                        */
/* ============================================================================================== */


/* ============================================================================================== */
/*  �O���֐�                                                                                      */
/* ============================================================================================== */
int Exif_Read_Main(void)                               /* Exif �擾 ���C������ */
{
	return Exif_Read_Proc();
}

/* ---------------------------------------------------------------------------------------------- */
/*  �O��I/F�֐�                                                                                   */
/* ---------------------------------------------------------------------------------------------- */

void Exif_IF_SetFileName(char *buf)
{
	Exif_Proc_SetFileName(&buf);
}

void Exif_IF_SetFilePointer(FILE *fp)
{
	Exif_Proc_SetFilePointer(fp);
}

long Exif_IF_GetFileSize(IMAGE_EXIF_INFO *exifinfo)
{
	long ret_filesize;
	
	ret_filesize = Exif_Proc_GetFileSize();
	exifinfo->FileSize = ret_filesize;
	
	return ret_filesize;
}

long Exif_IF_GetImgWidth(IMAGE_EXIF_INFO *exifinfo)
{
	long ret_imgwidth;
	
	ret_imgwidth = Exif_Proc_GetImgWidth();
	exifinfo->ImgWidth = ret_imgwidth;
	
	return ret_imgwidth;
}

long Exif_IF_GetImgLength(IMAGE_EXIF_INFO *exifinfo)
{
	long ret_imglength;
	
	ret_imglength = Exif_Proc_GetImgLength();
	exifinfo->ImgLength = ret_imglength;
	
	return ret_imglength;
}

char *Exif_IF_GetDateTime(IMAGE_EXIF_INFO *exifinfo)
{
	unsigned char *ret_datetime;
	
	ret_datetime = Exif_Proc_GetDateTime();
	memcpy(&exifinfo->DateTime, ret_datetime, sizeof(exifinfo->DateTime));

	return ret_datetime;
}

long Exif_IF_GetFormatLength(IMAGE_EXIF_INFO *exifinfo)
{
	long ret_formatlength;
	
	ret_formatlength = Exif_Proc_GetFormatLength();
	exifinfo->FormatLength = ret_formatlength;
	
	return ret_formatlength;
}

long Exif_IF_GetThumbnailOffset(IMAGE_EXIF_INFO *exifinfo)
{
	long ret_thumbofs;

	ret_thumbofs = Exif_Proc_GetThumbnailOffset();
	exifinfo->ThumbnailOffset = ret_thumbofs;
	
	return ret_thumbofs;
}

int Exif_IF_SeekFilePointer(FILE *fp, long ofs)
{
	int res = 0;
	if (ofs < 0) {
		res = -1;
	} else {
		res = Exif_Proc_SeekFilePointer(fp, ofs);
	}
	
	return res;
}


#endif /* JPG_DEC_INCLUDE */

