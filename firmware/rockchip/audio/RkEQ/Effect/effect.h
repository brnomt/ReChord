/******************************************************************************
*
*  Copyright (C),2007, Fuzhou Rockchip Co.,Ltd.
*
*  File name :     effect.h
*  Description:   audio effect process.
*  Remark:
*
*  History:
*           <author>      <time>     <version>       <desc>
*           Huweiguo     07/09/27      1.0
*
*******************************************************************************/

#ifndef __EFFECT_H__
#define __EFFECT_H__


/* EQ mode */
typedef enum
{
    EQ_HEAVY,
    EQ_POP,    // 1
    EQ_JAZZ,
    EQ_UNIQUE, // 3
    EQ_CLASS,
    EQ_BASS,    //5
    EQ_ROCK,
    EQ_USER,
    EQ_NOR      //8

} eEQMode;

typedef long    EQ_TYPE;
typedef long long   EQ_TYPE_LONG;

/*parameter structure of RK audio effect.*/
typedef __packed struct
{
    short    dbGain[5]; //5 band EQµÄÔöÒæ
} tRKEQCoef;


typedef __packed struct
{
    eEQMode Mode;      // EQ mode
    unsigned short max_DbGain;
   //#ifdef _RK_EQ_
    tRKEQCoef  RKCoef;
   // #endif

    //tPFEQCoef  PFCoef;
} RKEffect;

#define CUSTOMEQ_LEVELNUM        7

/* API interface function. */

long EffectInit(void);       // initialization.
long EffectEnd(void);      //handle over.
long EffectProcess(EQ_TYPE *pBuffer, long PcmLen);   //audio effect process function,call it every frame.
long EffectAdjust(void);      //this function is for adjust audio effect.
long RKEQAdjust(RKEffect *pEft);

#endif

