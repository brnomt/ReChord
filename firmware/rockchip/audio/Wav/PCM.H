/***************************************************************************/
#ifndef __PCM_H__
#define __PCM_H__


/* WAVE form wFormatTag IDs */
#define  WAVE_FORMAT_UNKNOWN    0x0000  /*  Microsoft Corporation  */
#define  WAVE_FORMAT_PCM      0x0001  /*  Microsoft Corporation  */
#define  WAVE_FORMAT_ADPCM     0x0002  /*  Microsoft Corporation  */
#define  WAVE_FORMAT_IEEE_FLOAT    0x0003  /*  Microsoft Corporation  */
/*  IEEE754: range (+1, -1]  */
/*  32-bit/64-bit format as defined by */
/*  MSVC++ float/double type */
#define  WAVE_FORMAT_IBM_CVSD    0x0005  /*  IBM Corporation  */
#define  WAVE_FORMAT_ALAW      0x0006  /*  Microsoft Corporation  */
#define  WAVE_FORMAT_MULAW     0x0007  /*  Microsoft Corporation  */
#define  WAVE_FORMAT_OKI_ADPCM    0x0010  /*  OKI  */
#define  WAVE_FORMAT_DVI_ADPCM    0x0011  /*  Intel Corporation  */
#define  WAVE_FORMAT_IMA_ADPCM  (WAVE_FORMAT_DVI_ADPCM) /*  Intel Corporation  */
#define  WAVE_FORMAT_MEDIASPACE_ADPCM  0x0012  /*  Videologic  */
#define  WAVE_FORMAT_SIERRA_ADPCM   0x0013  /*  Sierra Semiconductor Corp  */
#define  WAVE_FORMAT_G723_ADPCM    0x0014  /*  Antex Electronics Corporation  */
#define  WAVE_FORMAT_DIGISTD     0x0015  /*  DSP Solutions, Inc.  */
#define  WAVE_FORMAT_DIGIFIX     0x0016  /*  DSP Solutions, Inc.  */
#define  WAVE_FORMAT_DIALOGIC_OKI_ADPCM  0x0017  /*  Dialogic Corporation  */
#define  WAVE_FORMAT_MEDIAVISION_ADPCM  0x0018  /*  Media Vision, Inc. */
#define  WAVE_FORMAT_YAMAHA_ADPCM   0x0020  /*  Yamaha Corporation of America  */
#define  WAVE_FORMAT_SONARC     0x0021  /*  Speech Compression  */
#define  WAVE_FORMAT_DSPGROUP_TRUESPEECH 0x0022  /*  DSP Group, Inc  */
#define  WAVE_FORMAT_ECHOSC1     0x0023  /*  Echo Speech Corporation  */
#define  WAVE_FORMAT_AUDIOFILE_AF36   0x0024  /*    */
#define  WAVE_FORMAT_APTX      0x0025  /*  Audio Processing Technology  */
#define  WAVE_FORMAT_AUDIOFILE_AF10   0x0026  /*    */
#define  WAVE_FORMAT_DOLBY_AC2    0x0030  /*  Dolby Laboratories  */
#define  WAVE_FORMAT_GSM610     0x0031  /*  Microsoft Corporation  */
#define  WAVE_FORMAT_MSNAUDIO    0x0032  /*  Microsoft Corporation  */
#define  WAVE_FORMAT_ANTEX_ADPCME   0x0033  /*  Antex Electronics Corporation  */
#define  WAVE_FORMAT_CONTROL_RES_VQLPC  0x0034  /*  Control Resources Limited  */
#define  WAVE_FORMAT_DIGIREAL     0x0035  /*  DSP Solutions, Inc.  */
#define  WAVE_FORMAT_DIGIADPCM    0x0036  /*  DSP Solutions, Inc.  */
#define  WAVE_FORMAT_CONTROL_RES_CR10  0x0037  /*  Control Resources Limited  */
#define  WAVE_FORMAT_NMS_VBXADPCM   0x0038  /*  Natural MicroSystems  */
#define  WAVE_FORMAT_CS_IMAADPCM   0x0039 /* Crystal Semiconductor IMA ADPCM */
#define  WAVE_FORMAT_ECHOSC3     0x003A /* Echo Speech Corporation */
#define  WAVE_FORMAT_ROCKWELL_ADPCM  0x003B  /* Rockwell International */
#define  WAVE_FORMAT_ROCKWELL_DIGITALK  0x003C  /* Rockwell International */
#define  WAVE_FORMAT_XEBEC     0x003D  /* Xebec Multimedia Solutions Limited */
#define  WAVE_FORMAT_G721_ADPCM    0x0040  /*  Antex Electronics Corporation  */
#define  WAVE_FORMAT_G728_CELP    0x0041  /*  Antex Electronics Corporation  */
#define  WAVE_FORMAT_MPEG      0x0050  /*  Microsoft Corporation  */
#define  WAVE_FORMAT_MPEGLAYER3    0x0055  /*  ISO/MPEG Layer3 Format Tag */
#define  WAVE_FORMAT_CIRRUS     0x0060  /*  Cirrus Logic  */
#define  WAVE_FORMAT_ESPCM     0x0061  /*  ESS Technology  */
#define  WAVE_FORMAT_VOXWARE     0x0062  /*  Voxware Inc  */
#define  WAVE_FORMAT_CANOPUS_ATRAC   0x0063  /*  Canopus, co., Ltd.  */
#define  WAVE_FORMAT_G726_ADPCM    0x0064  /*  APICOM  */
#define  WAVE_FORMAT_G722_ADPCM    0x0065  /*  APICOM      */
#define  WAVE_FORMAT_DSAT      0x0066  /*  Microsoft Corporation  */
#define  WAVE_FORMAT_DSAT_DISPLAY   0x0067  /*  Microsoft Corporation  */
#define  WAVE_FORMAT_SOFTSOUND    0x0080  /*  Softsound, Ltd.      */
#define  WAVE_FORMAT_RHETOREX_ADPCM  0x0100  /*  Rhetorex Inc  */
#define  WAVE_FORMAT_CREATIVE_ADPCM   0x0200  /*  Creative Labs, Inc  */
#define  WAVE_FORMAT_CREATIVE_FASTSPEECH8   0x0202  /*  Creative Labs, Inc  */
#define  WAVE_FORMAT_CREATIVE_FASTSPEECH10  0x0203  /*  Creative Labs, Inc  */
#define  WAVE_FORMAT_QUARTERDECK   0x0220 /*  Quarterdeck Corporation  */
#define  WAVE_FORMAT_FM_TOWNS_SND   0x0300  /*  Fujitsu Corp.  */
#define  WAVE_FORMAT_BTV_DIGITAL    0x0400  /*  Brooktree Corporation  */
#define  WAVE_FORMAT_OLIGSM     0x1000  /*  Ing C. Olivetti & C., S.p.A.  */
#define  WAVE_FORMAT_OLIADPCM    0x1001  /*  Ing C. Olivetti & C., S.p.A.  */
#define  WAVE_FORMAT_OLICELP     0x1002  /*  Ing C. Olivetti & C., S.p.A.  */
#define  WAVE_FORMAT_OLISBC     0x1003  /*  Ing C. Olivetti & C., S.p.A.  */
#define  WAVE_FORMAT_OLIOPR     0x1004  /*  Ing C. Olivetti & C., S.p.A.  */
#define  WAVE_FORMAT_LH_CODEC    0x1100  /*  Lernout & Hauspie  */
#define  WAVE_FORMAT_NORRIS     0x1400  /*  Norris Communications, Inc.  */


