#include "SysInclude.h"


#ifdef  _BLUETOOTH_

#include "BlueToothControl.h"
#include "AudioControl.h"
#include "BlueToothSysParam.h"

#include "btHwControl.h"
#include "btRtk8761aControl.h"

#include "lwbterr.h"
#include "pbuf.h"

#include "rk_bt_Api.h"

#include "bd_addr.h"


#if (BT_CHIP_CONFIG == BT_CHIP_RTL8761)

#define HCI_VENDER_DOWNLOAD_OCF     0x20    //mlc
#define PATCH_DATA_FIELD_MAX_SIZE     252


/*-----------------------------------------------------------------------------------*/
int32 Rtk8761aHwReset(void);
int32 Rtk8761aHwInit(void);
int32 Rtk8761aHwDownInitScript(void);
int32 Rtk8761aHwSetChipBaudrate(uint baudrate);

extern void GetBeepSourceInf(uint32 ModuleNum, uint32 *baseAddr, uint32 *moduleLen);

#define CMD_HEAD_LEN 4



typedef struct
{
    int(*power_event_notify)(POWER_NOTIFICATION_t event);
    uint8 cmd_complete_flag;
    uint8 Rtk8761aSleepConfigStatus;
    uint8 PowerStatus;
    uint8 needChangeMac;
    struct bd_addr bdaddr;

} _rtk8761aPcb_;

_rtk8761aPcb_ rtk8761apcb;


_ATTR_BLUETOOTHCONTROL_CODE_
int rtk8761a_init_complete(void *arg, struct hci_pcb *pcb, uint8 ogf,
                           uint8 ocf, uint8 result)
{
    rtk8761apcb.cmd_complete_flag = 1;
    if(result == 0x00) //HCI_SUCCESS
    {
        return 0;
    }

    return 1;
}

_ATTR_BLUETOOTHCONTROL_CODE_
int wait_rtk8761a_cmd_complete(uint timeoutUs)
{
    while(--timeoutUs)
    {
        if(rtk8761apcb.cmd_complete_flag)
        {
            rtk8761apcb.cmd_complete_flag = 0;
            return RETURN_OK;
        }

        DelayUs(1);
    }

    return TIMEOUT;
}

_ATTR_BLUETOOTHCONTROL_CODE_
int32 rtk8761a_power_up(void)
{
    uint32 timeout = 400;

    //8761
#if BT_VCC_ON_GPIO_CH != 0xFF
    Grf_GpioMuxSet(BT_VCC_ON_GPIO_CH, BT_VCC_ON_GPIO_PIN,0);
    Gpio_SetPinDirection(BT_VCC_ON_GPIO_CH, BT_VCC_ON_GPIO_PIN, GPIO_OUT);
    Gpio_SetPinLevel(BT_VCC_ON_GPIO_CH,BT_VCC_ON_GPIO_PIN,GPIO_LOW);
#endif

    Grf_GpioMuxSet(BT_POWER_GPIO_CH, BT_POWER_GPIO_PIN,0);
    Gpio_SetPinDirection(BT_POWER_GPIO_CH, BT_POWER_GPIO_PIN, GPIO_OUT);

#if BT_VCC_ON_GPIO_CH != 0xFF
    Gpio_SetPinLevel(BT_VCC_ON_GPIO_CH,BT_VCC_ON_GPIO_PIN,GPIO_HIGH);
    DelayMs(10);
#endif

    Gpio_SetPinLevel(BT_POWER_GPIO_CH,BT_POWER_GPIO_PIN,GPIO_LOW);
    DelayMs(10);

    Gpio_SetPinLevel(BT_POWER_GPIO_CH,BT_POWER_GPIO_PIN,GPIO_HIGH);

    DelayMs(600);
    //DEBUG("rtk8761_power_up");

    while(timeout--)
    {
        DelayMs(10);
        if (1 == Gpio_GetPinLevel(BT_HOST_RX_CH, BT_HOST_RX_PIN))
        {
            //SendMsg(MSG_BLUETOOTH_POWERON);
            return RETURN_OK;
        }
    }
    return RETURN_FAIL;
}

