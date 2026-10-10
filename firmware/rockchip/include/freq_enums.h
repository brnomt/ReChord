/* freq_enums.h — eFREQ_APP enum (shared; PowerManager.h redefines EXT). */
#ifndef FREQ_ENUMS_H
#define FREQ_ENUMS_H

typedef enum _FREQ_APP
{

    FREQ_IDLE = 0,
    FREQ_MIN,
    FREQ_INIT,
    FREQ_BLON,
    FREQ_AUDIO_INIT,
    FREQ_MP3,       //5
    FREQ_MP3H,
    FREQ_WAV,
    FREQ_AAC,       //10
    FREQ_AACL,
    FREQ_APE,
    FREQ_FLAC,
    FREQ_OGG,
    FREQ_NOGG,
    FREQ_HOGG,      //15
    FREQ_EHOGG,
    FREQ_HAPE,
    FREQ_HFLAC,
    FREQ_HALAC,
    FREQ_DSDIFF,
    FREQ_DSF,
    FREQ_SBC_ENCODING,
    FREQ_BLUETOOTH,
    FREQ_SSRC,
    FREQ_EQ_L,
    FREQ_EQ,
    FREQ_EQ_H,
    FREQ_JPG,
    FREQ_FM,
    FREQ_RECORDADPCM,
    FREQ_FMAUTOSEARCH,
    FREQ_MEDIAUPDATA,//25
    FREQ_USB,
    FREQ_BEEP,
    FREQ_MEDIA_INIT, //30
    FREQ_AVI,
    FREQ_MAX,       //32

    FREQ_APP_MAX


} eFREQ_APP;

#endif /* FREQ_ENUMS_H */
