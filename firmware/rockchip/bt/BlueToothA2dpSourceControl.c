/*
********************************************************************************
*                   Copyright (c) 2009,WangBo
*                      All rights reserved.
*
* File Name：   BlueToothControl.c
*
* Description:
*
* History:      <author>          <time>        <version>
*                 WangBo      2009-4-10       1.0
*    desc:    ORG.
********************************************************************************
*/

#include "SysInclude.h"
#ifdef _BLUETOOTH_
#ifdef _A2DP_SOUCRE_
#define _IN_BLUETOOTH_
#include "audio_file_access.h"

#include "BlueToothControl.h"
#include "AudioControl.h"
#include "MediaBroWin.h"

//#include "btVoiceNotifyProc.h"
#include "bd_addr.h"

#include "BlueToothSysParam.h"
#include "rk_bt_Api.h"

#include "btHwControl.h"
#include "SetBluetooth.h"

#if ((BT_CHIP_CONFIG == BT_CHIP_CC2564) || (BT_CHIP_CONFIG == BT_CHIP_CC2564B))
#include "btCC2564Control.h"
#endif

#if ((BT_CHIP_CONFIG == BT_CHIP_RTL8761))
#include "btRtk8761aControl.h"
#endif

//#define  ENABLE_DEEP_SLEEP
//#define   ENABLE_PAIR_TIMER

//#define GPIO_HOST_RTS     GPIOPortC_Pin0
//#define GPIO_HOST_CTS     GPIOPortB_Pin7
//#define GPIO_NSHOTDOWN    GPIOPortD_Pin2


_ATTR_BLUETOOTHCONTROL_DATA_ uint8  BTA2DPStatus = 0;
_ATTR_BLUETOOTHCONTROL_DATA_ uint8  CC2564SleepConfigStatus = 0;
_ATTR_BLUETOOTHCONTROL_DATA_ uint32  BTState      = 1<< BT_CONNECTING;
_ATTR_BLUETOOTHCONTROL_DATA_ uint32  PowerEvent = 0;
_ATTR_BLUETOOTHCONTROL_DATA_ uint32  bt_connect_handle = 0;
//_ATTR_BLUETOOTHCONTROL_DATA_ uint8  cc2564InitComplete = 0;

_ATTR_BLUETOOTHCONTROL_BSS_  uint8 ConnectReqMac[6];

_ATTR_BLUETOOTHCONTROL_DATA_ uint8  player_volume = -1;
_ATTR_BLUETOOTHCONTROL_BSS_  uint8 btAvrcpVolumeChanged;
extern void BT_UARTInt(void);
extern void  phybusif_h5_data_process(void);
extern void AudioSbcEncodeIntProecss(void);

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothConnectReqProc(struct bd_addr *bdaddr)
{
    memcpy(ConnectReqMac, bdaddr->addr, 6);
#ifdef ENABLE_BQB_RF_TEST
    hci_accept_connection_Request(bdaddr, 0x01); //所有设备都可以连接
#else
    hci_reject_connection_request(bdaddr, 0x0D);

    SET_BTSTATE(BTState,BT_WAIT_PROC_CONNECT_REQ);

#endif

}


_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothCC2564InitComplete(void)
{
    SET_BTSTATE(BTState,BT_CC2564_INIT_COMPLETE);
    //cc2564InitComplete = 1;
}

void h5_hci_link_mode_manage_enable(void);
void h5_hci_link_mode_manage_disable(void);


_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgACLConnected(uint8 result, struct bd_addr *bdaddr)
{
    if(result == 0)
    {
        uint16 *p = (uint16*)((char *)bdaddr -2);
        bt_connect_handle = *p;
        SendMsg(MSG_BLUETOOTH_ACL_CONNECTED);
    }
    else
    {
        SET_BTSTATE(BTState,BT_CONNECTE_ERR);
    }
}


