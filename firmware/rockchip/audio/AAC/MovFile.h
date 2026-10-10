/********************************************************************************************
*	Copyright (C), 2008, Fuzhou Rockchip Co.,Ltd.
*   All Rights Reserved
*
*	File:
*              MovFile.h
*	Description:
*
*	Author:
*              guosl
*	Note:
*	           None.
*	$Log: MovFile.h,v $
*	Revision 1.1.1.1  2009/06/18 09:19:55  zjd
*	submit the frist version by ZhenFu Fang in 06.18.2009
*
*	Revision 1.1  2008/07/08 07:29:20  HZF
*	add 3GP add mp4 format support.
*
*	Revision 1.1  2008/5/16 14:43:19  guosl
*	no message
*
*
*
*
********************************************************************************************/

#ifndef _MOVFILE_H
#define _MOVFILE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef  _IN_MOVFILE_H
    #undef  EXT
    #define EXT
#else
    #define EXT extern
#endif
#define ONLY_AUDIO
#define MOV_H264_MAX_FRAME_WIDTH 1920
#define MOV_H264_MAX_FRAME_HEIGHT 1080

#define MOV_MAX_FRAME_WIDTH 1280
#define MOV_MAX_FRAME_HEIGHT 720

/*#define MOV_FILE_PARSING_OK         0
#define MOV_FILE_FORMAT_ERR         -1
#define MOV_FILE_INDX_ERR           -2
#define MOV_FILE_NO_MOVI_CHUNK      -3
#define MOV_FILE_RESOLUTION_ERR     -4
#define MOV_FILE_VIDEO_CODEC_ERR    -5
#define MOV_FILE_AUDIO_CODEC_ERR    -6
#define MOV_FILE_OPEN_ERR           -7*/

#define MOV_FILE_PARSING_OK 0
#define MOV_FILE_BOX_PARSE_SUCCESS 0
#define MOV_FILE_BOX_PARSE_ERR -1
#define MOV_FILE_AUDIO_UNSUPPORTED -2
#define MOV_FILE_VIDEO_UNSUPPORTED -3
#define MOV_FILE_RESOLUTION_UNSUPPORTED -4
#define MOV_READ_DATA_FAILED -5
#define MOV_FILE_AUDIO_CODEC_ERR -6
#define MALLOC_FAILED -7
#define MOV_UNSUPPORTED_STREAM -8


/* open file state */
#define MOV_FILE_OPEN_ERR -1
#define MOV_FILE_OPEN_SUCCESS 0

/* memory section length for H.264 stream repacketed */
#define H264_VIDEO_BUFF_LEN 50000

/* mov play state */
#define MOV_STATE_PALYING 0
#define MOV_STATE_FFD 1
#define MOV_STATE_FFW 2
#define MOV_STATE_PAUSE 3
#define MOV_STATE_STOP 4
#define MOV_STATE_INITIALIZING 5

/* four bytes */
#define FOURBYTE 4

typedef struct tagVFILE
{
    unsigned short Flags;
    char Name[11];
    unsigned int DirClus;
    unsigned int FileSize;
    unsigned int FstClus;
    unsigned int Clus;
    unsigned int Offset;
    unsigned int RefClus;
    unsigned int RefOffset;
    void   *pVolume;
    unsigned int       err;
    void   *BufData;

    unsigned int BufClus;
    unsigned char  *pBuf;
   //080716,huangsl,
   #ifdef PCLIB
    void        *ClibFileForWrite;
   #endif
} MOVFILE;

/*data type define */
#ifndef DWORD
typedef unsigned long int DWORD;
#endif

typedef DWORD FOURCC;
#ifndef LONG
/*typedef unsigned long int LONG;*/
typedef long  LONG;
#endif
#ifndef BYTE
typedef unsigned char BYTE;
#endif
#ifndef WORD
typedef short int WORD;
#endif

#ifndef uint8
#define uint8   unsigned char
#endif
#ifndef uint16
#define uint16  unsigned short
#endif
#ifndef uint32
#define uint32  unsigned int
#endif

#ifndef uint64
#define uint64  unsigned long long
#endif

#ifndef int16
#define int16	short
#endif

#ifndef int32
#define int32  	int
#endif

#ifndef STATIC
#define STATIC static
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

#ifndef SEEK_END
#define SEEK_END 2
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef ASSERT
#define ASSERT(x)	if (x == 0) return -1
#endif




extern FILE * fopen_hl(const char * path,const char * mode);

