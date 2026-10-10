/************************************************************************
 *      Thumbnail Parse
 *
 * File:
 *      thumbnail_parse.h
 *
 * Author:
 *      CTF
 ************************************************************************/

#ifndef THUMB_PARSE_H
#define THUMB_PARSE_H

enum ThumbnailType
{
    PIC_JPEG,
    PIC_JPG,
    PIC_PEG,
    PIC_BMP,
    PIC_PNG,
};

enum PicFileType
{
    AUDIO_PIC,
    IMAGE_PIC,
    NONE_PIC
};

typedef struct
{
	long FileSize;
	long ImgWidth;
	long ImgLength;	
	long FormatLength;
	long ThumbnailOffset;
	char Thumnailsw;
	char DateTime[20];
} IMAGE_EXIF_INFO;

enum
{
	IMAGE_EXIF_THUMBNAIL_OFF,
	IMAGE_EXIF_THUMBNAIL_ON	
};

void IsDisplayBackground(int DisplayBackground);
void SetPicFileType(int PicFileType);
int ThumbParse(FILE* hFile);

#endif