_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgACLDisConnect(struct bd_addr *bdaddr)
{
    gSysConfig.BtConfig.btConnected = 0;
    SendMsg(MSG_BLUETOOTH_ACL_DISCONNECTED);
#ifdef BT_HOST_SNIFF
    h5_hci_link_mode_manage_disable();
#endif
}


_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgA2dpConnected(void)
{
    SendMsg(MSG_BLUETOOTH_A2DP_CONNECTED);
    #ifdef BT_HOST_SNIFF
    h5_hci_link_mode_manage_enable();
    #endif
}

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgAvRcpPaly(void)
{
    SendMsg(MSG_BLUETOOTH_AVRCP_PLAY);
}

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgAvRcpStop(void)
{
    SendMsg(MSG_BLUETOOTH_AVRCP_STOP);
}

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgAvRcpVolumeChanged(uint8 volume)
{
    if (player_volume != volume)
    {
        player_volume = volume;
        SendMsg(MSG_BLUETOOTH_AVRCP_VOLUME_CHANGED);
    }
}

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgAvRcpPause(void)
{
    SendMsg(MSG_BLUETOOTH_AVRCP_PAUSE);
}

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgAvRcpNext(void)
{
    SendMsg(MSG_BLUETOOTH_AVRCP_NXET);
}

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgAvRcpPrev(void)
{
    SendMsg(MSG_BLUETOOTH_AVRCP_PREV);
}

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgA2dpStreaming(void)
{
    SendMsg(MSG_BLUETOOTH_A2DP_STREAMING);
}

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgA2dpSuspend(void)
{
    SendMsg(MSG_BLUETOOTH_A2DP_SUSPEND);

}

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgA2dpClose(void)
{
    gSysConfig.BtConfig.btConnected = 0;
    SendMsg(MSG_BLUETOOTH_A2DP_CLOSE);
}

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgA2dpAbort(void)
{
    gSysConfig.BtConfig.btConnected = 0;
    SendMsg(MSG_BLUETOOTH_A2DP_ABORT);
}


#ifdef ENABLE_DEEP_SLEEP
_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgHcillSleepInd(void)
{
    SendMsg(MSG_BLUETOOTH_HCILL_SLEEP_IND);
}

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgHcillSleepAck(void)
{
    SendMsg(MSG_BLUETOOTH_HCILL_SLEEP_ACK);
}

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgHcillWakeUpInd(void)
{
    SendMsg(MSG_BLUETOOTH_HCILL_WAKE_UP_IND);
}

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgHcillWakeUpAck(void)
{
    SendMsg(MSG_BLUETOOTH_HCILL_WAKE_UP_ACK);
}

#endif

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothPowerEvnetNotify(POWER_NOTIFICATION_t event)
{
    SET_BTSTATE(BTState,BT_GET_POWER_EVENT);

    PowerEvent = event;
}
_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgAvrcpConnected(void)
{
    SendMsg(MSG_BLUETOOTH_AVRCP_CONNECTED);
}

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothMsgInit(void)
{
    ClearMsg(MSG_BLUETOOTH_A2DP_START);
    ClearMsg(MSG_BLUETOOTH_A2DP_PAUSE);
    ClearMsg(MSG_BLUETOOTH_A2DP_PLAY);
    ClearMsg(MSG_BLUETOOTH_ACL_CONNECTED);
    ClearMsg(MSG_BLUETOOTH_ACL_DISCONNECTED);
    ClearMsg(MSG_BLUETOOTH_A2DP_CONNECTED);
    ClearMsg(MSG_BLUETOOTH_A2DP_STREAMING);
    ClearMsg(MSG_BLUETOOTH_A2DP_SUSPEND);
    ClearMsg(MSG_BLUETOOTH_A2DP_CLOSE);
    ClearMsg(MSG_BLUETOOTH_A2DP_ABORT);
    ClearMsg(MSG_BLUETOOTH_AVRCP_CONNECTED);


#ifdef ENABLE_PAIR_TIMER
    ClearMsg(MSG_BLUETOOTH_GO_TO_WAIT_PAIR);
#endif
}




