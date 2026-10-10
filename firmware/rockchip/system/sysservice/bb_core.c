#include "SysInclude.h"
#include "audio_main.h"

#ifdef MP3_DEC_INCLUDE
_ATTR_MP3DEC_BIN_TEXT_
const uint8 mp3_code_bin[] =
{
    #include "mp3_code.bin"
};


_ATTR_MP3DEC_BIN_DATA_
uint8 mp3_data_bin[] =
{
    #include "mp3_data.bin"
};
#endif

#ifdef WAV_DEC_INCLUDE
_ATTR_WAVDEC_BIN_TEXT_
const uint8 wav_code_bin[] =
{
    #include "wav_code.bin"
};


_ATTR_WAVDEC_BIN_DATA_
uint8 wav_data_bin[] =
{
    #include "wav_data.bin"
};
#endif

#ifdef AAC_DEC_INCLUDE
_ATTR_AACDEC_BIN_TEXT_
const uint8 aac_code_bin[] =
{
    #include "aac_code.bin"
};


_ATTR_AACDEC_BIN_DATA_
uint8 aac_data_bin[] =
{
    #include "aac_data.bin"
};
#endif


#ifdef OGG_DEC_INCLUDE
_ATTR_OGGDEC_BIN_TEXT_
const uint8 ogg_code_bin[] =
{
    #include "ogg_code.bin"
};


_ATTR_OGGDEC_BIN_DATA_
uint8 ogg_data_bin[] =
{
    #include "ogg_data.bin"
};

#endif

#ifdef _HIFI_APE_DEC
_ATTR_HIFI_APEDEC_BIN_TEXT_
const uint8 hifi_ape_code_bin[] =
{
    #include "hifi_ape_code.bin"
};


_ATTR_HIFI_APEDEC_BIN_DATA_
uint8 hifi_ape_data_bin[] =
{
    #include "hifi_ape_data.bin"
};

#endif

#ifdef _HIFI_FLAC_DEC
_ATTR_HIFI_FLACDEC_BIN_TEXT_
const uint8 hifi_flac_code_bin[] =
{
    #include "hifi_flac_code.bin"
};


_ATTR_HIFI_FLACDEC_BIN_DATA_
uint8 hifi_flac_data_bin[] =
{
    #include "hifi_flac_data.bin"
};

#endif

#ifdef _HIFI_ALAC_DEC
_ATTR_HIFI_ALACDEC_BIN_TEXT_
const uint8 hifi_alac_code_bin[] =
{
    #include "hifi_alac_code.bin"
};


_ATTR_HIFI_ALACDEC_BIN_DATA_
uint8 hifi_alac_data_bin[] =
{
    #include "hifi_alac_data.bin"
};

#ifdef DSDIFF_DEC_INCLUDE
_ATTR_DSDIFFDEC_BIN_TEXT_
const uint8 dsdiff_code_bin[] =
{
    #include "dsdiff_code.bin"
};
_ATTR_DSDIFFDEC_BIN_DATA_
uint8 dsdiff_data_bin[] =
{
    #include "dsdiff_data.bin"
};
#endif

#ifdef DSF_DEC_INCLUDE
_ATTR_DSFDEC_BIN_TEXT_
const uint8 dsf_code_bin[] =
{
    #include "dsf_code.bin"
};
_ATTR_DSFDEC_BIN_DATA_
uint8 dsf_data_bin[] =
{
    #include "dsf_data.bin"
};
#endif

#endif