extern unsigned short  fread_hl(void*buffer,size_t size,size_t count,FILE*stream);
extern unsigned char fseek_hl(FILE *stream, long offset, int fromwhere);
extern unsigned  long ftell_hl(FILE *stream);
extern unsigned char  fclose_hl(FILE *stream);





#ifdef BOARD
#define MovRead     MovParser_Sys_interface.Fread
#define MovSeek     MovParser_Sys_interface.Fseek
#define MovOpen     MovParser_Sys_interface.Fopen
#define MovClose    MovParser_Sys_interface.Fclose
#define MovTell     MovParser_Sys_interface.Ftell
#define MovWrite    //FS_fwrite
#define MovFILE     MOVFILE
#else
#define MovRead     fread_hl
#define MovSeek     fseek_hl
#define MovOpen     fopen_hl
#define MovClose    fclose_hl
#define MovTell     ftell_hl
#define MovWrite    fwrite
#define MovFILE     FILE
#endif

#define MovSTopen   movBufInit
#define MovSTclose  movBufdeinit
#define MovSTread   movBufRead
#define MovSTseek   movBufSeek
#define MovSTtell   movBufTell
#define MOV_ST    	MovBuf
#define MALLOC      MovMemAlloc
#define FREE        MovMemFree

#define BYTESWAP(a) (((a >> 24) & 0x000000ff) | ((a >> 8) & 0x0000ff00) | ((a << 8) & 0x00ff0000) | (a << 24))

#define WORDSWAP(a) (((a >> 8) & 0x00ff) | (a << 8))

#define UINT64SWAP(a) (((a >> 56) & 0x00000000000000ffll) | ((a >> 40) & 0x000000000000ff00ll) \
                      | ((a >> 24) & 0x0000000000ff0000ll) | ((a >> 8) & 0x00000000ff000000ll)   \
                      | ((a << 8) & 0x000000ff00000000ll) | ((a << 24) & 0x0000ff0000000000ll)   \
                      | ((a << 40) & 0x00ff000000000000ll) | (a << 56))


#define SIGN_FTYP 0x70797466 //ftyp
#define SIGN_PDIN 0x6E696470 //pdin
#define SIGN_MOOV 0x766F6F6D //moov
#define SIGN_MOOF 0x666F6F6D //moof
#define SIGN_MFRA 0x6172666D //mfra
#define SIGN_FREE 0x65657266 //free
#define SIGN_MDAT 0x7461646D //mdat
#define SIGN_SKIP 0x70696B73 //skip
#define SIGN_META 0x6174656D //meta
#define SIGN_WIDE 0x65646977 //wide
#define SIGN_UUID 0x64697575 //uuid

#define SIGN_MVEX 0x7865766D  //mvex
#define SIGN_TRAK 0x6B617274 //trak
#define SIGN_MVHD 0x6468766D //mvhd
#define SIGN_IPMC 0x636D7069 //ipmc
#define SIGN_MFHD 0x6468666D //mfhd
#define SIGN_TRAF 0x66617274 //traf
#define SIGN_TFRA 0x61726674 //tfra
#define SIGN_MFRO 0x6F72666D //mfro
#define SIGN_UDTA 0x61746475 //udta
#define SIGN_HDLR 0x726C6468 //hdlr
#define SIGN_DINF 0x666E6964 //dinf
#define SIGN_IPMC 0x636D7069 //ipmc
#define SIGN_ILOC 0x636F6C69 //iloc
#define SIGN_IPRO 0x6F727069 //ipro
#define SIGN_IINF 0x666E6969 //iinf
#define SIGN_XML  0x206C6D78 //xml
#define SIGN_BXML 0x6C6D7862 //bxml
#define SIGN_PITM 0x6D746970 //pitm

#define SIGN_TKHD 0x64686B74 //tkhd
#define SIGN_TREF 0x66657274 //tref
#define SIGN_EDTS 0x73746465 //edts
#define SIGN_MDIA 0x6169646D //mdia
#define SIGN_MEHD 0x6468656D //mehd
#define SIGN_TREX 0x78657274 //trex
#define SIGN_TFHD 0x64686674 //tfhd
#define SIGN_TRUN 0x6E757274 //trun
#define SIGN_CPRT 0x74727063 //cprt

