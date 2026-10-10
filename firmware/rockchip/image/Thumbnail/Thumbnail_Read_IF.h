/* ********************************************************************************************** */
/*                  TEMP機能フロントエンドヘッダ                                                  */
/* ********************************************************************************************** */
/* 	Programmed by			Yuki.Kishimoto                                      */
/* **************************************************************************** */

#ifndef THUMBNAIL_IF_H
#define THUMBNAIL_IF_H "0.01"


typedef struct _pic_data {
//	char *Pic;
	long  PicStart;			/* Picture Start Postion */
	long  PicSize;			/* Picture Size */
} PIC_DATA;


/* ============================================================================================== */
/* 	外部関数のプロトタイプ宣言：他機能でも使用する関数宣言                                        */
/* ============================================================================================== */
void Thumbnail_Read_Main(void);											/* Exif 取得 メイン処理 */


/* ---------------------------------------------------------------------------------------------- */
/* 	外部I/F関数                                                                                   */
/* ---------------------------------------------------------------------------------------------- */
void Thumbnail_IF_SetFileName(char *buf);
void Thumbnail_IF_SetFilePointer(FILE *fp);

PIC_DATA *Thumbnail_IF_GetPictureData(void);
long Thumbnail_IF_GetThumbnailOffset(void);
long Thumbnail_IF_GetThumbnailSize(void);
int Thumbnail_IF_GetPictureType(void);

#endif
