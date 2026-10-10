#ifndef _H_SF_WAV_

#define _H_SF_WAV_

#include <stdio.h>

typedef int sf_count_t ;

#define SFE_MALLOC_FAILED (-1)
#define SFE_BAD_SEEK (-2)

enum
{       /* Major formats. */
        SF_FORMAT_WAV                   = 0x010000,             /* Microsoft WAV format (little endian). */
        SF_FORMAT_AIFF                  = 0x020000,             /* Apple/SGI AIFF format (big endian). */
        SF_FORMAT_AU                    = 0x030000,             /* Sun/NeXT AU format (big endian). */
        SF_FORMAT_RAW                   = 0x040000,             /* RAW PCM data. */
        SF_FORMAT_PAF                   = 0x050000,             /* Ensoniq PARIS file format. */
        SF_FORMAT_SVX                   = 0x060000,             /* Amiga IFF / SVX8 / SV16 format. */
        SF_FORMAT_NIST                  = 0x070000,             /* Sphere NIST format. */
        SF_FORMAT_VOC                   = 0x080000,             /* VOC files. */
        SF_FORMAT_IRCAM                 = 0x0A0000,             /* Berkeley/IRCAM/CARL */
        SF_FORMAT_W64                   = 0x0B0000,             /* Sonic Foundry's 64 bit RIFF/WAV */
        SF_FORMAT_MAT4                  = 0x0C0000,             /* Matlab (tm) V4.2 / GNU Octave 2.0 */
        SF_FORMAT_MAT5                  = 0x0D0000,             /* Matlab (tm) V5.0 / GNU Octave 2.1 */
        SF_FORMAT_PVF                   = 0x0E0000,             /* Portable Voice Format */
        SF_FORMAT_XI                    = 0x0F0000,             /* Fasttracker 2 Extended Instrument */
        SF_FORMAT_HTK                   = 0x100000,             /* HMM Tool Kit format */

        /* Subtypes from here on. */

        SF_FORMAT_PCM_S8                = 0x0001,               /* Signed 8 bit data */
        SF_FORMAT_PCM_16                = 0x0002,               /* Signed 16 bit data */
        SF_FORMAT_PCM_24                = 0x0003,               /* Signed 24 bit data */
        SF_FORMAT_PCM_32                = 0x0004,               /* Signed 32 bit data */

        SF_FORMAT_PCM_U8                = 0x0005,               /* Unsigned 8 bit data (WAV and RAW only) */

        SF_FORMAT_FLOAT                 = 0x0006,               /* 32 bit float data */
        SF_FORMAT_DOUBLE                = 0x0007,               /* 64 bit float data */

        SF_FORMAT_ULAW                  = 0x0010,               /* U-Law encoded. */
        SF_FORMAT_ALAW                  = 0x0011,               /* A-Law encoded. */
        SF_FORMAT_IMA_ADPCM             = 0x0012,               /* IMA ADPCM. */
        SF_FORMAT_MS_ADPCM              = 0x0013,               /* Microsoft ADPCM. */

        SF_FORMAT_GSM610                = 0x0020,               /* GSM 6.10 encoding. */
        SF_FORMAT_VOX_ADPCM             = 0x0021,               /* OKI / Dialogix ADPCM */

        SF_FORMAT_G721_32               = 0x0030,               /* 32kbs G721 ADPCM encoding. */
        SF_FORMAT_G723_24               = 0x0031,               /* 24kbs G723 ADPCM encoding. */
        SF_FORMAT_G723_40               = 0x0032,               /* 40kbs G723 ADPCM encoding. */

        SF_FORMAT_DWVW_12               = 0x0040,               /* 12 bit Delta Width Variable Word encoding. */
        SF_FORMAT_DWVW_16               = 0x0041,               /* 16 bit Delta Width Variable Word encoding. */
        SF_FORMAT_DWVW_24               = 0x0042,               /* 24 bit Delta Width Variable Word encoding. */
        SF_FORMAT_DWVW_N                = 0x0043,               /* N bit Delta Width Variable Word encoding. */

        SF_FORMAT_DPCM_8                = 0x0050,               /* 8 bit differential PCM (XI only) */
        SF_FORMAT_DPCM_16               = 0x0051,               /* 16 bit differential PCM (XI only) */


        /* Endian-ness options. */