#define SIGN_STSD 0x64737473 //stsd
#define SIGN_STTS 0x73747473 //stts
#define SIGN_CTTS 0x73747463 //ctts
#define SIGN_STSC 0x63737473 //stsc
#define SIGN_STSZ 0x7A737473 //stsz
#define SIGN_STZ2 0x327A7473 //stz2
#define SIGN_STCO 0x6F637473 //stco
#define SIGN_CO64 0x34366F63 //co64
#define SIGN_STSS 0x73737473 //stss
#define SIGN_STSH 0x68737473 //stsh
#define SIGN_PADB 0x62646170 //padb
#define SIGN_STDP 0x70647473 //stdp
#define SIGN_SGPD 0x64706773 //sgpd
#define SIGN_SDTP 0x70746473 //sdtp
#define SIGN_SBGP 0x70676273 //sbgp
#define SIGN_SUBS 0x73627573 //subs
#define SIGN_ESDS 0x73647365 //esds
#define SIGN_AVCC 0x43637661 //avcC

#define SIGN_DREF 0x66657264 //dref
#define SIGN_URL  0x206C7275 //"url "
#define SIGN_URN  0x206E7275 //"urn "
#define SIGN_SINF 0x666E6973 //sinf

#define SIGN_ELST 0x74736C65 //elst
#define SIGN_MDHD 0x6468646D //mdhd
#define SIGN_HDLR 0x726C6468 //hdlr
#define SIGN_MINF 0x666E696D //minf
#define SIGN_FRMA 0x616D7266 //frma
#define SIGN_IMIF 0x66696D69 //imif
#define SIGN_SCHM 0x6D686373 //schm
#define SIGN_SCHI 0x69686373 //schi

#define SIGN_VMHD 0x64686D76 //vmhd
#define SIGN_SMHD 0x64686D73 //smhd
#define SIGN_HMHD 0x64686D68 //hmhd
#define SIGN_NMHD 0x64686D6E //nmhd
#define SIGN_DINF 0x666E6964 //dinf
#define SIGN_STBL 0x6C627473 //stbl


#define SIGN_VIDEO 0x0000
#define SIGN_AUDIO 0x0001
#define SIGN_OTHER 0x0002

#define HANDLER_VIDE 0x65646976
#define HANDLER_SOUN 0x6E756F73

#define MP4ESDescrTag 0x03
#define MP4DecConfigDescrTag 0x04
#define MP4DecSpecificDescrTag 0x05

#define VIDEO_TRACK 0x65646976 //video track
#define AUDIO_TRACK 0x6E756F73 //audio track

#define VIDEOCODE_MP4V 0x7634706D //mp4v
#define VIDEOCODE_S263 0x33363273 //s263
#define VIDEOCODE_H263 0x33363268 //h263 ljf@20100507
#define VIDEOCODE_AVC1 0x31637661 //avc1
#define AUDIOCODE_SAMR 0x726D6173 //samr
//#define AUDIOCODE_SAWB 0x62776173 //sawb
#define AUDIOCODE_MP4A 0x6134706D //mp4a
#define AUDIOCODE_MP3 0x33706D2E //mp3

#define VIDEO_CODEC_LIB_NULL 0
#define VIDEO_CODEC_LIB_MP4V 1
#define VIDEO_CODEC_LIB_H263 2
#define VIDEO_CODEC_LIB_H264 3

#define AUDIO_CODEC_LIB_NULL 0
#define AUDIO_CODEC_LIB_SAMR 1
#define AUDIO_CODEC_LIB_MP4A 2
#define AUDIO_CODEC_LIB_MP3  3

#define STSZ_MAX_MALLOC_SPACE 0x573D8
#define STSZ_SAVEED_BASE 0x60DA30A0
#define STSZ_SAVED_END 0x60EFFFFF
//#define STSZ_SAVED_END 0x61e00000

//#define H264_FRAME_HEAD_LENGTH 0x10
#define H264_FRAME_HEAD_LENGTH 0x4
#define VIDEO_DATA_END -2
#define ALIGN_DWORD 0x4//align by byte
typedef enum
{
    FFD_STATE,
    FFW_STATE,
    NO_SKIP,
    SEEK_TIME,
    RESUME
}SKIPSTATE;

typedef struct
{
    uint32  clus;               //current cluster number.
    uint32  offset;             //file pointer offset.
}FILE_POS;

typedef struct
{
    uint32 low_part;
    uint32 high_part;
} MovDuration;

typedef struct
{
	uint32  readSize;     /* size read out. */
	uint32  chunkOffset;
	uint32  curChunkSize;
    uint32  sampleSum;
    uint32  firstSampleNo;
    uint32  endSampleNo;
}CurChunkinf;

typedef struct
{
    uint32  readSize;
    uint32  sampleOffset;
    uint32  curSampleSize;
    //uint32  msTime;
}CurSampleinf;

