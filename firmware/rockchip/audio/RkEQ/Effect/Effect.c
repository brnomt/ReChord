

/******************************************************************************
*
*  Copyright (C),2007, Fuzhou Rockchip Co.,Ltd.
*
*  File name :     effect.c
*  Description:    ��ƵЧ������
*  Remark:
*
*  History:
*           <author>      <time>     <version>       <desc>
*           Huweiguo     07/09/27      1.0
*
*******************************************************************************/

#include "audio_globals.h"
#include "SysInclude.h"
#ifdef _RK_EQ_

#include "AudioControl.h"
#include "effect.h"

extern AudioInOut_Type     AudioIOBuf;
extern int CurrentCodec;

#define EQ_NUM  31

/**************************************************************************
* function description: set parameters of RockEQ
* input:  [pEft]   --
* output:
* return:    0:success; other:failure.
* note:
***************************************************************************/
#define EQADDVOL     6

_ATTR_AUDIO_DATA_ short PresetGain[4][EQ_NUM] =   //����12   �����ܺ�   �޸�һ�� ��0Ϊ -17db ���õĺܺõĲ�����
 {
 {12, 12, 12, 12, 12,
 12,  12, 15, 12, 12,   //3//
 12,  12, 12, 12, 12,
 12,  12, 9,  12, 12,
 12,  12, 15, 12, 12,
 12,  12, 12, 12, 12, 12
 },
{12, 12,  12, 12, 12,
 12,  12, 12, 12, 12,
 12,  12, 15, 12, 12,
 12,  12, 15, 12, 12,
 12,  12,  9, 12, 12,
 12,  12, 12, 12, 12, 12
 },
{12, 12,  12, 12, 12,
 12,  12, 18, 12, 12,
 12,  12, 12, 12, 12,
 12,  12, 12, 12, 12,
 12,  12,  9, 12, 12,
 12,  12, 15, 12, 12, 12
 },
 {12, 12, 12, 12, 12,
 12,  12, 18, 12, 12,
 12,  12, 6, 12, 12,
 12,  12, 16, 12, 12,
 12,  12, 13, 12, 12,
 12,  12, 18, 12, 12, 12
 }
/*
     {0, 1, 5, 5, 8,
 11,  14, 15, 14, 13,   //�ص���Ч����ã���Ƶ�ʲ���
 15,  14, 13, 11, 10,
 11,  11, 12, 12, 13,
 13,  13, 13, 13, 13,
 12,  12, 12, 12, 12, 12
 }*/
 }; // BASS BOOST

 _ATTR_AUDIO_DATA_ short EqBassBoostGain[EQ_NUM] =
{    12,  12, 12, 12, 12,
     22,  12, 12, 17, 12,
     12,  12, 12, 12, 12,
     12,  12, 12, 12, 12,
     12,  12, 12, 12, 12,
     12,  12, 12, 12, 12, 12  // BASS BOOST
 };

_ATTR_AUDIO_DATA_  short UseEQ[EQ_NUM] =
{    12,  12, 12, 12, 12,
     12,  12, 12, 12, 12,
     12,  12, 12, 12, 12,
     12,  12, 12, 12, 12,
     12,  12, 12, 12, 12,
     12,  12, 12, 12, 12, 12  // BASS BOOST
 };

/**************************************************************************
* function description: audio effect is end.
* input   :  null
* output  : null
* return  :    0:success; other:failure.
* note:
***************************************************************************/
_ATTR_AUDIO_TEXT_
ReadEqData(char *p , int off ,int size)
{
    ReadModuleData(MODULE_ID_AUDIO_EQ , p , off , size );
}


/**************************************************************************
* function description: audio effect is end.
* input   :  null
* output  : null
* return  :    0:success; other:failure.
* note:
***************************************************************************/
_ATTR_AUDIO_TEXT_
long RKEQAdjust(RKEffect *pEft)
{
    int i;
    unsigned long SamplingRate = 44100;

    CodecGetSampleRate(&SamplingRate);
    switch(SamplingRate)
        {
            case 8000:
            case 11025:
            case 12000 :
                SamplingRate = 11025;
                break;
            case 16000:
            case 22050:
            case 24000:
                SamplingRate = 22050;
                break;
            case 32000:
            case 44100:
            case 48000:
            case 64000:
            case 88200:
            case 96000:
            case 128000:
            case 176400:
            case 192000:

                SamplingRate = 44100;
                break;
       }

    switch (pEft->Mode)
    {
        case EQ_ROCK:
        case EQ_POP:
        case EQ_CLASS:
        case EQ_JAZZ:
        case EQ_HEAVY:
        case EQ_UNIQUE:
            RockEQAdjust(SamplingRate, PresetGain[pEft->Mode], 1);
            break;

        case EQ_USER:
            for (i = 0;i < 5;i++)
            {
                UseEQ[7+i*5] = pEft->RKCoef.dbGain[i] + 12;
            }
            RockEQAdjust(SamplingRate, UseEQ, 1);
            break;

        case EQ_BASS:
            RockEQAdjust(SamplingRate, EqBassBoostGain, 1);
            break;
        default:
            break;
    }
    return 0;
}

