#ifndef    _FMCONTROL_
#define    _FMCONTROL_

#undef  EXT
#ifdef _IN_FM_CONTROL_
#define EXT
#else
#define EXT extern
#endif



//section define
#define _ATTR_FMCONTROL_TEXT_     
#define _ATTR_FMCONTROL_DATA_     
#define _ATTR_FMCONTROL_BSS_      

#define    TUNERFREQINDEX           20
#define    TUNERFREQSTEP            10              /* UP/DOWN TUNER frequency STEP    unit 100K        */

#define     FREQMAXNUMBLE           30

#define FM_MIX_HP           1
#define FM_I2S_HP           2
#define FM_OUT_CONFIG       FM_I2S_HP

#define FmFreqMin  FmFreqMinVal
#define FmFreqMax  FmFreqMaxVal    //10800

#define gbFmFreq        (gpRadioplayerRegKey->FmFreq)
#define gbFmFreqIndex   (gpRadioplayerRegKey->FmSaveNum)
#define RadioOutputVol  (gSysConfig.OutputVolume)
#define     FM_OPT_MODE_MANUAL              0 //hand
#define     FM_OPT_MODE_PRESET              1 //preset
#define     FM_OPT_MODE_SAVE                2 //save
#define     FM_OPT_MODE_DELETE              3 //delete
#define     FM_OPT_MODE_AUTOSAVE            4 //automatic
//add Rec/paly rec data/del rec data in FM menu
#define     FM_OPT_MODE_REC                 5 //REC
#define     FM_OPT_MODE_PLAY_REC_DATA       6 //play rec data
#define     FM_OPT_MODE_DELETE_REC_DATA     7 //delete rec data
#define     FM_OPT_MODE_MAX                 8
#define     FM_SAVE_MAX_COUNTER             30

#define     RADIO_CHINA         0                       //china
#define     RADIO_EUROPE        1                       //europe
#define     RADIO_JAPAN         2                       //japan
#define     RADIO_USA           3                       //america

#define     RADIO_STEREO_CLOSE  0                       //sterec open
#define     RADIO_STEREO_OPEN   1                       //sterec close.

//_ATTR_FMCONTROL_BSS_  EXT UINT32   FmFunctionSele;
_ATTR_FMCONTROL_BSS_  EXT UINT32   FmFunctionSeleBack;

//_ATTR_FMCONTROL_BSS_  EXT UINT16   FmListSaveAndDel;
_ATTR_FMCONTROL_BSS_  EXT UINT16   gbFmTuneRegion;
_ATTR_FMCONTROL_BSS_  EXT UINT16   gbFreqBack;

_ATTR_FMCONTROL_BSS_  EXT UINT16  FmFreqMaxVal;   //max frequency.(japan fm station)
_ATTR_FMCONTROL_BSS_  EXT UINT16  FmFreqMinVal;
_ATTR_FMCONTROL_BSS_  EXT UINT8   FmFreqNum;
_ATTR_FMCONTROL_BSS_  EXT UINT8   FmAutoSearchStop;

_ATTR_FMCONTROL_BSS_  EXT UINT8   FmFirstEntryflag;//fjp
_ATTR_FMCONTROL_BSS_  EXT UINT32  FmFirstEntryCount;
_ATTR_FMCONTROL_BSS_  EXT UINT32  FmRegVolFlag;
typedef enum
{
    FM_InvalidStation = 0,
    FM_SearchFail,
    FM_FoundStation
}FM_SEARCH_RESULT;

typedef enum
{
    CH5800 = 0,
    JA5800
} FM_AREA;

typedef enum
{
    FM_SearchModeState_ByHand = 0,
    FM_SearchModeState_Auto
}FmSearchModeState;

typedef enum
{
    FM_State_HandStepFreq = 0,
    FM_State_AutoSearch,
    FM_State_HandSearch,
    FM_State_StepStation,
    FM_State_Searching,
    FM_State_Idle
}FM_STATE;

typedef enum
{
    FM_DIRECT_DEC = 0,       //direct of decreasing
    FM_DIRECT_INC,           //direct of increment
    FM_DIRECT_Idle
}FM_DIRECT;

