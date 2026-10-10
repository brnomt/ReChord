#include "SysInclude.h"

#ifdef  _BLUETOOTH_
#include "audio_file_access.h"

#include "BlueToothControl.h"
//#include "Btbuffer.h"
#include "AudioControl.h"
#include "MediaBroWin.h"

//#include "btVoiceNotifyProc.h"
#include "bd_addr.h"

#include "BlueToothSysParam.h"
#include "rk_bt_Api.h"

int32 BlueToothHwSetDevMac(struct bd_addr *bdaddr);
_ATTR_BLUETOOTHCONTROL_BSS_ bt_hw_control_t *g_bt_hw_control;

/*
--------------------------------------------------------------------------------
  Function name :  int BlueToothPowerOn(void )
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
int32 BlueToothHwPowerOn(void * config)
{
    POWER_ON_CONFIG * powerConfig;
    powerConfig = config;
    //DEBUG("Enter BlueToothHwPowerOn\n");
    if(g_bt_hw_control)
    {
        if(powerConfig->enable_dev_mac)
        {
            BlueToothHwSetDevMac(&powerConfig->bdaddr);
        }
        //lwbt_memp_init();
        if(g_bt_hw_control->on)
        {
            g_bt_hw_control->on(config);
        }
        if(powerConfig->enable_dev_name)
        {
            bt_set_local_name(powerConfig->dev_name, sizeof(powerConfig->dev_name));
        }

        if(powerConfig->enable_pin_code)
        {
            bt_set_pin_code(powerConfig->pin_code, 4);
        }
        bt_init(powerConfig->bt_init_flag |ENABLE_CMD_COMPLETE);

#ifdef ENABLE_BQB_RF_TEST
        hci_enable_device_under_test_mode_command();
        DelayMs(50);
#endif
        bt_start(BT_START_FLAG_DEFAULT);

#ifdef _A2DP_SINK_
        bt_wait_hci_cmd_init_complete();
#endif
    }
    //DEBUG("Leaving BlueToothHwPowerOn\n");
    return RETURN_OK;
}



/*
--------------------------------------------------------------------------------
  Function name :  int BlueToothPowerOff(void )
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
int32 BlueToothHwPowerOff(void * config)
{
    if(g_bt_hw_control)
    {
        if(g_bt_hw_control->off)
        {
            g_bt_hw_control->off(config);
        }
    }

    bt_deinit();
    return RETURN_OK;
}


/*
--------------------------------------------------------------------------------
  Function name :  int BlueToothReset(void )
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
int32 BlueToothHwInit(bt_hw_control_t *bt_hw_control)
{
    g_bt_hw_control = bt_hw_control;
    if(g_bt_hw_control->init)
    {
        return g_bt_hw_control->init();
    }

}


/*
--------------------------------------------------------------------------------
  Function name :  int BlueToothHwSleepEnable(void )
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
int32 BlueToothHwSleepEnable(void *config)
{
    if(g_bt_hw_control)
    {
        if(g_bt_hw_control->sleep_enable)
        {
            g_bt_hw_control->sleep_enable(config);
        }
    }
    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name :  int BlueToothHwSleepDisable(void )
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
int32 BlueToothHwSleepDisable(void *config)
{
    if(g_bt_hw_control)
    {
        if(g_bt_hw_control->sleep_disable)
        {
            g_bt_hw_control->sleep_disable(config);
        }
    }
    return RETURN_OK;
}


/*
--------------------------------------------------------------------------------
  Function name :  int BlueToothHwWakeup(void )
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
int32 BlueToothHwWakeup(void *config)
{
    if(g_bt_hw_control)
    {
        if(g_bt_hw_control->wake)
        {
            g_bt_hw_control->wake(config);
        }
    }
    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name :  int BlueToothHwGetPowerStatus(void )
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
int32 BlueToothHwGetPowerStatus(void)
{
    if(g_bt_hw_control)
    {
        if(g_bt_hw_control->get_power_status)
        {
            return g_bt_hw_control->get_power_status();
        }
    }
    return POWER_STATUS_ON;
}


/*
--------------------------------------------------------------------------------
  Function name :  int BlueToothHwSetChipBaudrate(void )
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
int32 BlueToothHwSetChipBaudrate(uint baudrate)
{

    if(g_bt_hw_control)
    {
        if(g_bt_hw_control->set_baudrate)
        {
            g_bt_hw_control->set_baudrate(baudrate);
        }
    }
    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name :  int BlueToothRegisterPowerNotification(void )
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
int32 BlueToothHwRegisterPowerNotification(int(*func)(POWER_NOTIFICATION_t event))
{

    if(g_bt_hw_control)
    {
        if(g_bt_hw_control->power_event_notifications)
        {
            g_bt_hw_control->power_event_notifications(func);
        }
    }

    return RETURN_OK;
}



/*
--------------------------------------------------------------------------------
  Function name :  int BlueToothRegisterPowerNotification(void )
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
int32 BlueToothHwRegisterHwError(int(*func)(POWER_NOTIFICATION_t event))
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
int32 BlueToothHwSetDevMac(struct bd_addr *bdaddr)
{

    if(g_bt_hw_control)
    {
        if(g_bt_hw_control->set_mac_addr)
        {
            g_bt_hw_control->set_mac_addr(bdaddr);
        }
    }
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
int32 BlueToothHwSetUartRecAbility(UART_ABILITY_t Ability)
{
    if(g_bt_hw_control)
    {
        if(g_bt_hw_control->set_uart_recv_ablity)
        {
            g_bt_hw_control->set_uart_recv_ablity(Ability);
        }
    }

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
int32 BlueToothHwService()
{
    if(g_bt_hw_control)
    {
        if(g_bt_hw_control->hw_service)
        {
            g_bt_hw_control->hw_service();
        }
    }

    return RETURN_OK;

}

#endif