//
//****************************************************************************
//
//  constants used by the Microsoft 4 Bit ADPCM algorithm
//
//****************************************************************************
#define MSADPCM_CSCALE                      8
#define MSADPCM_PSCALE                      8
#define MSADPCM_CSCALE_NUM                  (1 << MSADPCM_CSCALE)
#define MSADPCM_PSCALE_NUM                  (1 << MSADPCM_PSCALE)
#define MSADPCM_DELTA4_MIN                  16
#define MSADPCM_OUTPUT4_MAX                 7
#define MSADPCM_OUTPUT4_MIN                 -8


//
#define  PCM_NOT_SUPPORT    0
#define  PCM_SUPPORT_FORMAT   1


#define  IMA_DECODE_SUCESS   1
#define  IMA_DECODE_FAILURE   0


#define  IMA_3BITMODE    3
#define  IMA_4BITMODE    4


typedef struct tWAVFOURCCID
{
    char Data1;
    char Data2;
    char Data3;
    char Data4;

} WAVFOURCCID;




//****************************************************************************
//
// This structure contains the coefficients used by the MS ADPCM algorithm as
// stored in the wave format structure in the "fmt " chunk of the WAVE file.
//
//****************************************************************************
typedef struct
{
    short iCoef1;
    short iCoef2;
} MSADPCMCoefSet;


//****************************************************************************
//
// This structure contains the definition of the wave format structure as
// stored in the "fmt " chunk of the WAVE file.
//
//****************************************************************************
typedef struct IMAADPCMWAVEFORMAT_tag
{
    unsigned short wFormatTag;
    unsigned short nChannels;
    unsigned long nSamplesPerSec;
    unsigned long nAvgBytesPerSec;
    unsigned short nBlockAlign;
    unsigned short wBitsPerSample;
    unsigned short cbSize;
    unsigned short wSamplesPerBlock;
    unsigned short wNumCoef;
    MSADPCMCoefSet aCoef[7];

} PCMWAVEFORMAT;