/**************************************************************************
* function description: audio effect is processing.
* input   :[pBuffer] -- the buffer put pcm data.
           [PcmLen] --  the length of pcm data.
* output  : null
* return  : 0:success; other:failure.
* note    : the data that audio effect would take care need use data format LRLRLR.
***************************************************************************/
_ATTR_AUDIO_TEXT_
long EffectProcess(EQ_TYPE *pBuffer, long PcmLen)
{
    AudioInOut_Type  *pAudio = &AudioIOBuf;
    RKEffect   *pEffect = &pAudio->EffectCtl;
    int Mode;

    if(pAudioRegKey->IsEQUpdate)
    {
        return 0;
    }

    if (pBuffer == 0)
        return 1;

    if (PcmLen <= 0)
        return 0;

    switch (pEffect->Mode)
    {
        case EQ_NOR:
            break;

        case EQ_ROCK:
        case EQ_POP:
        case EQ_CLASS:
        case EQ_BASS:
        case EQ_JAZZ:
        case EQ_HEAVY:
        case EQ_UNIQUE:
        case EQ_USER:
            if (!(PcmLen & (64-1))) //by Vincent
            {
                RockEQProcess(pBuffer, PcmLen);
            }
            break;

        default:
            break;
    }

    return 0; //success
}

/**************************************************************************
* description   : audio effect adjust.
* input         : null
* output        : null
* return        : 0:success; other:failure.
* note          :
***************************************************************************/
_ATTR_SYS_BSS_ EXT  uint32  AudioLen;
static int EQ_FR_FLAG = 0;
_ATTR_AUDIO_TEXT_
long EffectAdjust(void)
{
    AudioInOut_Type  *pAudio = &AudioIOBuf;
    RKEffect         *pEffect = &pAudio->EffectCtl;
    unsigned long SamplingRate = 44100;
    int i,j=0;
    int Mode;
    int EQ_AudioLen ;
    CodecGetSampleRate(&SamplingRate);
    UserIsrDisable();
    if (pEffect->Mode == EQ_USER)
    {
        for (i=0; i<EQ_NUM; i++)
        {
            if (AudioIOBuf.EffectCtl.RKCoef.dbGain[i] == 0)
            {
                j++;
            }
        }

        if (j == EQ_NUM)
        {
            pEffect->Mode = EQ_NOR;
            IsEQNOR = TRUE;
        }
    }
    switch (pEffect->Mode)
    {
        case EQ_NOR:
        {
            switch(EQ_FR_FLAG)
            {
              case 1:
                FREQ_ExitModule(FREQ_EQ_L);
                EQ_FR_FLAG = 0;
                break;
              case 2:
                FREQ_ExitModule(FREQ_EQ);
                EQ_FR_FLAG = 0;
                break;
              case 3:
                FREQ_ExitModule(FREQ_EQ_H);
                EQ_FR_FLAG = 0;
                break;
              case 0:
                break;
            }
            AudioSetVolume();
        }

            break;

        case EQ_POP:
        case EQ_CLASS:
        case EQ_BASS:
        case EQ_JAZZ:
        case EQ_HEAVY:
        case EQ_UNIQUE:
        case EQ_USER:
            /* TODO : here load code */
            ModuleOverlay(MODULE_ID_AUDIO_RKEQ, MODULE_OVERLAY_ALL);

            EQ_AudioLen =  AudioLen >>1;

            if((EQ_AudioLen  <= 1152)&&( SamplingRate <=48000))
            {
                FREQ_EnterModule(FREQ_EQ_L);
                EQ_FR_FLAG = 1;
            }
            else if(((EQ_AudioLen  < 4096)&&( SamplingRate <=48000))||((EQ_AudioLen  <= 1152 )&&( (SamplingRate >48000)&&(SamplingRate <=88200))))
            {
                FREQ_EnterModule(FREQ_EQ);
                EQ_FR_FLAG = 2;
            }
            else
            {
                FREQ_EnterModule(FREQ_EQ_H);
                EQ_FR_FLAG = 3;
            }
            RKEQAdjust(pEffect);
            AudioSetVolume();
            SendMsg(MSG_AUDIO_EQ_PRO);
            break;

        default:
            break;
    }

    UserIsrEnable();

    pAudioRegKey->IsEQUpdate = 0;

    return 0; //success.
}