        SF_ENDIAN_FILE                  = 0x00000000,   /* Default file endian-ness. */
        SF_ENDIAN_LITTLE                = 0x10000000,   /* Force little endian-ness. */
        SF_ENDIAN_BIG                   = 0x20000000,   /* Force big endian-ness. */
        SF_ENDIAN_CPU                   = 0x30000000,   /* Force CPU endian-ness. */

        SF_FORMAT_SUBMASK               = 0x0000FFFF,
        SF_FORMAT_TYPEMASK              = 0x0FFF0000,
        SF_FORMAT_ENDMASK               = 0x30000000
} ;


enum
{       /* True and false */
        SF_FALSE        = 0,
        SF_TRUE         = 1,

        /* Modes for opening files. */
        SFM_READ        = 0x10,
        SFM_WRITE       = 0x20,
        SFM_RDWR        = 0x30
} ;

/* A pointer to a SF_INFO structure is passed to sf_open_read () and filled in.
** On write, the SF_INFO structure is filled in by the user and passed into
** sf_open_write ().
*/

struct SF_INFO
{       sf_count_t      frames ;                /* Used to be called samples.  Changed to avoid confusion. */
        int                     samplerate ;
        int                     channels ;
        int                     format ;
        int                     sections ;
        int                     seekable ;
} ;

typedef struct SF_INFO SF_INFO ;

/*
** Enums and typedefs for adding dither on read and write.
** See the html documentation for sf_command(), SFC_SET_DITHER_ON_WRITE
** and SFC_SET_DITHER_ON_READ.
*/

enum
{       SFD_DEFAULT_LEVEL       = 0,
        SFD_CUSTOM_LEVEL        = 0x40000000,

        SFD_NO_DITHER           = 500,
        SFD_WHITE                       = 501,
        SFD_TRIANGULAR_PDF      = 502
} ;

typedef struct
{       int                     type ;
        double          level ;
        const char      *name ;
} SF_DITHER_INFO ;


#define SF_BUFFER_LEN                   (8192)
#define SF_FILENAME_LEN                 (256)
#define SF_HEADER_LEN                   (2048)
#define SF_TEXT_LEN                             (1024)
#define SF_SYSERR_LEN                   (256)
#define SF_MAX_STRINGS                  (16)
#define PEAK_CHANNEL_COUNT              (16)


#define BITWIDTH2BYTES(x)       (((x) + 7) / 8)

/*      For some reason SIGNED_SIZEOF returns an unsigned  value which causes
**      a warning when that value is added or subtracted from a signed
**      value. Use SIGNED_SIZEOF instead.
*/
#define         SIGNED_SIZEOF(x)        ((int) sizeof (x))

#define         SF_COUNT_MAX_POSITIVE(x)                \
                        {       x = 1 ;                                         \
                                while (((x << 1) + 1) > 0)      \
                                        x = (x << 1) + 1 ;              \
                                } ;

enum
{       /* PEAK chunk location. */
        SF_PEAK_START           = 42,
        SF_PEAK_END                     = 43,

        /* PEAK chunk location. */
        SF_SCALE_MAX            = 52,
        SF_SCALE_MIN            = 53,

        /* str_flags values. */
        SF_STR_ALLOW_START      = 0x0100,
        SF_STR_ALLOW_END        = 0x0200,

        /* Location of strings. */
        SF_STR_LOCATE_START     = 0x0400,
        SF_STR_LOCATE_END       = 0x0800,

        SFD_TYPEMASK            = 0x0FFFFFFF
} ;

#define         SFM_MASK        (SFM_READ | SFM_WRITE | SFM_RDWR)
#define         SFM_UNMASK      (~SFM_MASK)

/*---------------------------------------------------------------------------------------
** Formats that may be supported at some time in the future.
** When support is finalised, these values move to src/sndfile.h.
*/

enum
{       /* Work in progress. */
        SF_FORMAT_SDS                   = 0x0C0000,             /* Midi Sample Sump Standard */

        /* Formats supported read only. */
        SF_FORMAT_WVE                   = 0x4020000,            /* Psion ALaw Sound File */
        SF_FORMAT_TXW                   = 0x4030000,            /* Yamaha TX16 sampler file */
        SF_FORMAT_DWD                   = 0x4040000,            /* DiamondWare Digirized */

        /* Following are detected but not supported. */
        SF_FORMAT_OGG                   = 0x4090000,