typedef struct EXTENSIBLE_WAVEFORMAT_tag//ff fe 标志
{
    unsigned short wFormatTag;
    unsigned short nChannels;
    unsigned long nSamplesPerSec;
    unsigned long nAvgBytesPerSec;
    unsigned short nBlockAlign;  //字节对齐
    unsigned short wBitsPerSample;
    unsigned short cbSize;    
    union
    {
        unsigned short wValidBitsPerSample; /* bits of precision 应小于等于wBitsPerSample*/
        unsigned short wSamplesPerBlock;    /* valid if wBitsPerSample==0 */
        unsigned short wReserved;           /* If neither applies, set to zero */
    } Samples;
    unsigned long    dwChannelMask;        /* which channels are present in stream */
    unsigned char  SubFormat[16];      //GUID
} WAVEFORMATEXTENSIBLE;

//****************************************************************************
//
// This structure contains the information for  the IMA ADPCM encoder/decoder.
//
//****************************************************************************
typedef struct
{
    //
    // The number of channels of audio in the file.
    //
    unsigned long ucChannels;


    // The number of samples in each encoded block of audio.
    //
    unsigned short usSamplesPerBlock;

    unsigned short uBitsPerSample;


} tPCMheader;


//****************************************************************************
//
// API for IMA ADPCM
//
//****************************************************************************
extern unsigned long
    DecodeIMAADPCM(tPCMheader *pPCM, unsigned char *pucSrc, short *psLeft,
                   short *psRight, long ulSrcLen);

extern void
    EncodeIMAADPCM(tPCMheader *pPCM, short *psLeft, short *psRight,
                   unsigned char *pucDst);

//****************************************************************************
//
// API for MS ADPCM
//
//****************************************************************************
extern unsigned long
    DecodeMSADPCM(tPCMheader *pPCM, unsigned char *pucSrc, short *psLeft,
                  short *psRight, unsigned long ulSrcLen);

extern void
    EncodeMSADPCM(tPCMheader *pPCM, short *psLeft, short *psRight,
                  unsigned char *pucDst);



//****************************************************************************
//
// Coefficients for IMA ADPCM
//
//****************************************************************************
extern const int step_table[];
extern const int IndexTab_4bit[];
extern const int IndexTab_3bit[];


//****************************************************************************
//
// Coefficients for MS ADPCM
//
//****************************************************************************
extern const short psCoefficient1[];
extern const short psCoefficient2[];
extern const short psP4[];



typedef unsigned long Fptr_ADPCM0(tPCMheader *, unsigned char *, short *,  short *, long);
typedef void Fptr_ADPCM1(tPCMheader *, short *, short *,      unsigned char *);
typedef unsigned long Fptr_ADPCM2(tPCMheader *, unsigned char *, short *,  short *, unsigned long);
typedef void Fptr_ADPCM3(tPCMheader *, short *, short *,      unsigned char *);



#define MSADPCM_MAX_ENCBUF_LENGTH       4096
#define MSADPCM_MAX_PCM_LENGTH          2048
#define IMAADPCM_MAX_PCM_LENGTH         4096

#define WAVE_FORCC_INFO_SIZE   4
#define WAVE_CHUNK_SIZE_BYTE   4
#define WAVE_CHUNK_FORCC_SIZE_BYTE  8
#define WAVE_FIRST_CHUNK_OFFSET   12

#define WAV_MAX_PCM_LENGTH  1024
#define WAV_IMAMAX_PCM_LENGTH 2730