/**************************************************************************
* function description: audio effect is initialization.
* input   :  null
* output  : null
* return  :    0:success; other:failure.
* note:
***************************************************************************/
_ATTR_AUDIO_TEXT_
long EffectInit(void)
{
    ModuleOverlay(MODULE_ID_AUDIO_RKEQ, MODULE_OVERLAY_ALL);
    return 0;
}


/**************************************************************************
* function description: audio effect is end.
* input   :  null
* output  : null
* return  :    0:success; other:failure.
* note:
***************************************************************************/
 _ATTR_AUDIO_TEXT_
long EffectEnd(void)
{
    AudioInOut_Type  *pAudio = &AudioIOBuf;
    RKEffect   *pEffect = &pAudio->EffectCtl;

    {
        switch(EQ_FR_FLAG)
        {
          case 1:
            FREQ_ExitModule(FREQ_EQ_L);
            break;
          case 2:
            FREQ_ExitModule(FREQ_EQ);
            break;
          case 3:
            FREQ_ExitModule(FREQ_EQ_H);
            break;
        }
    }
    return 0;
}


_ATTR_AUDIO_TEXT_
void RockEQReduce9dB(EQ_TYPE *pwBuffer, long cwBuffer, long mode)
{
    AudioInOut_Type  *pAudio = &AudioIOBuf;
    RKEffect   *pEffect = &pAudio->EffectCtl;
    switch (pEffect->Mode)
       {
           case EQ_NOR:
           case EQ_ROCK:
           case EQ_CLASS:
                 break;

           case EQ_HEAVY:
           case EQ_POP:
              while(cwBuffer--)
                {
                    *pwBuffer = (EQ_TYPE)(((EQ_TYPE_LONG)(*pwBuffer)*5)>>3);//20log(0.625)=-4
                    pwBuffer += mode;
                }
                 break;
           case EQ_JAZZ:
           case EQ_UNIQUE:
                while(cwBuffer--)
                {
                    *pwBuffer = (EQ_TYPE)(((EQ_TYPE_LONG)(*pwBuffer)*7)>>4);//20log(0.438)=-7.1
                    pwBuffer += mode;
                }
                 break;
           case EQ_USER:
                while(cwBuffer--)
                {
                    *pwBuffer = (EQ_TYPE)(((EQ_TYPE_LONG)(*pwBuffer))>>2);//20log(0.25)=-12
                     pwBuffer += mode;
                }
                 break;
           case EQ_BASS:

                while(cwBuffer--)
                {
                    if(pEffect->max_DbGain == 12)
                    {
                        *pwBuffer = (EQ_TYPE)(((EQ_TYPE_LONG)(*pwBuffer))>>2);
                    } //20log(0.25)=-12  //20log(3/8)=8.5
                    else if(pEffect->max_DbGain == 10)
                    {
                      *pwBuffer = (EQ_TYPE)(((EQ_TYPE_LONG)(*pwBuffer)*5)>>4);
                    }
                    else if(pEffect->max_DbGain == 8)
                    {
                      *pwBuffer = (EQ_TYPE)(((EQ_TYPE_LONG)(*pwBuffer)*3)>>3);
                    }
                    else if(pEffect->max_DbGain == 6)
                    {
                      *pwBuffer = (EQ_TYPE)(((EQ_TYPE_LONG)(*pwBuffer))>>1);
                    }
                    else if(pEffect->max_DbGain == 4)
                    {
                      *pwBuffer = (short)(((long)(*pwBuffer)*5)>>3);
                    }
                    else if(pEffect->max_DbGain == 2)
                    {
                      *pwBuffer = (short)(((long)(*pwBuffer)*3)>>2);
                    }
                    pwBuffer += mode;
                }

                 break;

        }
}

#endif// _RK_EQ_