/*
--------------------------------------------------------------------------------
  Function name :  void BlueToothPowerOn(void )
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
int32  rtk8761a_power_down(void)
{
    uint32 timeout = 20;

    Gpio_SetPinLevel(BT_POWER_GPIO_CH, BT_POWER_GPIO_PIN ,GPIO_LOW);

    DelayMs(1100);
#if BT_VCC_ON_GPIO_CH != 0xFF
    Gpio_SetPinLevel(BT_VCC_ON_GPIO_CH,BT_VCC_ON_GPIO_PIN,GPIO_LOW);
#endif

    Gpio_SetPinDirection(BT_POWER_GPIO_CH, BT_POWER_GPIO_PIN, GPIO_IN);
#if BT_VCC_ON_GPIO_CH != 0xFF
    DelayMs(100);
    Gpio_SetPinDirection(BT_VCC_ON_GPIO_CH, BT_VCC_ON_GPIO_PIN, GPIO_IN);
#endif

    return RETURN_FAIL;
}


_ATTR_BLUETOOTHCONTROL_CODE_
void rtk8761a_set_host_rts_leave(eGPIOPinLevel_t level)
{
    Gpio_SetPinLevel(BT_HOST_RTS_CH, BT_HOST_RTS_PIN, level);
}


_ATTR_BLUETOOTHCONTROL_CODE_
void rtk8761a_send_go_to_sleep_ack(void)
{

}

_ATTR_BLUETOOTHCONTROL_CODE_
void rtk8761a_send_wake_up_ind(void)
{

}

_ATTR_BLUETOOTHCONTROL_CODE_
void rtk8761a_send_wake_up_ack(void)
{

}


_ATTR_BLUETOOTHCONTROL_CODE_
void rtk8761a_wake_up_int()
{

}

/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwPowerOn(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping        2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwPowerOn(void * config)
{
    int ret = RETURN_OK;
    int retry = 3;
power_up:
    if(rtk8761a_power_up() == RETURN_FAIL)
    {
        return RETURN_FAIL;
    }
    BT_UARTInit(BT_UART_CH,24*1000*1000,115200,UART_DATA_8B,UART_ONE_STOPBIT,UART_EVEN_PARITY);
    DelayMs(50);
    phybusif_init(&rtk8761a_hw_control);
    bt_init(0);

    rtk8761a_set_host_rts_leave(GPIO_LOW);


    ret = Rtk8761aHwDownInitScript();
    if(ret != RETURN_OK)
    {
        BT_DEBUG("ret=%d\n", ret);
        if(retry--)
        {
            rtk8761a_power_down();
            goto power_up;
        }
        else
        {
            return ret;
        }

    }

    hci_cmd_complete(rtk8761a_init_complete);
    Rtk8761aHwReset();
    wait_rtk8761a_cmd_complete(50000);


    rtk8761apcb.PowerStatus = POWER_STATUS_ON;

    return ret;

}


/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwPowerOff(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping         2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwPowerOff(void * config)
{
    rtk8761a_power_down();
    UARTSetIntDisabled(BT_HCI_UART_ID, UART_IE_RX);
    UARTDeInit(BT_UART_CH); //chad.ma add 20160921
    rtk8761apcb.PowerStatus == POWER_STATUS_OFF;
}

/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwReset(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwReset(void)
{
    hci_reset();
}

/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwInit(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwInit(void)
{
    memset(&rtk8761apcb, 0, sizeof(_rtk8761aPcb_));

    return RETURN_OK;

}
/*******************************************************************************
 * Function :  Rtk8761aDownload_patchcode
 * Descr    :  download rtk8761a pathch code
 *
 * Input    :
 *
 * Output   :
 *
 * Return   : index
 *
 * Author   : MLC
 * Others   : 2014-2-25
 * Notes    : Download realtek patch code and config file together.
 *
 *******************************************************************************/