/* just used in mp4v */
typedef struct
{
    FILE_POS esdsFilePos;
    uint32   length;
}EsdsBoxinf;

/* just used in H.264 */
typedef struct
{
    FILE_POS avcCFilePos;
    uint32   length;
}AvcCBoxinf;

//#define MOV_BUFFER_SIZE 4096
#define MOV_BUFFER_SIZE 512

typedef struct
{
    uint8  buf[MOV_BUFFER_SIZE];
    uint16 rdpos;
    uint16 rdmax;
    FILE *st;
} MovBuf;

//#define MOV_MEM_SIZE    (211 * 1024) //223
#define MOV_MEM_SIZE    (20* 1024) //223

typedef struct
{
    uint8 buf[MOV_MEM_SIZE];
    uint32 sizeused;
} MovMem;

typedef struct
{
    MOV_ST *indexbuf;
    uint32 indexpos;
    uint32 curpos;
} MovIndex;

typedef struct
{
    CurChunkinf  audioChunk;        /* current video sample information. */
	CurChunkinf  videoChunk;
    CurSampleinf audioSample;
    CurSampleinf videoSample;

    EsdsBoxinf   esdsInf;
    AvcCBoxinf   avcCinf;

    uint32       curAudioChunkNo;
    uint32       curVideoChunkNo;
    uint32       curAudioSampleNo;
    uint32       curVideoSampleNo;
    uint32       realAudioSampleNo;

    uint32       audioChunkNum;
    uint32       videoChunkNum;
    uint32       audioSampleNum;
    uint32       videoSampleNum;
    uint32       audioSampleSize;
    uint32       videoSampleSize;
    uint32       videoSampleToChunkNo;
    uint32       audioSampleToChunkNo;

    uint32       audioTimeToSampleNo;
    uint32       videoTimeToSampleNo;

    int          videoSyncSampleNo;

    uint32       videoTimeScale;
    uint32       audioTimeScale;
    uint32       uint32VideoDuration;
    uint32       uint32AudioDuration;
    //uint64       uint64VideoDuration;
    //uint64       uint64AudioDuration;
    MovDuration  VideoDuration;
    MovDuration  AudioDuration;
    uint32       length ;  //ms

    WORD         channelCount;   //either 1(mono) or2 (stereo)
    WORD         res; /* ljf@20100409 for align */

    uint32       sampleRate;     //audio sampling rate

    WORD         width;
    WORD         height;


    WORD		 aFormat;
	WORD		 vFormat;
    BYTE		 audioFlag;			/* audio flag. if audio stream is not exist or not supported, its value will be 0. */
	BYTE		 videoFlag;			/* video flag. if video stream is not exist or not supported, its value will be 0. */
    BYTE         reserved[2];       /* ljf@20100409 for align */

    MOV_ST       movVidBuf;
    MOV_ST       movAudBuf;

    char         AudioDec[16];
    char         VideoDec[4];

}MovFileinf;

typedef struct M4a_Name
{
   //char path[3 + (8 - 1) * 12 + 1];
   char name[11];
   int32 Clus;
   int32 Index;
}M4a_Name ;


/*the SOBD data in H264. */
typedef struct
{
    uint32 sodb_length;
    uint32 sodb_consumed;
}SODB;

EXT MovFileinf gMovFile;
EXT SKIPSTATE gBeingSkip;


EXT int MovFileInit(FILE* aac_raw_file);

EXT int	MovFileParsing(MOV_ST *videoFile);

EXT void MovFileClose(void);



EXT int MovAudioGetDataToSDRAM(MOV_ST *videoFile,
                               MovFileinf *pFileinf,
                               char *buffer,
                               const uint32 *size);




EXT int MovGetAudioFormat(void);

EXT int MovAudioSeek(MOV_ST *videoFile, long offset, int origin);

EXT void MovGlobalVarInit(void);

EXT void MovGlobalVarDspInit(void);

EXT void MovSynAudio2Video(long videoSampleNo);







EXT void MovGenerateMovFrmHeader(char *pHeader,
								 uint32 frmLength,
								 uint32 msTime);


EXT int MovIF_close(void);

EXT int MovIF_AudioGetStream(char *buff, int size, int mode);
EXT int MovIF_AudioFSeek(long offset, int origin);
EXT int MovIF_SynAudio2Video(unsigned int timems);

EXT unsigned int MovIF_AudioGetCurrentTime(void);


#ifdef __cplusplus
}
#endif

#endif