#ifdef ENABLE_PAIR_TIMER

void blueToothWaitPairedTimerProc(void);
_ATTR_BLUETOOTHCONTROL_DATA_
SYSTICK_LIST blueToothWaitPairedTimer =
{
    NULL,
    0,
    PAIR_TIME_OUT,
    0,
    blueToothWaitPairedTimerProc,
};

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothWaitPairedTimerProc(void)
{
#ifdef ENABLE_PAIR_TIMER
    SystickTimerStop(&blueToothWaitPairedTimer);
#endif

    if(!CHECK_BTSTATE(BTState, BT_ACL_CONNECTED))
    {
#ifdef ENABLE_DEEP_SLEEP
        BlueToothHwSleepEnable();
#endif

        bt_discoverable_disable();
    }

}
#endif



_ATTR_BLUETOOTHCONTROL_CODE_
void BlueToothIntInit(void)
{
    IntPendingClear(BT_UART_INT_ID);
    IntPendingClear(BT_HCI_SERVER_INT_ID);
    IntPendingClear(BT_GPIO_INT_ID);
    IntPendingClear(BT_SBC_PROCESS_INT_ID);

    IntRegister(BT_UART_INT_ID, BT_UARTInt);
    //IntRegister(BT_HCI_SERVER_INT_ID , hci_serve);
    IntRegister(BT_HCI_SERVER_INT_ID , phybusif_h5_data_process);
    IntRegister(BT_SBC_PROCESS_INT_ID, AudioSbcEncodeIntProecss);

    IntEnable(BT_UART_INT_ID);
    IntEnable(BT_HCI_SERVER_INT_ID);
    IntEnable(BT_SBC_PROCESS_INT_ID);
}