        SF_FORMAT_REX                   = 0x40A0000,            /* Propellorheads Rex/Rcy */
        SF_FORMAT_SD2                   = 0x40C0000,            /* Sound Designer 2 */
        SF_FORMAT_REX2                  = 0x40D0000,            /* Propellorheads Rex2 */
        SF_FORMAT_KRZ                   = 0x40E0000,            /* Kurzweil sampler file */
        SF_FORMAT_OCT                   = 0x40F0000,            /* GNU Octave data file */
        SF_FORMAT_WMA                   = 0x4100000,            /* Windows Media Audio. */
        SF_FORMAT_SHN                   = 0x4110000,            /* Shorten. */
        SF_FORMAT_FLAC                  = 0x4120000,

        /* Unsupported encodings. */
        SF_FORMAT_VORBIS                = 0x1001,

        SF_FORMAT_SVX_FIB               = 0x1020,               /* SVX Fibonacci Delta encoding. */
        SF_FORMAT_SVX_EXP               = 0x1021,               /* SVX Exponential Delta encoding. */

        SF_FORMAT_PCM_N                 = 0x1030
} ;

/*---------------------------------------------------------------------------------------
**      PEAK_CHUNK - This chunk type is common to both AIFF and WAVE files although their
**      endian encodings are different.
*/

typedef struct
{       float        value ;            /* signed value of peak */
        unsigned int position ;         /* the sample frame for the peak */
} PEAK_POS ;

typedef struct
{       unsigned int  version ;                                         /* version of the PEAK chunk */
        unsigned int  timestamp ;                                       /* secs since 1/1/1970  */
        PEAK_POS      peak [PEAK_CHANNEL_COUNT] ;       /* the peak info */
} PEAK_CHUNK ;

typedef struct
{       int             type ;
        int             flags ;
        char    *str ;
} STR_DATA ;

/*=======================================================================================
**      SF_PRIVATE stuct - a pointer to this struct is passed back to the caller of the
**      sf_open_XXXX functions. The caller however has no knowledge of the struct's
**      contents.
*/

typedef struct sf_private_tag
{
        unsigned char   header          [SF_HEADER_LEN] ; /* Must be unsigned */
        int                             rwf_endian ;    /* Header endian-ness flag. */

        char                    *str_end ;
        int                             str_flags ;

        int                             Magick ;

        unsigned int    logindex ;
        unsigned int    headindex, headcurrent ;
        int                             has_text ;
        int                             do_not_close_descritor ;

        int                     filedes ;
        FILE            *fileptr;
        int                             end_of_file ;
        int                             error ;

        int                             mode ;                  /* Open mode : SFM_READ, SFM_WRITE or SFM_RDWR. */
        int                             endian ;                /* File endianness : SF_ENDIAN_LITTLE or SF_ENDIAN_BIG. */
        int                             float_endswap ; /* Need to endswap float32s? */

        SF_INFO                 sf ;

        int                             have_written ;  /* Has a single write been done to the file? */
        int                             has_peak ;              /* Has a PEAK chunk (AIFF and WAVE) been read? */
        int                             peak_loc ;              /* Write a PEAK chunk at the start or end of the file? */
        PEAK_CHUNK              peak ;

        sf_count_t              filelength ;    /* Overall length of (embedded) file. */
        sf_count_t              fileoffset ;    /* Offset in number of bytes from beginning of file. */

        sf_count_t              dataoffset ;    /* Offset in number of bytes from beginning of file. */
        sf_count_t              datalength ;    /* Length in bytes of the audio data. */
        sf_count_t              dataend ;               /* Offset to file tailer. */

        int                             blockwidth ;    /* Size in bytes of one set of interleaved samples. */
        int                             bytewidth ;             /* Size in bytes of one sample (one channel). */

        void                    *dither ;
        void                    *interleave ;

        int                             last_op ;               /* Last operation; either SFM_READ or SFM_WRITE */
        sf_count_t              read_current ;
        sf_count_t              write_current ;

        void                    *fdata ;                /*      This is a pointer to dynamically allocated file format
                                                                        **      specific data.
                                                                        */

        int                             norm_double ;
        int                             norm_float ;

        int                             auto_header ;

        int                             ieee_replace ;
        /* A set of file specific function pointers */

        sf_count_t              (*read_short)   (struct sf_private_tag*, short *ptr, sf_count_t len) ;

        sf_count_t              (*write_short)  (struct sf_private_tag*, short *ptr, sf_count_t len) ;

        sf_count_t              (*seek)                 (struct sf_private_tag*, int mode, sf_count_t samples_from_start) ;

} SF_PRIVATE ;

#endif