_ATTR_BLUETOOTHCONTROL_CODE_
int Rtk8761aChange_BaudRate()
{
    struct pbuf *p;

    uint8 patchcode_15M[4] = {0x02, 0x80, 0x92, 0x04};
    uint8 patchcode_9216[4] = {0x04, 0x50, 0xF7, 0x05};
    //uint8 patchcode[4] = {0x14, 0xC0, 0x52, 0x02};
    if((p = pbuf_alloc(PBUF_RAW, 8, PBUF_RAM)) == NULL)
    {
        //LWIP_DEBUGF(HCI_DEBUG, ("hci_download_patchcode: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }

    /* Assembling command packet */
    ((uint8 *)p->payload)[0] = 0x01;
    ((uint8 *)p->payload)[1] = 0x17;
    ((uint8 *)p->payload)[2] = 0xfc;

    /// Set data struct.
    ((uint8 *)p->payload)[3] = 0x04;//length
    memcpy(&((uint8 *)p->payload)[4], patchcode_15M, 4);

    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return 1;
}
/*******************************************************************************
 * Function :  Rtk8761aDownload_patchcode
 * Descr    :  download rtk8761a pathch code
 *
 * Input    :
 *
 * Output   :
 *
 * Return   : index
 *
 * Author   : MLC
 * Others   : 2014-2-25
 * Notes    : Download realtek patch code and config file together.
 *
 *******************************************************************************/
_ATTR_BLUETOOTHCONTROL_CODE_
int Rtk8761aDownload_patchcode(uint8* patchcode,int patch_len, int index)
{
    struct pbuf *p;

    int cur_index = index;

    if((p = pbuf_alloc(PBUF_RAW, 3 + 2 + patch_len, PBUF_RAM)) == NULL)
    {
        //LWIP_DEBUGF(HCI_DEBUG, ("hci_download_patchcode: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }

    /* Assembling command packet */
    ((uint8 *)p->payload)[0] = 0x01;
    ((uint8 *)p->payload)[1] = HCI_VENDER_DOWNLOAD_OCF;
    ((uint8 *)p->payload)[2] = 0xfc;

    /// Set data struct.
    ((uint8 *)p->payload)[3] = patch_len + 1;//add index
    ((uint8 *)p->payload)[4] = cur_index;

    if (NULL != patchcode)
    {
        memcpy(&((uint8 *)p->payload)[5], patchcode, patch_len);
    }
    /* Assembling cmd prameters */

    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return cur_index;
}


static void Set_BDAdress(uint8* des,uint8* bd_add,uint8 *cur_len)
{
    int i;
    int len,pos;
    uint16 data_len = 0;

    if( des == NULL|| bd_add == NULL)
        return;

    pos = -1;
    len = *cur_len;

    for(i = 0; i < len; i++)
    {
        //config file's signature 0x55AB2387
        if( des[i] == 0x55
            && des[i+1] == 0xAB
            && des[i+2] == 0x23
            && des[i+3] == 0x87
          )
        {
            pos = i;        //config file is in last loadbuf data.
            break;
        }
    }

    if( pos == -1 ) //means not found config file start signature in this buffer .
    {
        BT_DEBUG("Not found config file signature,need to get more data.");
    }
    else    //we get the config file's position in last load buffer data.
    {
        data_len = (des[pos + 4] | des[pos + 5] << 8);

        data_len += 9;
        des[pos + 4] = data_len &  0x00ff;
        des[pos + 5] = data_len >> 8 & 0xff;

        if(len + 9 < PATCH_DATA_FIELD_MAX_SIZE ) //not exceed the patch code maximum data size
        {
            des[len]     = 0x3C;    //offset of BT Efuse.
            des[len + 1] = 0x00;
            des[len + 2] = 0x06;    //length of BDAdrr.
            memcpy(&des[len + 3],bd_add,sizeof(bd_add));

            *cur_len = len + 9;     //add 9 bytes bdaddr data not
        }
    }

}


/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwDownInitScript(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwDownInitScript(void)
{
    int offset = 0,i = 0;

    int downcnt;
    int ret;
    unsigned char *data = NULL;
    uint32 loadAddr;
    unsigned char loadbuf[252];
    int finish_flag = 0;
    uint8 iCurIndex = 0;
    uint8 iCurLen = 0;
    uint32 patchcodeLen;
    uint32 configFileLen = 0;   //size of config file
    int macSplitFlag =0; /*ÿ´ú±ímac·Ö²ð·ÖÔÚÁ½¸ö°üÖÐ*/
    int macleftcnt = 0;
    uint32 script_module_id = MODULE_ID_INIT_SCRIPT;

    configFileLen = sizeof( rtk_config_data );

    GetBeepSourceInf(script_module_id, &loadAddr, &patchcodeLen);

    ModuleOverlayLoadData(loadAddr + offset, (uint32)loadbuf, PATCH_DATA_FIELD_MAX_SIZE);

    data = loadbuf;
    downcnt = 0;

    //    if((data[0] != 0x68)
    //    &&(data[1] != 0x63)
    //    &&(data[2] != 0x65)
    //    &&(data[3] != 0x74)
    //    &&(data[4] != 0x6C)
    //    &&(data[5] != 0x61)
    //    &&(data[6] != 0x65)
    //    &&(data[7] != 0x52)
    //          )
    //    {
    // return;
    //    }

    hci_cmd_complete(rtk8761a_init_complete);

    Rtk8761aChange_BaudRate();
    ret = wait_rtk8761a_cmd_complete(2000000);
    if(ret == TIMEOUT && !finish_flag)
    {
        BT_DEBUG("wait_rtk8761a_cmd_complete timeout\n");
        return TIMEOUT;
    }
    else
    {
        DelayMs(300);
        UARTInit(BT_UART_CH,1500000,UART_DATA_8B,UART_ONE_STOPBIT,UART_EVEN_PARITY);
        UARTSetIntEnabled(BT_HCI_UART_ID, UART_IE_RX);
    }

    while(downcnt  < patchcodeLen)
    {
        if(downcnt + PATCH_DATA_FIELD_MAX_SIZE >  patchcodeLen)

        {
            iCurIndex = iCurIndex | 0x80;
            //iCurLen = patchcodeLen - downcnt - 1;   //last packet length
            iCurLen = patchcodeLen - downcnt;   //last packet length
#ifdef BT_ENABLE_SET_MAC
            if(macSplitFlag)
            {
                for(i=0; i++; i< macleftcnt)
                {
                    loadbuf[i] = rtk8761apcb.bdaddr.addr[BD_ADDR_LEN-macleftcnt+i];
                }
            }
            else
            {
                for(i=0; i<BD_ADDR_LEN; i++)
                {
                    loadbuf[iCurLen-1-i] = rtk8761apcb.bdaddr.addr[BD_ADDR_LEN-1-i];
                }
            }

#endif
        }
        else if(downcnt + PATCH_DATA_FIELD_MAX_SIZE == patchcodeLen)
        {
            iCurIndex = iCurIndex | 0x80;
            iCurLen = PATCH_DATA_FIELD_MAX_SIZE;    //last packet length

#ifdef BT_ENABLE_SET_MAC
            {

                for(i=0; i++; i< BD_ADDR_LEN)
                {
                    loadbuf[PATCH_DATA_FIELD_MAX_SIZE-1-i] = rtk8761apcb.bdaddr.addr[BD_ADDR_LEN-1-i];
                }

            }

#endif
        }
        else
        {
            iCurIndex = iCurIndex & 0x7F;
            iCurLen = PATCH_DATA_FIELD_MAX_SIZE;
            if(patchcodeLen-(downcnt+PATCH_DATA_FIELD_MAX_SIZE) < PATCH_DATA_FIELD_MAX_SIZE)
            {
                /*×îºóÒ»¸ö°ü,±£´æµ¹Êý  µÚ¶þ°ü×îºó6¸ö×Ö½Ú*/
#ifdef BT_ENABLE_SET_MAC
                macleftcnt = patchcodeLen-(downcnt+PATCH_DATA_FIELD_MAX_SIZE);

                if(macleftcnt<6)
                {
                    for(i=0; i<6-macleftcnt; i++)
                        loadbuf[PATCH_DATA_FIELD_MAX_SIZE-1-i] = rtk8761apcb.bdaddr.addr[BD_ADDR_LEN-macleftcnt-1-i];

                    macSplitFlag =1; /*ÐèÒªÔÚÏÂÒ»¸ö°üÖÐ²¹ÉÏ ÆäËûµÄmac*/
                }


#endif
            }
        }

        if( (iCurIndex & 0x80) && configFileLen <= patchcodeLen - downcnt)
        {
            //last loaded buffer include the whole all config data.
            if(rtk8761apcb.needChangeMac)
            {
                //change the bt chip mac addr
            }
            finish_flag = 1;
        }
        else
        {
            ///Other conditions may be complex.
        }

        Rtk8761aDownload_patchcode(data, iCurLen,iCurIndex );
        if(finish_flag)
        {
            DelayMs(100);
            UARTInit(BT_UART_CH,1500000,UART_DATA_8B,UART_ONE_STOPBIT,UART_EVEN_PARITY);
            UARTSetIntEnabled(BT_HCI_UART_ID, UART_IE_RX);

        }
        ret = wait_rtk8761a_cmd_complete(2000000);
        if(ret == TIMEOUT && !finish_flag)
        {
            return TIMEOUT;
        }
        if( iCurIndex & 0x80 )  //patch code & configuration file have download successed
            break;

        downcnt += iCurLen;
        offset += iCurLen;
        ModuleOverlayLoadData(loadAddr + offset, (uint32)(loadbuf), PATCH_DATA_FIELD_MAX_SIZE);
        iCurIndex ++;
    }

    DelayMs(400);
    hci_cmd_complete(NULL);

    return RETURN_OK;
}


/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwSleepEnable(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwSleepEnable(void * config)
{

    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwSleepDisable(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwSleepDisable(void * config)
{

    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwWakeup(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwWakeup(void * config)
{
    rtk8761a_set_host_rts_leave(GPIO_LOW);
    rtk8761a_send_wake_up_ind();

    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwGetPowerStatus(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwGetPowerStatus(void)
{
    return rtk8761apcb.PowerStatus;
}


/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwSetChipBaudrate(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwSetChipBaudrate(uint baudrate)
{
    return RETURN_OK;
}


/*
--------------------------------------------------------------------------------
  Function name :  int BlueToothHwSetDevMac(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwSetDevMac(struct bd_addr *bdaddr)
{
    rtk8761apcb.needChangeMac = TRUE;
    memcpy(rtk8761apcb.bdaddr.addr,bdaddr->addr, 6);
    return RETURN_OK;
}


/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwRegisterPowerNotification(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwRegisterPowerNotification(int(*func)(POWER_NOTIFICATION_t event))
{
    rtk8761apcb.power_event_notify = func;
    return RETURN_OK;
}



/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwRegisterHwError(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwRegisterHwError(int(*func)(POWER_NOTIFICATION_t event))
{
    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name :  int BlueToothHwSetDevMac(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwSetUartRecAbility(UART_ABILITY_t Ability)
{
    if(Ability == UART_ABILITY_RECV_ENABLE)
    {
        IntEnable(BT_UART_INT_ID);
        rtk8761a_set_host_rts_leave(GPIO_LOW);
    }
    else if(Ability == UART_ABILITY_RECV_DISABLE)
    {
        rtk8761a_set_host_rts_leave(GPIO_HIGH);
        IntDisable(BT_UART_INT_ID);
    }

    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwHwService(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwService()
{
    return RETURN_OK;
}


/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwHwService(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwVendorRecv(uint8 *data ,uint len)
{
    uint8 c;

    c = *data;

    switch(c)
    {


        default:
            break;
    }

    return RETURN_OK;

}


bt_hw_control_t rtk8761a_hw_control =
{
    Rtk8761aHwInit,
    Rtk8761aHwPowerOn,
    Rtk8761aHwPowerOff,
    NULL,
    NULL,
    NULL,
    NULL,
    Rtk8761aHwGetPowerStatus,
    NULL,
    Rtk8761aHwRegisterPowerNotification,
    Rtk8761aHwSetUartRecAbility,
    Rtk8761aHwSetDevMac,
    NULL,
    NULL

};

#endif    //(BT_CHIP_CONFIG == BT_CHIP_RTL8761)


#endif
