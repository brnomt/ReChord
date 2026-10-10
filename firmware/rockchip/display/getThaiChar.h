
#ifndef __GETTHAICHAR_H__
#define __GETTHAICHAR_H__

#ifdef __cplusplus
extern "C" {
#endif  /*__cplusplus */

#define THAI_FONT_BYTEUNIT

#define THAI_TYPE_12    0
#define THAI_TYPE_16    1

#define THAI_TYPE_12_WIDTH          9
#define THAI_TYPE_12_HEIGHT         12
#define THAI_TYPE_12_DATA_WIDTH     16
#define THAI_TYPE_12_DATA_HEIGHT    12

#define THAI_TYPE_16_WIDTH          9
#define THAI_TYPE_16_HEIGHT         16
#define THAI_TYPE_16_DATA_WIDTH     16
#define THAI_TYPE_16_DATA_HEIGHT    16

/*
 * Error Code
 */
#define THAI_ERR_NONE           0
#define THAI_ERR_INVALID_PARAM  1
#define THAI_ERR_INVALID_GRIF   2


short getThaiCharSize(short nType, short *pnWidth, short *pnHeight);
short getThaiCharImage(short nType, unsigned short* pString, unsigned char *pImage, short *pnWidth, short *pnHeight, short *pnNum);
short getThaiCharNum(short nType, unsigned short* pString, short *pnWidth, short *pnHeight, short *pnNum);

#ifdef __cplusplus
};
#endif  /* __cplusplus */

#endif  /* __GETTHAICHAR_H__ */