/*
--------------------------------------------------------------------------------
  Function name :  void BlueToothGetStaus(void)
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
void BlueToothIntDeInit(void)
{
    IntDisable(BT_UART_INT_ID);
    IntDisable(BT_HCI_SERVER_INT_ID);
    IntDisable(BT_SBC_PROCESS_INT_ID);


    IntPendingClear(BT_UART_INT_ID);
    IntPendingClear(BT_HCI_SERVER_INT_ID);
    IntPendingClear(BT_SBC_PROCESS_INT_ID);


    IntUnregister(BT_UART_INT_ID);
    IntUnregister(BT_HCI_SERVER_INT_ID);
    IntUnregister(BT_SBC_PROCESS_INT_ID);

#ifdef ENABLE_PAIR_TIMER
    SystickTimerStop(&blueToothWaitPairedTimer);
#endif

#ifdef ENABLE_DEEP_SLEEP
    IntDisable(BT_GPIO_INT_ID);
    IntPendingClear(BT_GPIO_INT_ID);
    IntUnregister(BT_GPIO_INT_ID);
#endif

}

/*
--------------------------------------------------------------------------------
  Function name :  void BlueToothControlInit(void *pArg)
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
void BlueToothControlInit(void *pArg)
{
    uint32 local_len;
    uint32  local_mac_size;
    char   devMacAddr[6];
    POWER_ON_CONFIG power_on_config;
    char local_name[BT_LOCAL_DEV_NAME_MAX] = {"RKNanoD_Source"};
    char newmac[6]= {0x8c, 0xDE, 0x52, 0x20, 0x55, 0x55,};
    uint32 timeout = 500000;

    gSysConfig.AudioOutputMode = 1;
    gSysConfig.BtConfig.btConnected = 0;
    gSysConfig.EQEnable = 0;

    //local bt name NOT set before Bluetooth thread created.
    if(gSysConfig.BtConfig.LocalBTName[0] == 0)
    {
        local_len = sizeof(local_name);
        if(local_len > BT_LOCAL_DEV_NAME_MAX)
        {
            memcpy(&gSysConfig.BtConfig.LocalBTName[0],local_name,BT_LOCAL_DEV_NAME_MAX);
        }
        else
        {
            memcpy(&gSysConfig.BtConfig.LocalBTName[0],local_name,local_len );
        }
    }

#ifdef BT_ENABLE_SET_MAC
    local_mac_size = 6;
    //local_mac_size = EmmcGetBluetoothMac(gSysConfig.BtConfig.LocalBTMacAddr);
    if(IsValidBtMacAddr(gSysConfig.BtConfig.LocalBTMacAddr))  //
    {
        //use bt mac address read from IDB3 sector.
        memcpy(newmac,&(gSysConfig.BtConfig.LocalBTMacAddr[0]),local_mac_size);

        #if 1
        {//just for debug
            BtMacAddr2String(gSysConfig.BtConfig.LocalBTMacAddr);
        }
        #endif
    }
    else
    {
        //not get Mac addr from system, We use some one.
        DEBUG("### LOCAL Mac read from IDB is NULL ###");
        memcpy(&(gSysConfig.BtConfig.LocalBTMacAddr[0]),newmac,local_mac_size);
    }

#endif

    //Gpio_SetPinLevel(GPIO_BT_STATUS,GPIO_LOW);
    //DEBUG("Enter BlueToothControlInit\n");
    FREQ_EnterModule(FREQ_BLUETOOTH);

    AutoPowerOffDisable();
    //ModuleOverlay(MODULE_ID_LWBT, MODULE_OVERLAY_ALL);
    ModuleOverlay(MODULE_ID_LWBT_UARTIF, MODULE_OVERLAY_ALL);
    ModuleOverlay(MODULE_ID_LWBT, MODULE_OVERLAY_DATA | MODULE_OVERLAY_BSS);
    ModuleOverlay(MODULE_ID_LWBT_INIT, MODULE_OVERLAY_CODE);

    //这里需要一个变量初始化函数
    BTA2DPStatus = BTAUDIO_STATE_STOP;
    //BTCallStatus = BTAUDIO_STATE_STOP;
    //BTADStatus = BT_AD_STATE_STOP;
    BTState      = 1<< BT_CONNECTING;
    CC2564SleepConfigStatus = 0;
    //BTAvcrpStatus = AVCRP_STATE_PAUSE;
    blueToothMsgInit();
    BlueToothIntInit();
    bt_set_init_complete_hook(blueToothCC2564InitComplete);

    //这里需要一个蓝牙串口初始化函数
    //DebugSwitch(DEBUG_TYPE_VIRTUAL_UART);

    memset(&power_on_config, 0, sizeof(POWER_ON_CONFIG));

    power_on_config.bt_init_flag = BT_INIT_FLAG;
    power_on_config.enable_dev_name = TRUE;
    memcpy(power_on_config.dev_name, local_name, sizeof(local_name));
//#ifdef ENABLE_NFC
    power_on_config.enable_dev_mac = TRUE;
    memcpy(&power_on_config.bdaddr, newmac, 6);
//#endif

#if ((BT_CHIP_CONFIG == BT_CHIP_CC2564) || (BT_CHIP_CONFIG == BT_CHIP_CC2564B))
    BlueToothHwInit(&cc2564_hw_control);
#endif

#if ((BT_CHIP_CONFIG == BT_CHIP_RTL8761))
    BlueToothHwInit(&rtk8761a_hw_control);
#endif

    BlueToothHwPowerOn(&power_on_config);
    BlueToothHwRegisterPowerNotification(blueToothPowerEvnetNotify);


    hci_acl_connnect_req(blueToothConnectReqProc);

    hci_acl_connected_notify(blueToothMsgACLConnected);
    hci_acl_disconnect_notify(blueToothMsgACLDisConnect);

    avdtp_streaming_notify(blueToothMsgA2dpStreaming);
    avdtp_suspend_notify(blueToothMsgA2dpSuspend);
    avdtp_close_notify(blueToothMsgA2dpClose);
    avdtp_abort_notify(blueToothMsgA2dpAbort);
    avdtp_connected_notify(blueToothMsgA2dpConnected);


    avctp_connected_notify(blueToothMsgAvrcpConnected);
    avctp_audio_play_notify(blueToothMsgAvRcpPaly);
    avctp_audio_pause_notify(blueToothMsgAvRcpPause);
    avctp_audio_next_notify(blueToothMsgAvRcpNext);
    avctp_audio_previous_notify(blueToothMsgAvRcpPrev);
    avctp_audio_stop_notify(blueToothMsgAvRcpStop);
    avctp_audio_volume_change_notify(blueToothMsgAvRcpVolumeChanged);

#ifdef ENABLE_PAIR_TIMER
    SystickTimerStart(&blueToothWaitPairedTimer);
#endif

    while(timeout)
    {
        if(CHECK_BTSTATE(BTState, BT_CC2564_INIT_COMPLETE))
            //if(cc2564InitComplete == 1)
        {
            CLEAR_BTSTATE(BTState, BT_CC2564_INIT_COMPLETE);
            break;
        }

        timeout--;

        DelayUs(1);
    }
    ModuleOverlay(MODULE_ID_LWBT, MODULE_OVERLAY_CODE);

    SendMsg(MSG_BLUETOOTH_CONNECTING);

    gbBTSwitchOn = BT_ON;
    IsBTOpened = TRUE;

    //DEBUG("Leaving BlueToothControlInit\n");
}


_ATTR_BLUETOOTHCONTROL_CODE_
void BlueToothControlReInit(void)
{
    BlueToothHwSetUartRecAbility(UART_ABILITY_RECV_DISABLE);
    ModuleOverlay(MODULE_ID_LWBT_INIT, MODULE_OVERLAY_CODE);
    bt_deinit();
    bt_init(BT_INIT_FLAG);
#if ((BT_CHIP_CONFIG == BT_CHIP_CC2564) || (BT_CHIP_CONFIG == BT_CHIP_CC2564B))
    phybusif_init(&cc2564_hw_control);
#endif

#if ((BT_CHIP_CONFIG == BT_CHIP_RTL8761))
    //phybusif_init(&rtk8761a_hw_control);
    phybusif_reinit(&rtk8761a_hw_control);
#endif

    hci_acl_connnect_req(blueToothConnectReqProc);
    hci_acl_connected_notify(blueToothMsgACLConnected);
    hci_acl_disconnect_notify(blueToothMsgACLDisConnect);

    avdtp_streaming_notify(blueToothMsgA2dpStreaming);
    avdtp_suspend_notify(blueToothMsgA2dpSuspend);
    avdtp_close_notify(blueToothMsgA2dpClose);
    avdtp_abort_notify(blueToothMsgA2dpAbort);
    avdtp_connected_notify(blueToothMsgA2dpConnected);
    avctp_audio_play_notify(blueToothMsgAvRcpPaly);
    avctp_audio_pause_notify(blueToothMsgAvRcpPause);
    avctp_audio_next_notify(blueToothMsgAvRcpNext);
    avctp_audio_previous_notify(blueToothMsgAvRcpPrev);
    avctp_audio_stop_notify(blueToothMsgAvRcpStop);
    avctp_connected_notify(blueToothMsgAvrcpConnected);

    BlueToothHwSetUartRecAbility(UART_ABILITY_RECV_ENABLE);

    bt_start(BT_START_FLAG_DEFAULT);
    ModuleOverlay(MODULE_ID_LWBT, MODULE_OVERLAY_CODE);

}


/*
--------------------------------------------------------------------------------
  Function name :   UINT32 BlueToothControlService(void)
  Author        :  zs
  Description  :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
extern void BTSBCAudioInit(void);
//avdtp_suspend();


_ATTR_BLUETOOTHCONTROL_CODE_
UINT32 BlueToothControlService(void)
{
    int ret = 0;
    uint16 avdtp_status;
    uint8  call_status;
    uint8  call_setup_status;

//   hci_serve();
    bt_time_serve();

    if(CHECK_BTSTATE(BTState, BT_WAIT_PROC_CONNECT_REQ))
    {
        if(CHECK_BTSTATE(BTState,BT_CONNECTE_ERR))
        {
            uint8 * link_key;
            CLEAR_BTSTATE(BTState, BT_WAIT_PROC_CONNECT_REQ);
            CLEAR_BTSTATE(BTState, BT_CONNECTE_ERR);

            link_key = get_link_key((struct bd_addr *)ConnectReqMac);
            if(link_key)
            {
                bt_a2dp_connect((struct bd_addr *)ConnectReqMac, NULL);
            }
        }
    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_ACL_CONNECTED))
    {
        SET_BTSTATE(BTState,BT_ACL_CONNECTED);
        BlueToothHwSleepDisable(NULL);
        bt_discoverable_disable();

//        #ifdef ENABLE_PAIR_TIMER
//        SystickTimerReStart(&blueToothWaitPairedTimer);
//        #endif
    }


    if(TRUE == GetMsg(MSG_BLUETOOTH_ACL_DISCONNECTED))
    {
        BTA2DPStatus = BTAUDIO_STATE_STOP;
        BTState      = 1<< BT_CONNECTING;
        gSysConfig.BtConfig.btConnected = 0;
        printf("MSG_BLUETOOTH_ACL_DISCONNECTED\n");
        SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
        //SendMsg(MSG_BLUETOOTH_CONNECTING);

        //BlueToothControlReInit();
        bt_discoverable_enable();

        CLEAR_BTSTATE(BTState,BT_ACL_CONNECTED);

        SendMsg(MSG_BLUETOOTH_WIN_DISCONNECTED);

#ifdef   ENABLE_PAIR_TIMER
        SystickTimerStart(&blueToothWaitPairedTimer);
#endif

    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_AVRCP_CONNECTED))
    {
        SET_BTSTATE(BTState,BT_AVRCP_CONNECTED);
    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_A2DP_CONNECTED))
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            if(gSysConfig.AudioOutputMode == 1)
            {
                extern uint sbcTotalCnt;
                extern  uint16 AudioEncodeCnt;
                extern uint16 AudioEncodeLeftCnt;
                extern uint16 AudioDecodeCount;
                extern  uint8 AudioResampleInit;
                extern uint32 AudioResampleRate;
                //extern uint8  UartReqInt;
                extern int AudioUnssrcCnt;
                avdtp_start();
                DelayMs(50);

                sbcTotalCnt = 0;

                AudioEncodeCnt = 0;
                AudioEncodeLeftCnt = 0;

                AudioDecodeCount = 0;
                AudioUnssrcCnt = 0;

                //UartReqInt = 0;
                FREQ_EnterModule(FREQ_SBC_ENCODING);
                sbc_enc_init();

                if(pAudioRegKey->samplerate != FS_44100Hz)
                {
                    if(AudioResampleInit == 0)
                    {
                        if (pAudioRegKey->samplerate >= FS_128KHz)
                        {
                            AudioResampleRate = pAudioRegKey->samplerate/4;
                        }
                        else if (pAudioRegKey->samplerate >= FS_64KHz)
                        {
                            AudioResampleRate = pAudioRegKey->samplerate/2;
                        }
                        else
                        {
                            AudioResampleRate = pAudioRegKey->samplerate;
                        }
                        DEBUG("AudioResampleRate=%d\n", AudioResampleRate);
                        if (AudioResampleRate != FS_44100Hz)
                        {
                            resample_init(pAudioRegKey->channels, AudioResampleRate, 44100);
                            AudioResampleInit = 1;
                        }
                    }
                }
                else
                {
                    AudioResampleRate = pAudioRegKey->samplerate;
                }

                SET_BTSTATE(BTState, BT_A2DP_CONNECTED);
                gSysConfig.BtConfig.btConnected = 1;
            }
        }
        else
        {
            SET_BTSTATE(BTState, BT_A2DP_CONNECTED);
            //SendMsg(MSG_BLUETOOTH_CONNECTED); // 发射这边用来更新sbc 解码器
            gSysConfig.BtConfig.btConnected = 1;
        }
    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_A2DP_STREAMING))
    {
        //音频流接收，立刻关掉sleep

        SET_BTSTATE(BTState, BT_A2DP_STREAMING);
    }

    if (TRUE == GetMsg(MSG_BLUETOOTH_AVRCP_VOLUME_CHANGED))
    {
        extern BOOL avctp_volume_changed_flag;
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            if (player_volume <= MAX_VOLUME && player_volume >= 0)
            {
                BT_DEBUG("avctp_play_volume=%d\n", player_volume);
                MusicOutputVol = player_volume;

                btAvrcpVolumeChanged = 1;
                AudioDecodeProc(MSG_AUDIO_VOLUMESET,NULL);
                btAvrcpVolumeChanged = 0;;

            }
            SendMsg(MSG_MUSIC_DISPFLAG_VOL);
        }
    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_A2DP_CLOSE))
    {
        if(BTA2DPStatus != BTAUDIO_STATE_STOP)
        {
            BTA2DPStatus = BTAUDIO_STATE_STOP;
        }

        CLEAR_BTSTATE(BTState, BT_A2DP_STREAMING);
        CLEAR_BTSTATE(BTState, BT_A2DP_PLAYING);
        CLEAR_BTSTATE(BTState, BT_A2DP_CONNECTED);
    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_A2DP_ABORT))
    {
        CLEAR_BTSTATE(BTState, BT_A2DP_STREAMING);
        CLEAR_BTSTATE(BTState, BT_A2DP_PLAYING);
        CLEAR_BTSTATE(BTState, BT_A2DP_CONNECTED);
    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_A2DP_SUSPEND))
    {
        CLEAR_BTSTATE(BTState, BT_A2DP_STREAMING);
    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_AVRCP_PLAY))
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            //if (AUDIO_STATE_PAUSE == AudioPlayState)
            if (AUDIO_STATE_PAUSE == AudioPlayerState) // yh change 2015/12/28
            {
                BT_DEBUG("play\n");
                MusicDecodeProc(MSG_AUDIO_RESUME, NULL);
            }
            SendMsg(MSG_MUSIC_DISPFLAG_STATUS);
        }
    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_AVRCP_STOP))
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            //if (AUDIO_STATE_PLAY == AudioPlayState)
            if (AUDIO_STATE_PLAY == AudioPlayerState) // yh change 2015/12/28
            {
                BT_DEBUG("stop\n");
                MusicDecodeProc(MSG_AUDIO_PAUSE, NULL);
            }
            SendMsg(MSG_MUSIC_DISPFLAG_STATUS);
        }
    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_AVRCP_PAUSE))
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            //if (AUDIO_STATE_PLAY == AudioPlayState)
            if (AUDIO_STATE_PLAY == AudioPlayerState) // yh change 2015/12/28
            {
                BT_DEBUG("pause\n");
                MusicDecodeProc(MSG_AUDIO_PAUSE, NULL);
            }
            SendMsg(MSG_MUSIC_DISPFLAG_STATUS);
        }
    }


    if(TRUE == GetMsg(MSG_BLUETOOTH_AVRCP_NXET))
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            AudioErrorFileCount = 0;//sen #20090803#1 all audio file is not support
            AudioStopMode = AUDIO_STOP_FORCE;
            MusicNextFile = 1;
            MusicDecodeProc(MSG_AUDIO_NEXTFILE, (void*)MusicNextFile);
        }
    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_AVRCP_PREV))
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            AudioErrorFileCount = 0;//sen #20090803#1 all audio file is not support
            AudioStopMode = AUDIO_STOP_FORCE;
            MusicNextFile = -1;
            MusicDecodeProc(MSG_AUDIO_NEXTFILE, (void*)MusicNextFile);
        }
    }

//////////////////////////////////////////////////////////////////////////////
//低功耗处理
//////////////////////////////////////////////////////////////////////////////
    BlueToothHwService();

    if(CHECK_BTSTATE(BTState,BT_GET_POWER_EVENT))
    {
        CLEAR_BTSTATE(BTState,BT_GET_POWER_EVENT);

        switch(PowerEvent)
        {
            case POWER_WILL_WAKE_UP_CFM:

                CLEAR_BTSTATE(BTState, BT_IN_SLEEP);
#ifdef ENABLE_DEEP_SLEEP
                BlueToothHwSleepDisable();
#endif

                phybusif_send_waitlist();

                if(CHECK_BTSTATE(BTState, BT_WAIT_PALY_NOTIFY))
                {
                    CLEAR_BTSTATE(BTState, BT_WAIT_PALY_NOTIFY);
                    SendMsg(MSG_PLAY_VOICE_WAIT_PAIR);
                }

                if(CHECK_BTSTATE(BTState, BT_NFC_WAIT_CONNECTING))
                {
                    CLEAR_BTSTATE(BTState, BT_NFC_WAIT_CONNECTING);

                    if(gSysConfig.BtConfig.btConnected)
                    {
                        bt_disconnect((struct bd_addr *)&gSysConfig.BtConfig.LastConnectMac);

                        DelayMs(200);

                        gSysConfig.BtConfig.btConnected = 0;
                    }

                    SendMsg(MSG_BLUETOOTH_GO_TO_WAIT_PAIR);
                }
                break;

            case  POWER_WILL_SLEEP:
                SET_BTSTATE(BTState, BT_IN_SLEEP);
                // Os_idle();

                break;

            case POWER_WILL_WAKE_UP_IND:

                CLEAR_BTSTATE(BTState, BT_IN_SLEEP);

                break;
        }
    }


#ifdef   ENABLE_PAIR_TIMER
    if(TRUE == GetMsg(MSG_BLUETOOTH_GO_TO_WAIT_PAIR))
    {
        BlueToothControlReInit();
#ifdef ENABLE_DEEP_SLEEP

        BlueToothHwSleepDisable();

#endif
        bt_discoverable_enable();

#ifdef ENABLE_PAIR_TIMER
        SystickTimerStart(&blueToothWaitPairedTimer);

#endif
    }
#endif
    return ret;
}

/*
--------------------------------------------------------------------------------
  Function name :  void BlueToothControlDeInit(void)
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
void BlueToothControlDeInit(void)
{
    gbBTSwitchOn = BT_OFF;

    gSysConfig.BtConfig.btConnected = 0;
    DelayMs(400);
    bt_disconnect((struct bd_addr *)&gSysConfig.BtConfig.LastConnectMac);
    DelayMs(300);
    ModuleOverlay(MODULE_ID_LWBT_INIT, MODULE_OVERLAY_CODE);
    BlueToothHwPowerOff();

    //UARTInit(24*1000*1000,115200,UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);
    //DebugSwitch(DEBUG_TYPE_UART);

    BlueToothIntDeInit();

    FREQ_ExitModule(FREQ_BLUETOOTH);
    gSysConfig.EQEnable = 1;
}

#endif
#endif
/*
********************************************************************************
*
*                         End of BlueToothControl.c
*
********************************************************************************
*/




