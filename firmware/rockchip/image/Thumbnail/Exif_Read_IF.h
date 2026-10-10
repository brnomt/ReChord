/* ********************************************************************************************** */
/*                  Exif解析機能フロントエンドヘッダ                                                  */
/* ********************************************************************************************** */
/*  Programmed by           Yuki.Kishimoto                                      */
/* **************************************************************************** */

#ifndef EXIF_IF_H
#define EXIF_IF_H "0.01"



/* ============================================================================================== */
/*  外部関数のプロトタイプ宣言：他機能でも使用する関数宣言                                        */
/* ============================================================================================== */
int Exif_Read_Main(void);                                          /* Exif 取得 メイン処理 */


/* ---------------------------------------------------------------------------------------------- */
/*  外部I/F関数                                                                                   */
/* ---------------------------------------------------------------------------------------------- */
void Exif_IF_SetFileName(char *buf);
void Exif_IF_SetFilePointer(FILE *fp);
int Exif_IF_SeekFilePointer(FILE *fp, long ofs);
long Exif_IF_GetFileSize(IMAGE_EXIF_INFO *exifinfo);
long Exif_IF_GetImgWidth(IMAGE_EXIF_INFO *exifinfo);
long Exif_IF_GetImgLength(IMAGE_EXIF_INFO *exifinfo);
char *Exif_IF_GetDateTime(IMAGE_EXIF_INFO *exifinfo);
long Exif_IF_GetFormatLength(IMAGE_EXIF_INFO *exifinfo);
long Exif_IF_GetThumbnailOffset(IMAGE_EXIF_INFO *exifinfo);
#endif
