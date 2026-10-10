/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   Msg.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _MSG_H_
#define _MSG_H_

#undef  EXT
#ifdef _IN_TASK_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                           Macro define
*
*-------------------------------------------------------------------------------
*/

/*
*-------------------------------------------------------------------------------
*
*                           Struct define
*
*-------------------------------------------------------------------------------
*/
typedef enum
{
    //system message
    MSG_KEYSCAN_PROCESSED = (UINT32)0x00,
    MSG_KEY_VOL_STATE,
    MSG_KEY_VOL_STATE_UPDATE,
    MSG_KEY_VOL_SHOWSTATE,
    MSG_KEYSCAN_HOLD,

    MSG_VBUS_INSERT,                                                 //VBUS insert status
    MSG_VBUS_REMOVED,                                                //VBUS removed
    MSG_USB_SUSPEND,                                                //USB Suspend
    MSG_USB_RESUMED,                                                //USB resumed
    MSG_USB_DISCONNECT,                                             //USB disconnect
    MSG_USB_EXIT_FUSB,                                              //USB Exit fun usb
    MSG_USB_ENTER_FUSB,                                             //USB enter fun usb
    MSG_USB_SUSPEND_FUSB,                                           // Suspend fs usb
    MSG_USB_RESUMED_FUSB,                                           // resume fs usb
    MSG_USB_DISCONNECT_FUSB,                                        //
    MSG_USB_SRL_CONNECT,                                            //USB serial connect
    MSG_USB_CONNECT_FAIL,
    MSG_PC_DISCONNECT_USB,

    MSG_SDCARD_EJECT,                                               //SD card insert status
    MSG_SDCARD_UPDATE,                                             //SD card poll out

    MSG_BL_OFF,                                                     //close backlight
    MSG_SYS_RESUME,                                                 //backlight restore
    MSG_AUTOPOWEROFF_EN,                                            //IDLE shutdown
    MSG_POWER_DOWN,                                                 //shutdown message
    MSG_LOW_POWER,                                                  //low power message.
    MSG_BL_OFF_DISABLE,                                             //close backlight enable.
    MSG_LCD_STANDBY_DISABLE,                                        //do not close LCD when close backlight.
    MSG_BATTERY_UPDATE,
    MSG_CHARGE_ENABLE,
    MSG_CHARGE_START,
    MSG_NEED_PAINT_ALL,
    MSG_NEED_BACKLIGHT_ON,
    MSG_SOUND_PRESSURE_COUNTER_EXPIRED,

    //mian menu UI message
    MSG_MENU_DISPLAY_ALL,                                          //main menu backgroud pictrue display
    MSG_MENU_DISPLAY_ITEM,
    MSG_MENU_DISPLAY_BATT,                                          //main menu display battery
    MSG_MENU_DISPLAY_HOLD,                                          //main menu hold picture display
    MSG_MENU_DISPLAY_DEMO,

    //audio playing  UI message
    MSG_MUSIC_DISPLAY_ALL,
    MSG_MUSIC_DISPLAY_UPDATE,
    MSG_MUSIC_DISPLAY_BACKGROUND,                                       //background
    MSG_MUSIC_DISPFLAG_HOLD,                                            //Hold
    MSG_MUSIC_DISPFLAG_BATT,                                            //Battery
    MSG_MUSIC_DISPFLAG_VOL,                                             //Vol
    MSG_MUSIC_DISPFLAG_CURRENT_TIME,                                    //Current Time
    MSG_MUSIC_DISPFLAG_TOTAL_TIME,                                      //Total   Time
    MSG_MUSIC_DISPFLAG_AB,                                              //A-B
    MSG_MUSIC_DISPFLAG_STATUS,                                          //PLAY/PAUSE
    MSG_MUSIC_DISPFLAG_REPEATMODE,                                      //music playing cycle mode
    MSG_MUSIC_DISPFLAG_ORDER,                                           //music playing random mode
    MSG_MUSIC_DISPFLAG_EQ,                                              //music playing EQ
    MSG_MUSIC_DISPFLAG_BITRATE,                                         //music playing brud rate
    MSG_MUSIC_DISPFLAG_SAMPLERATE ,                                     //music play sampling
    MSG_MUSIC_DISPFLAG_FILENUM,                                         //music play file index
    MSG_MUSIC_DISPFLAG_FILENAME,                                        //audio playing file name
    MSG_MUSIC_DISPFLAG_SCROLL_FILENAME,                                 //whether is need to display scroll file name when audio playing
    MSG_MUSIC_DISPFLAG_AUDIOTYPE,                                       //music type display
    MSG_MUSIC_DISPFLAG_SCHED,                                           //music play progress bar
    MSG_MUSIC_DISPFLAG_SCHED_ALL,                                       //music play progress bar
    MSG_MUSIC_DISPFLAG_LY,                                              //music LY mode
    MSG_MUSIC_DISPFLAG_LRC,                                             //audio playing display LRC
    MSG_MUSIC_DISPFLAG_LRC_SCROLL,
    MSG_MUSIC_LRC_EXIST,                                                //audio has lRC
    MSG_MUSIC_DISPFLAG_SPECTRUM,                                        //audio spectrum display
    MSG_AUDIO_CARD_CHECK,
    MSG_AUDIO_ID3_JPEG,                                                 //album picture
    MSG_AUDIO_FILE_ERROR,
    MSG_MUSIC_NEXTFILE,                                                 //switch next song
    MSG_MUSIC_NEXTFILE_OK,                                             //success to switch next song
    MSG_MUSIC_FADE_IN,
    MSG_MUSIC_FADE_OK,

    //recording UI message
    MSG_RECORDWIN_DISPLAY_ALL,
    MSG_RECORDWIN_DISPFLAG_REFRESH,
    MSG_RECORDWIN_DISPFLAG_BACKGROUND,                              //record base map
    MSG_RECORDWIN_DISPFLAG_HOLD,                                    //Hold
    MSG_RECORDWIN_DISPFLAG_BATT,                                    //Battery
    MSG_RECORDWIN_DISPFLAG_VOL,                                     //VOl
    MSG_RECORDWIN_DISPFLAG_RECQUALITY,                              //record quality(High,Nor)
    MSG_RECORDWIN_DISPFLAG_TIME,                                    //record time(current record time and total left time)
    MSG_RECORDWIN_DISPFLAG_TOTALTIME,
    MSG_RECORDWIN_DISPFLAG_FILENAME,                                //playing recording file name
    MSG_RECORDWIN_DISPFLAG_BITRATE,                                 //playing recording baud rate
    MSG_RECORDWIN_DISPFLAG_SAMPLERATE,                              //playing recording sampling
    MSG_RECORDWIN_DISPFLAG_RADIO,
    MSG_RECORDWIN_DISPFLAG_REC,
    MSG_RECORDWIN_DISPFLAG_REC_STANDBY,

    //recording backgroud message
    MSG_RECORD_STATUS_PREPARE,                                      //recording ready
    MSG_RECORD_STATUS_PLAY,                                         //playing recording
    MSG_RECORD_STATUS_PAUSE,                                        //recording pause
    MSG_RECORD_STATUS_SAVE ,                                        //save recording file
    MSG_RECORD_STATUS_TIME_UPDATE,                                  //update time
    MSG_RECORD_STATUS_GETFILE_FAIL,                                 //fail to display record file name
    MSG_RECORD_STATUS_CREATEFILE_FAIL,                             // fail to create file
    MSG_RECORD_STATUS_DISK_FULL,                                    //disk full(before and in recording)
    MSG_RECORD_STATUS_WRITEFILE_FAIL,                               //write file fail when recording
    MSG_RECORD_STATUS_CLOSEFILE_FAIL,                               //fail to close recording file

    MSG_RECORD_INITUI,                                              //initial recording UI
    MSG_RECORD_SERVICESTART,                                        //start recording backgroud service.
    MSG_RECORD_GETRECORDTYPE,                                       //select recording type
    MSG_RECORD_ENCODESTART,                                         //start coding
    MSG_RECORD_WAVENCODESTOP,                                       //stop coding
    MSG_RECORD_TYPE,                                                //get coding type from backgroud.
    MSG_RECORD_GETINFO,                                             //start recording from bakcgroud
    MSG_RECORD_INIT,                                                //start coding in backgroud
    MSG_RECORD_STARTE,                                              //codeing start,write file head, open timer interrupt.
    MSG_RECORD_PAUSE,                                               //codeing pause
    MSG_RECORD_RESUME,                                              //turn suspend status to recording status,resume recording
    MSG_RECORD_STOP,                                                //stop record,write file,close file
    MSG_RECORD_WRITEFILE,                                           //write file

    //FM  UI message
    MSG_RADIOWIN_DISPLAY_ALL,
    MSG_RADIOWIN_DISPLAY_NULL,                                      //
    MSG_RADIOWIN_DISPLAY_BACK,                                      //FM backgroud image
    MSG_RADIOWIN_DISPLAY_BATT,                                      //Battery
    MSG_RADIOWIN_DISPLAY_HOLD,                                      //
    MSG_RADIOWIN_DISPLAY_FREQ,                                      //FM main window digit frequency display
    MSG_RADIOWIN_DISPLAY_GUAGE,                                     //FM main window progress bar
    MSG_RADIOWIN_DISPLAY_SENS,                                      //
    MSG_RADIOWIN_DISPLAY_STEREO,                                    //STEREO
    MSG_RADIOWIN_DISPLAY_CH,                                        //CH
    MSG_RADIOWIN_DISPLAY_STEREO_UPDATA,
    MSG_RADIOWIN_DISPLAY_VOL,                                       //VOl
    MSG_RADIOWIN_DISPLAY_REGION,                                    //area
    MSG_RADIOWIN_DISPLAY_SETTING,
    MSG_RADIOWIN_DISPLAY_BASICFREQ,
    MSG_RADIOWIN_DISPLAY_UP_DOWN,
    MSG_RADIOWIN_DISPLAY_SCAN,
    MSG_RADIOWIN_DISPLAY_EXIT_FM_MENU,

    //FM child windows  UI message
    MSG_RADIOWIN_DISPLAY_SUB_ALL,
    MSG_RADIOWIN_DISPLAY_SUB_BK,                                    //
    MSG_RADIOWIN_DISPLAY_SUB_SEL,                                   //
    MSG_RADIOWIN_DISPLAY_SUB_LIST_BK,                               //
    MSG_RADIOWIN_DISPLAY_SUB_LIST_SEL,                              //
    MSG_RADIOWIN_DISPLAY_SUB_LIST_SCROLL,                           //
    MSG_RADIOWIN_DISPLAY_SUB_NULL,                                  //
    MSG_RADIOWIN_DISPLAY_SUB_SCROLL,
    MSG_RADIOSUBWIN_DISPLAY_FLAG,                                   //
    MSG_RADIOSUBWIN_DIALOG,                                         //
    MSG_RADIOWIN_DISPLAY_STATUS,
    MSG_RADIO_STERO_UPDATES,
    MSG_RADIOSUBWIN_DISPLAY_FM_MENU,
    MSG_RADIOSUBWIN_DISPLAY_OFF,
    MSG_RADIOSUBWIN_DISPLAY_REC,
    MSG_RADIOSUBWIN_DISPLAY_PLAY_RECDATA,
    MSG_RADIOSUBWIN_DISPLAY_DELETE_RECDATA,
    MSG_RADIOSUBWIN_DISPLAY_CANNOT_CONTINUE,
    MSG_RADIOSUBWIN_SETTING_AUTO_PRESET,
    MSG_RADIOSUBWIN_HOLDONPLAYSAVE,
    MSG_RADIOSUBWIN_HOME_BACK,
    MSG_RADIOSUBWIN_REC_OR_DELETE_REC,

    //FM backgroud service message.
    MSG_FM_INIT,
    MSG_FM_START,
    MSG_FM_HANDSEARCH,
    MSG_FM_AUTOSEARCH,
    MSG_FM_STEROSWITCH,
    MSG_FM_STEPFREQ,
    MSG_FM_STEPSTATION,
    MSG_FM_GETSTEROSTATUS,
    MSG_FM_DEINIT,
    MSG_RADIO_VOLUMESET,                                           //FM volum
    MSG_RADIO_VOLUMRESET,

    //setting menu message
    MSG_SETDIALOG_FLAG,                                             //set menu dialog
    MSG_USEREQ_SET_INITFLAG ,                                       //user EQ setting initial
    MSG_USEREQ_SET_TITLE,                                           //user EQ setting title display
    MSG_USEREQ_SET_DISPFLAG,                                        //user EQ setting display flag
    MSG_SERVICE_EQ_UPDATE,                                          //user EQ setting update
    MSG_PRODUCTINFO,                                                //check produce information
    MSG_FIRMWAREUPGRADE,                                            //frimware update
    MSG_ENTER_LOADER_USB,                                            //enter loader usb
    MSG_MES_FIRMWAREUPGRADE,                                        //frimware update check
    MSG_SYSDEFAULTVALSET,                                           //restore system default setting
    MSG_SYS_SET_DISPLAY_NULL ,                                      //do not refresh
    MSG_SYS_SET_DISPLAY_BATT  ,                                     //
    MSG_SYS_SET_DISPLAY_HOLD,                                       //
    MSG_SYS_SET_DISPLAY,                                            //setting refresh screen
    MSG_SYS_SET_DISPLAY_ALL,                                        //refresh full screen.
    MSG_SYS_SET_DISPLAY_SCR,                                        //fresh scroll item
    MSG_SYS_SET_DISPLAY_TITLE,                                      //title fresh message
    MSG_SYS_SET_DISP_REFRESH_CUR_PREV_ITEM ,                        //cursor scroll to up
    MSG_SYS_SET_DISP_REFRESH_CUR_NEXT_ITEM ,                        //cursor scroll to dowm
    MSG_SYS_SET_INFO_SCROLL,
    MSG_SYS_SET_MUSIC_HOLD,                                          //EQ ylz++
    MSG_SYS_DISPLAY_CANNOT_CONTINUE,

    MSG_FORMAT_FLASH,
    MSG_FORMAT_SDCARD,                                               //sd card format chad.ma add
    // Brightness menu message
    MSG_BRIGHT_DISPLAY_ALL,

    //USB & charge message
    MSG_USB_DISPLAY_CHARGE,
    MSG_USB_DISPLAY_ALL,
    MSG_USB_DISPFLAG_STATUS,
    MSG_USB_DISPFLAG_VOL,
    MSG_USB_DISPFLAG_EQ,
    MSG_USB_DISPFLAG_SPECTRUM,
    MSG_USB_DISPFLAG_SAMPLERATE,

    //DialogBox
    MSG_DIALOG_DISPLAY_ALL,                                         //
    MSG_DIALOG_DISPLAY_BUTTON,                                      //
    MSG_DIALOG_KEY_OK,                                              //
    MSG_DIALOG_KEY_CANCEL,                                          //

    //MessageBox
    MSG_MESSAGE_DISPLAY_ALL,
    MSG_MESSAGEBOX_DESTROY,                                         //messagebox destory
    MSG_NO_FILE,

    //Hold
    MSG_HOLD_DISPLAY_ALL,
    MSG_HOLDDIALOG_DESTROY,

    //LowPower
    MSG_LOWPOWER_DISPLAY_ALL,
    MSG_LOWPOWER_DESTROY,

    //audio playing backgroud message
    MSG_AUDIO_DECINIT,                                              //
    MSG_AUDIO_DECSTART,                                             //
    MSG_AUDIO_PAUSE,                                                //
    MSG_AUDIO_RESUME,                                               //
    MSG_AUDIO_FF_PAUSE,                                            //
    MSG_AUDIO_FF_RESUME,                                           //
    MSG_AUDIO_STOP,                                                 //
    MSG_AUDIO_DECODE_ERROR,
    MSG_AUDIO_DECODE_END,
    MSG_AUDIO_NOFILE,                                               //no audio file.
    MSG_AUDIO_OPENERROR,                                            //audio file open error.
    MSG_AUDIO_INIT_SUCESS,
    MUSIC_UPDATESPECTRUM,                                            //update spectrum

    MSG_AUDIO_FFD,                                                  //
    MSG_AUDIO_FFW,                                                  //
    MSG_AUDIO_SETPOSI,                                              //
    MSG_AUDIO_ABSETA,                                               //
    MSG_AUDIO_ABPLAY,                                               //
    MSG_AUDIO_ABSTOP,                                               //stop AB repeat,return to normal play status
    MSG_AUDIO_VOLUMESET,
    MSG_AUDIO_MUTEOFF,
    MSG_AUDIO_EQSET,
    MSG_AUDIO_EQSET_UPDATA,
    MSG_AUDIO_EQSET_UPDATE_USER_EQ,                                 //by Vincent , update user EQ
    MSG_AUDIO_EQ_PRO,
    MSG_AUDIO_EQ_FADE_OUT,

    MSG_AUDIO_GETBITRATE,                                           //
    MSG_AUDIO_GETFILENAME,                                          //
    MSG_AUDIO_GETSAMPLERATE,                                        //
    MSG_AUDIO_GETLENGTH,                                            //
    MSG_AUDIO_GETPOSI,                                              //
    MSG_AUDIO_UPDATETIME,                                           //
    MSG_AUDIO_REPORTERR,                                            //
    MSG_AUDIO_NEXTFILE,                                             //
    MSG_AUDIO_UNMUTE,                                               //
    MSG_AUDIO_EXIT,
    MSG_SERVICE_MUSIC_MODE_UPDATE,  //music play mode update.it will be sent when setting menu,music receive.
    MSG_SERVICE_MUSIC_ORDER_UPDATE, //music order play update,it will be sent when setting menu,music receive.
    MSG_AUDIO_DECODE_FILL_BUFFER,
    MSG_SHUFFLE_FIRST_NEXT,
    MSG_SHUFFLE_FIRST_PREV,

    MSG_XVID_INIT,
    MSG_XVID_OPEN,
    MSG_XVID_START,
    MSG_XVID_DECODE,
    MSG_XVID_CLOSE,
    MSG_XVID_STOP,
    MSG_XVID_PAUSE,
    MSG_XVID_RESUME,
    MSG_XVID_NEXTFILE,
    MSG_XVID_VOLUMESET,
    MSG_XVID_FFD,
    MSG_XVID_FFW,
    MSG_XVID_NOFILE,
    MSG_XVID_DISPFLAG_STATUS,                                          //PLAY/PAUSE
    MSG_XVID_DISPLAY_ALL,
    MSG_VIDEO_DISPFLAG_BATTERY,
    MSG_VIDEO_DISPFLAG_CURRENT_TIME,
    MSG_VIDEO_DISPFLAG_TOTAL_TIME,
    MSG_VIDEO_DISPFLAG_FILENUM,
    MSG_VIDEO_DISPFLAG_SCHED,
    MSG_VIDEO_DISPFLAG_VOL,
    MSG_VIDEO_CARD_CHECK,
    MSG_VIDEO_FILENOTSUPPORT,
    MSG_VIDEO_EXIT_BROWSER,
    MSG_VIDEO_DISPLAY_HOLD,
    MSG_VIDEO_DISPLAY_UNHOLD,
    MSG_VIDEO_VERTICAL_DISPLAY,  //ylz++

    //ebook message
    MSG_BOOK_DISPLAY_ALL,
    MSG_BOOK_DISPLAY_HOLD,
    MSG_BOOK_DISPLAY_BATT,
    MSG_BOOK_DISPLAY_BACKGROUND,
    MSG_BOOK_DISPLAY_TITLE,
    MSG_BOOK_DISPLAY_TITLE_TOTAIPAGE,
    MSG_BOOK_DISPLAY_TITLE_CURRENTPAGE,
    MSG_BOOK_DISPLAY_TITLE_SCROLL,
    MSG_BOOK_DISPLAY_UPDATE,
    MSG_BOOK_DISPLAY_PLAYSTATUS,
    MSG_BOOK_DISPLAY_WIN_FALG,   //YLZ++

    MSG_BOOK_MARK_DISPLAY_ALL,
    MSG_BOOK_MARK_DISPLAY_HOLD,
    MSG_BOOK_MARK_DISPLAY_BATT,
    MSG_BOOK_MARK_DISPLAY_BACKGROUND,
    MSG_BOOK_MARK_DISPLAY_TITLE,
    MSG_BOOK_MARK_DISPLAY_ITEM,
    MSG_BOOK_MARK_UPDATE,
    MSG_BOOK_MARK_SCROLL_UPDATE,

    //explorer ui
    MSG_BROW_DIS_ALL,
    MSG_BROW_DIS_BAR,
    MSG_BROW_DIS_ALL_ITEM,
    MSG_BROW_DIS_SELE_ITEM,
    MSG_BROW_DIS_BATT,
    MSG_BROW_DIS_HOLD,
    MSG_BROW_DIS_SCLLO_ITEM,
    BROWSER_DISPFLAG_SCROLL_FILENAME,
    MSG_BROW_DELETE_FILE,
    MSG_BROW_DELETEING_FILE,//it is deleting file.
    MSG_BROW_FROM_MAINMENU,
    MSG_BROW_CHECK_FILE,
    MSG_BROW_TASK_HAS_SWITCH,

    MSG_BROW_TO_BREAKPOINT,


    //picture module message
    //ui-->image
    MSG_IMAGE_DECSTART,
    MSG_IMAGE_DEC,
    MSG_IMAGE_PAUSE,
    MSG_IMAGE_RESUME,
    MSG_IMAGE_STOP,
    MSG_IMAGE_DECNEXT,
    MSG_IMAGE_GET_NEXT_FILE,
    MSG_IMAGE_GET_TWO_NEXT_FILE,

    //image-->ui
    MSG_IMAGE_NOFILE,//no file
    MSG_IMAGE_OPENERROR,
    MSG_IMAGE_INIT_SUCESS,
    MSG_IMAGE_INIT_ERROR,
    MSG_IMAGE_DEC_SUCESS,
    MSG_IMAGE_DEC_ERROR,
    MSG_IMAGE_DEC_END,//decode finish
    MSG_IMAGE_UNSPORTED_PIC,

    //media libary message
    MSG_MEDIAWIN_DISPLAY_ALL,
    MSG_MEDIAWIN_ALL_ITEM,
    MSG_MEDIAWIN_FRESH_ITEM,

    MSG_MEDIABRO_DISPLAY_ALL,
    MSG_MEDIABRO_ALL_ITEM,
    MSG_MEDIABRO_FRESH_ITEM,
    //MSG_MEDIABRO_FRESH_THUMBNAIL,
    MSG_MEDIABRO_FRESH_THUMBNAIL_UP,
    MSG_MEDIABRO_FRESH_THUMBNAIL_DOWN,
    MSG_MEDIABRO_UPDATE_THUMBNAIL,
    MSG_MEDIABRO_DISPFLAG_SCROLL_FILENAME,
    MSG_MEDIABRO_NOFIND_FILE,
    MSG_MEDIABRO_DEMO_ALL_SONGS,

    MSG_PICBRO_DISPLAY_ALL,
    MSG_PICBRO_ALL_ITEM,
    MSG_PICBRO_FRESH_ITEM,
    MSG_PICBRO_FRESH_THUMBNAIL,
    MSG_PICBRO_DISPFLAG_SCROLL_FILENAME,
    MSG_PICBRO_NOFIND_FILE,
    MSG_PICBRO_DEMO_ALL_SONGS,

    MSG_MEDIABROSUB_DISPLAY_ALL,
    MSG_MEDIABROSUB_ALL_ITEM,
    MSG_MEDIABROSUB_FRESH_ITEM,

    MSG_MEDIAFAVOSUB_DISPLAY_ALL,
    MSG_MEDIAFAVOSUB_ALL_ITEM,
    MSG_MEDIAFAVOSUB_FRESH_ITEM,
    MSG_MEDIA_BREAKPOINT_PLAY,
    MSG_MEDIA_NOW_PLAY,
    MSG_MEDIA_SCROLL_PAINT,
    MSG_MEDIALIB_SCROLL_PAINT,
    MSG_MEDIALIB_UPDATA,
    MSG_MEDIALIB_TO_BREAKPOINT,
    MSG_FILE_IN_FAVO,
    MSG_MEDIAFAVO_DELALL, //clear favourite

    MSG_SYS_REBOOT,
    MSG_FLASH_MEM0_UPDATE,
    MSG_FLASH_MEM1_UPDATE,
    MSG_SDCARD_MEM_UPDATE,
    MSG_FORMAT_DISK,

    MSG_SYS_FW_RECOVERY,
    MSG_SYS_FW_UPGRADE,

    MSG_SYS_CLEAR_KEYVAL,

    MSG_MUSIC_DISPFLAG_STATUS_FF,
    MSG_MUSIC_DISPFLAG_FILENAME_ICON,
    MSG_MUSIC_DISPFLAG_SELECTION_SOURCE,

    //Volume dialog
    MSG_VOLUME_DISPLAY_ALL,
    MSG_VOLUMEDIALOG_DESTROY,

    //BlueTooth message
    MSG_BLUETOOTH_DISPLAY_ALL,
    MSG_BLUETOOTH_CONNECTING,
    MSG_BLUETOOTH_CONNECTED,
    MSG_BLUETOOTH_PAIR_ING,
    MSG_BLUETOOTH_PAIR_SUCCEED,
    MSG_BLUETOOTH_BUFFER_NODATA,
    MSG_BLUETOOTH_RECONNECT,

    MSG_BLUETOOTH_A2DP_START,
    MSG_BLUETOOTH_A2DP_PAUSE,
    MSG_BLUETOOTH_A2DP_PLAY,
    MSG_BLUETOOTH_SCO_CONNECTED,
    MSG_BLUETOOTH_SCO_DISCONNECTED,
    MSG_BLUETOOTH_ACL_CONNECTED,
    MSG_BLUETOOTH_ACL_DISCONNECTED,
    MSG_BLUETOOTH_CALL_STATUS_CHANGE,
    MSG_BLUETOOTH_CALL_SETUP_STATUS_CHANGE,
    MSG_BLUETOOTH_CALL_RING,

    MSG_BLUETOOTH_HFP_GET_PHONE_NO,
    MSG_BLUETOOTH_HFP_CONNECTED,
    MSG_BLUETOOTH_HFP_DISCONNECTED,
    MSG_BLUETOOTH_A2DP_CONNECTED,
    MSG_BLUETOOTH_A2DP_STREAMING,
    MSG_BLUETOOTH_A2DP_SUSPEND,
    MSG_BLUETOOTH_A2DP_CLOSE,
    MSG_BLUETOOTH_A2DP_ABORT,
    MSG_BLUETOOTH_GET_VAD,
    MSG_BLUETOOTH_AVRCP_CONNECTED,

    MSG_BLUETOOTH_OPP_CONNECTED,
    MSG_BLUETOOTH_OPP_CREATE_FILE,
    MSG_BLUETOOTH_OPP_CLOSE_FILE,
    MSG_BLUETOOTH_OPP_WRITE_FILE,
    MSG_BLUETOOTH_OPP_DELETE_FILE,
    MSG_BLUETOOTH_OPP_DISCONNECTED,
    MSG_BLUETOOTH_OPP_DISPFLAG_FILENAME,
    MSG_BLUETOOTH_OPP_DISPFLAG_STANDY,
    MSG_BLUETOOTH_OPP_DISPFLAG_SCHED,

    MSG_BLUETOOTH_DISPLAY_CALL_COMING,
    MSG_BLUETOOTH_DISPLAY_CALL_PLAY,
    MSG_BLUETOOTH_DISPLAY_CALL_STOP,
    MSG_BLUETOOTH_AVRCP_PLAY,
    MSG_BLUETOOTH_AVRCP_STOP,
    MSG_BLUETOOTH_AVRCP_PAUSE,
    MSG_BLUETOOTH_AVRCP_NXET,
    MSG_BLUETOOTH_AVRCP_PREV,
    MSG_BLUETOOTH_AVRCP_VOLUME_CHANGED,
    MSG_BLUETOOTH_DISCONNECT_SUCCEED,
    MSG_BLUETOOTH_LINK_KEY_NOTIFY,
    MSG_BLUETOOTH_CONNECT_SUCCEED,
    MSG_BLUETOOTH_CONNECT_FAIL,
    MSG_BLUETOOTH_CONNECT_RETURN,
    MSG_BLUETOOTH_WIN_DISCONNECTED,
    MSG_BLUETOOTH_SCAN_WIN_DESTROY,

    MSG_BT_SCAN_REQUEST,
    MSG_BT_SHOW_LOCAL_DEV_INFO,  //Show bt local device infomation
    MSG_BT_SHOW_PARIED_DEV_INFO,  //Show bt local device infomation
    MSG_BT_DEV_MAC_SCROLL,      //scroll bt mac addr
    MSG_BT_DEVINFO_WIN_DESTROY,

    MSG_BT_DISPLAY_PINCODE_INIT,    //show pin code number
    MSG_BT_DISPLAY_PINCODE_UPDATE,  //update pin code display
    MSG_BT_DISPLAY_PINCODE_TIPS,    //show pin code tips string
    MSG_BT_INPUT_PINCODE_OK,
    MSG_BT_INPUT_PINCODE_CANCLE,
    MSG_BT_DISPLAY_PINKEY_ALL,
    MSG_BT_DISPLAY_PINKEY_BUTTON,
    MSG_BT_CONFIRM_PINCODE_OK,
    MSG_BT_CONFIRM_PINCODE_NO_OK,


    MSG_PLAY_VOICE_WAIT_PAIR,
    MSG_PLAY_VOICE_NOTIFY_FINISH,
    MSG_PLAY_VOICE_NOTIFY_ACL_DISCONNECTED,

    MSG_BLUETOOTH_HCILL_SLEEP_IND,
    MSG_BLUETOOTH_HCILL_SLEEP_ACK,
    MSG_BLUETOOTH_HCILL_WAKE_UP_IND,
    MSG_BLUETOOTH_HCILL_WAKE_UP_ACK,
    MSG_BLUETOOTH_GET_WAKE_UP_INT,
    MSG_BLUETOOTH_NFC_INT_COMING,
    MSG_BLUETOOTH_GO_TO_WAIT_PAIR,

    MSG_BLUETOOTH_SPP_WORK,
    MSG_BLUETOOTH_SVAE_PAIR_DEV,
    MSG_BT_PIN_CODE_REQ,
    MSG_BT_AUTH_OK,
    MSG_BT_AUTH_FAIL,
    MSG_BT_USER_CONFIRMATION_REQUEST,
    MSG_BT_HCI_CANNEL_CONNECT_OK,
    MSG_BT_HCI_CANNEL_CONNECT_FAIL_CONNECED_EXIT,
    MSG_BT_HCI_CANNEL_CONNECT_FAIL_NO_CONNECTED,
#ifdef _USB_HOST_
    MSG_USBHOT_UPDATE,
#endif
//End
    MSG_ID_MAX  //max message number

}MSG_ID;

/*
*-------------------------------------------------------------------------------
*
*                           Variable define
*
*-------------------------------------------------------------------------------
*/
//_ATTR_OS_BSS_  EXT UINT32  Msg[(MSG_ID_MAX / 32) + 1];

/*
*-------------------------------------------------------------------------------
*
*                           Functon Declaration
*
*-------------------------------------------------------------------------------
*/
extern void SendMsg(MSG_ID MsgId);
extern BOOL GetMsg(MSG_ID MsgId);
extern void ClearMsg(MSG_ID MsgId);
extern BOOL CheckMsg(MSG_ID MsgId);

/*
********************************************************************************
*
*                         End of Msg.h
*
********************************************************************************
*/
#endif