//****************************************************************************
//
// A structure which defines the perisitent state of the MS ADPCM encoder/
// decoder.
//
//****************************************************************************
typedef struct
{
    //
    // The ADPCM information.
    //
    tPCMheader sPCMHeader;

    //
    // The buffer containing MS ADPCM data.
    //
    unsigned char pucEncodedData[MSADPCM_MAX_ENCBUF_LENGTH+512];

    //short FrameOutputLeft[WAV_IMAMAX_PCM_LENGTH*2];
    //short FrameOutputRight[WAV_IMAMAX_PCM_LENGTH*2];
    short	*OutputBuff[2];//
	//short	OutputBuff[2][WAV_IMAMAX_PCM_LENGTH*2];//for 1.5倍SRC WAV_IMAMAX_PCM_LENGTH*3+26
	int		CurDecodeIdx;

    //
    // The buffers containing PCM data.
    //
    //short psLeft[WAV_IMAMAX_PCM_LENGTH * 2];
    //short psRight[WAV_IMAMAX_PCM_LENGTH * 2];
    
//    BufferState *pOutput;
    //
    // The current offset into the encoded data buffer.
    //
    unsigned long usOffset;

    //
    // The number of valid bytes in the encoded data buffer.
    //
    unsigned long usValid;

    //
    // The offset into the file of the first byte of encoded data.
    //
    unsigned long ulDataStart;

    //
    // The byte-length of the encoded data.
    //
    unsigned long ulLength;

    //
    // The number of bytes of encoded data remaining in the file.
    //
    unsigned long ulDataValid;

    //
    // The byte rate of the encoded file.
    //
    unsigned long usByteRate;

    //
    // The sample rate of the decoded file.
    //
    unsigned long usSampleRate;

    //
    // The number of channels of audio in the file.
    //
    unsigned char ucChannels;


    unsigned char temp_char;

    //
    // The number of bytes in each encoded block of audio.
    //
    unsigned short usBytesPerBlock;  // block

    //
    // The number of samples in each encoded block of audio.
    //
    unsigned short usSamplesPerBlock; // block



    unsigned short uBitsPerSample;
    //
    //Audio Format
    //
    unsigned short wFormatTag;
    //
    // The length of the file in milliseconds.
    //
    unsigned long ulTimeLength;

    //
    // The number of samples that have been encoded/decoded.
    //
    unsigned long ulTimePos;


    unsigned long N_remain;

    unsigned long N_GeneratedSample;

	unsigned long msadpcm_outsize;
	long OutLength;
	short *poutLeft, *poutRight;
//	BufferState sPlayBuffer;

	long hFile;
	long fRead;
	long fSeek;
	long fLength;

	short *pOutbuf;
	
} tPCM;


typedef struct
{
    //
    // The ADPCM information.
    //
    tPCMheader sPCMHeader;

    //
    // The buffer containing MS ADPCM data.
    //
    unsigned char pucEncodedData[512];

	int		CurDecodeIdx;

    //
    // The buffers containing PCM data.
    //
    //short psLeft[WAV_IMAMAX_PCM_LENGTH * 2];
    //short psRight[WAV_IMAMAX_PCM_LENGTH * 2];

//    BufferState *pOutput;
    //
    // The current offset into the encoded data buffer.
    //
    unsigned long usOffset;

    //
    // The number of valid bytes in the encoded data buffer.
    //
    unsigned long usValid;

    //
    // The offset into the file of the first byte of encoded data.
    //
    unsigned long ulDataStart;

    //
    // The byte-length of the encoded data.
    //
    unsigned long ulLength;

    //
    // The number of bytes of encoded data remaining in the file.
    //
    unsigned long ulDataValid;

    //
    // The byte rate of the encoded file.
    //
    unsigned long usByteRate;

    //
    // The sample rate of the decoded file.
    //
    unsigned long usSampleRate;

    //
    // The number of channels of audio in the file.
    //
    unsigned char ucChannels;


    unsigned char temp_char;

    //
    // The number of bytes in each encoded block of audio.
    //
    unsigned short usBytesPerBlock;  // block

    //
    // The number of samples in each encoded block of audio.
    //
    unsigned short usSamplesPerBlock; // block



    unsigned short uBitsPerSample;
    //
    //Audio Format
    //
    unsigned short wFormatTag;
    //
    // The length of the file in milliseconds.
    //
    unsigned long ulTimeLength;

    //
    // The number of samples that have been encoded/decoded.
    //
    unsigned long ulTimePos;


    unsigned long N_remain;

    unsigned long N_GeneratedSample;

	unsigned long msadpcm_outsize;
	long OutLength;
	short *poutLeft, *poutRight;
//	BufferState sPlayBuffer;

	long hFile;
	long fRead;
	long fSeek;
	long fLength;

	short *pOutbuf;
	
} tPCM_enc;

extern unsigned long WAVFileGetData(tPCM *pPCM);
extern long WAVE_LoadINFOChunk(tPCM *pPCM, unsigned long ChunkSize, unsigned long mem_info_size);
extern long WAVE_LoadDATAChunk(tPCM *pPCM, unsigned long ulLength);
extern long WAVE_LoadChunkFOURCC(tPCM *pPCM,
                                     WAVFOURCCID *pObjectId,
                                     unsigned long *Size);


//-------------------------
//add by Vincent Hisung,Nov 6,2007
extern void InitADPCMEncoder(tPCM_enc *pPCM);
extern unsigned long InitPCMDecoder(tPCM *pPCM);
extern unsigned long MSADPCM_FORMAT(tPCM *pPCM);
extern unsigned long IMAADPCM_FORMAT(tPCM *pPCM);

#endif