typedef struct
{
    UINT16    FmSearchMode;                     // FM_SearchModeState_ByHand , FM_SearchModeState_Auto
    UINT16    FmSearchDirect;                   // search direction.
    UINT32    FmArea;                           // SID_RadioListAreaChina,  SID_RadioListAreaJapan, SID_RadioListAreaUSA, SID_RadioListAreaEurope
    BOOLEAN   FmStereo;                         // TRUE -- STEREO; FALSE -- MONO
    UINT16    FmState;                          // FM_State_HandStepFreq , FM_State_AutoSearch, FM_State_StepStation, FM_State_Searching, FM_State_Idle
    UINT32    FmFreq;                           // current fm station frequency.
    BOOLEAN   FmAutoSearchState;                // autoimation search status,FALSE -- don't not search station;TRUE -- get station
    UINT16    FmSaveNum;
    UINT16    FmFreqArray[FREQMAXNUMBLE];
   // UINT16    FmFreqChArray[FREQMAXNUMBLE];
   // UINT16    FmFreqJpArray[FREQMAXNUMBLE];
   // UINT16    FmFreqCaArray[FREQMAXNUMBLE];
    UINT8     FmHandSearchSt;
    UINT16    PreFmSaveNum; //prevent auto-search station refresh screen too much.

}FM_GLOBAL;

_ATTR_FMCONTROL_BSS_ EXT FM_DRIVER_TABLE_T  FmDriverTable;

#ifdef _IN_FM_CONTROL_
_ATTR_FMCONTROL_DATA_ EXT UINT8  FMDeInitDisable = 0;
#else
_ATTR_FMCONTROL_DATA_ EXT UINT8  FMDeInitDisable;
#endif


#ifdef _IN_FM_CONTROL_
_ATTR_FMCONTROL_DATA_ EXT BOOL  FmGetStereoStatus = TRUE;
#else
_ATTR_FMCONTROL_DATA_ EXT BOOL  FmGetStereoStatus;
#endif

#ifdef _IN_FM_CONTROL_
_ATTR_FMCONTROL_DATA_ EXT FM_GLOBAL     gFMConfig =         //sturcture write to register.
{
    FM_SearchModeState_ByHand,  //FmSearchMode;          // FM_SearchModeState_ByHand , FM_SearchModeState_Auto
    FM_DIRECT_INC,              //FmSearchDirect;                 //search direction
    0,                          //SID_RadioListAreaChina,     //3FmArea; // SID_RadioListAreaChina,  SID_RadioListAreaJapan, SID_RadioListAreaUSA, SID_RadioListAreaEurope
    1,                          //TRUE -- STEREO; FALSE -- MONO
    FM_State_HandStepFreq,      //FmState// FM_State_HandStepFreq , FM_State_AutoSearch, FM_State_StepStation, FM_State_Searching, FM_State_Idle
    8700,                       //FmFreq// current FM frequency
    0,                          //FmAutoSearchState;   false:no search out  station.  true:search out station.
    0,                          //FmSaveNum;
    {0},                        //preset station list
   // {0},//
   // {0},//
   // {0},//
    0
};
#else
_ATTR_FMCONTROL_DATA_ EXT FM_GLOBAL     gFMConfig;
#endif

extern FM_SRUCT_CODEC_CONFIG FmCodecConfig_General[MAX_VOLUME+1];
extern FM_SRUCT_CODEC_CONFIG FmCodecConfig_Europe[MAX_VOLUME+1];

//_ATTR_FMCONTROL_BSS_ UINT16  RadioOutputVol ;

_ATTR_FMCONTROL_BSS_ EXT UINT16         UpDownFlag;
_ATTR_FMCONTROL_BSS_ EXT FM_GLOBAL*     gpRadioplayerRegKey;   //global variable pointers used in application and driver.
_ATTR_FMCONTROL_BSS_ EXT UINT16         FM_RecordQuality;   //quality of recording.
_ATTR_FMCONTROL_BSS_ EXT CodecFS_en_t   FM_FS;
_ATTR_FMCONTROL_BSS_ EXT uint32         ACodec_I2S_Data_Width;
_ATTR_FMCONTROL_BSS_ EXT uint32         M_I2S_Data_Width;
_ATTR_FMCONTROL_BSS_ EXT UINT16         FM_RecordFmt;   //format of recording.
_ATTR_FMCONTROL_BSS_ EXT UINT32         FM_RecordLen;   //format of recording.
_ATTR_FMCONTROL_BSS_ EXT uint32         fm_is_stop;

BOOLEAN FM_Process(MSG_ID id, void *msg);
void FMInit(void *pArg);
UINT32 FMService(void);
void FMDeInit(void);
void PaintCHAndFreq(void);

/*
--------------------------------------------------------------------------------

  Description: fm thread definition.

--------------------------------------------------------------------------------
*/

#ifdef _IN_FM_CONTROL_
_ATTR_FMCONTROL_DATA_ EXT THREAD FMThread = {

    NULL,
    NULL,

    FMInit,
    FMService,
    FMDeInit,

    NULL
};
#else
_ATTR_FMCONTROL_DATA_ EXT THREAD FMThread;
#endif
#endif

