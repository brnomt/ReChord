
/*-----------------------------------------------------------------------------------*/
/* bt_ip_dt.c
*
* This is a control application that initialises a host controller and connects to a
* network as a DT through a DUN or LAP enabled device.
*/
/*-----------------------------------------------------------------------------------*/
#define _IN_BT_DT_IP
#include "lwbt.h"
#include "SysInclude.h"
#include "phybusif.h"
#include "lwbtdebug.h"
#include "lwbt_memp.h"
#include "hci.h"
#include "l2cap.h"
#include "sdp.h"
#include "pbuf.h"
#include "avdtp_source.h"
#include "avctp_source.h"
#include "avrcp_source.h"
#include "bt_ip_dt_source.h"

#include "BlueToothSysParam.h"

#ifdef _BLUETOOTH_
#ifdef _A2DP_SOUCRE_

#define BT_IP_DEBUG DBG_ON /* Control debug messages */

#define ENABLE_SSP
//#define ENABLE_FAST_DISCOVERABLE
#define ENABLE_DISCOVERABLE
#define ENABLE_EXTENDED_INQUIRY
#define ENABLE_BLE_MAC_SET
#define ENABLE_GET_DEV_PROFILE_VERSIONS
enum bt_profile
{
    LAP_PROFILE, DUN_PROFILE
};

/* Forward declarations */
err_t rfcomm_connected(void *arg, struct rfcomm_pcb *tpcb, err_t err);
err_t l2cap_connected(void *arg, struct l2cap_pcb *pcb, uint16 result, uint16 status);
err_t inquiry_complete(void *arg, struct hci_pcb *pcb, struct hci_inq_res *ires, uint16 result, uint16 complete);
err_t command_complete_for_init(void *arg, struct hci_pcb *pcb, uint8 ogf, uint8 ocf, uint8 result);
err_t command_complete(void *arg, struct hci_pcb *pcb, uint8 ogf, uint8 ocf, uint8 result);
err_t pin_req(void *arg, struct bd_addr *bdaddr);
err_t link_key_not(void *arg, struct bd_addr *bdaddr, uint8 *key);
err_t link_key_requset(void *arg, struct bd_addr *bdaddr);
err_t acl_conn_complete(void *arg, struct bd_addr *bdaddr);
err_t req_remote_name_complete(void *arg, struct hci_pcb *pcb,uint8 scan_status, struct bd_addr *bdaddr, uint8 * name, struct hci_inq_res *ires_header);

void auth_complete(uint8 result);
int bt_disconnect(struct bd_addr *bdaddr);
err_t read_bdaddr_complete(void *arg, struct bd_addr *bdaddr);
void user_confirmation_request(struct bd_addr *bdaddr, uint32 Numeric_Value);
typedef enum
{
    SCAN_IDLE,
    SCAN_WORKING,
    SCAN_COMPLETE,
    SCAN_REQ_NAME_COMPLETE,
} SCAN_STATE;


/*-----------------------------------------------------------------------------------*/
/*
* bt_start():
*
* Called by the main application to initialize and connect to a network
*
*/
/*-----------------------------------------------------------------------------------*/
#define INQUIRY_RETRY_MAX 5

_ATTR_LWBT_DATA_ uint systickprev = 0;
_ATTR_LWBT_DATA_ uint systickprev_t = 0;

#define BT_LOCAL_NAME_MAX_LEN 30
_ATTR_LWBT_DATA_ char bt_local_name[BT_LOCAL_NAME_MAX_LEN+2] = {0};
_ATTR_LWBT_DATA_ char bt_pin_code[5] = {"0000"};
_ATTR_LWBT_BSS_ static uint32 bt_flag;

_ATTR_LWBT_BSS_ static void(*scan_result)(struct hci_inq_res *ires, uint16 devCnt, int isComplete);
_ATTR_LWBT_BSS_ static void(*connect_result)(int result);
_ATTR_LWBT_BSS_ static void(*bt_init_complete)(void);
_ATTR_LWBT_DATA_ uint8 is_find_a2dp = 0;
_ATTR_LWBT_DATA_ uint8 is_stop_scan = 0;
_ATTR_LWBT_DATA_ uint8 is_sdp_disconnet = 0;
_ATTR_LWBT_BSS_  uint  sdp_disconnet_timer_pos;
_ATTR_LWBT_DATA_ uint8 scan_state = 0;
_ATTR_LWBT_BSS_  uint8 isAuth_complete;
_ATTR_LWBT_BSS_  uint8 pinCode_req_retry;
_ATTR_LWBT_BSS_  uint8 is_command_complete;
_ATTR_LWBT_BSS_  uint8 command_result;
_ATTR_LWBT_BSS_  uint16 g_remote_a2dp_version;
_ATTR_LWBT_BSS_  uint16 g_remote_avrcp_version;
_ATTR_LWBT_BSS_  uint32 g_remote_numeric_value;
#ifdef HAVE_BLE
_ATTR_LWBT_BSS_ uint8 ble_scan_response_data[31];
_ATTR_LWBT_BSS_ uint8 ble_advertising_data[31];
#endif

extern  struct hci_pcb *pcb;
_ATTR_LWBT_INIT_CODE_
err_t bt_wait_command_commplete(int timeout_ms)
{

    while(timeout_ms--)
    {
        if(is_command_complete)
        {
            is_command_complete = 0;
            if(command_result)
            {
                return command_result;
            }
            return ERR_OK;
        }

        DelayMs(1);
    }
    return ERR_TIMEOUT;


}

_ATTR_LWBT_INIT_CODE_
void bt_set_init_complete_hook(void(*bt_init_complete_hook)(void))
{
    bt_init_complete = bt_init_complete_hook;
}


_ATTR_LWBT_INIT_CODE_
void bt_start(uint32 flag)
{
    //uint8 cod_lap_dun[] = {0x00,0x04,0x24,0x00,0xFF,0xFF};
    uint8 cod_lap_dun[] = {0x00,0x00,0x00,0x00,0x00,0x00};
    scan_result = NULL;
    pinCode_req_retry = 0;
    //DEBUG("Enter bt_start\n");
    DelayMs(100);
    systickprev = GetSysTick();
    if(flag & SEND_HCI_RESET)
    {
        int ret;
        int retry = 10;
        do
        {
            hci_reset();
            ret = bt_wait_command_commplete(50);
            retry--;
        }while(ret && retry);
    }
    else
    {
        int ret;
        int retry = 10;
        do
        {
            hci_read_buffer_size();
            ret = bt_wait_command_commplete(50);
            retry--;
        }while(ret && retry);

    }
    if(flag & SEND_HCI_RESET)
    {
        int ret;
        int retry = 10;
        do
        {
            hci_read_buffer_size();
            ret = bt_wait_command_commplete(50);
            retry--;
        }while(ret && retry);
    }
#ifdef HAVE_BLE
    hci_set_event_mask(0x3FFFFFFFFFFFFFFF);
#else
    hci_set_event_mask(0x1FFFFFFFFFFFFFFF);
#endif
    bt_wait_command_commplete(50);
    //hci_write_default_link_policy_setting(0);
#ifdef BT_HOST_SNIFF
    hci_write_default_link_policy_setting(POLICY_SETTING_ENABLE_SNIFF_MODE);
#else
    hci_write_default_link_policy_setting(POLICY_SETTING_ENABLE_SNIFF_MODE);
#endif
    bt_wait_command_commplete(50);
    hci_read_bd_addr(read_bdaddr_complete);
    bt_wait_command_commplete(50);
    hci_set_event_filter(0x01, 0x01, cod_lap_dun); /* Report only devices with a specific type of CoD */
    bt_wait_command_commplete(50);
    hci_write_page_timeout(0x6000); /* 10.24s */
    {
        char cod[4]= {0x1C, 0x04, 0x28,0x00}; //src
        hci_write_cod(cod);
        bt_wait_command_commplete(50);
    }
    hci_change_local_name(bt_local_name, BT_LOCAL_NAME_MAX_LEN);
    bt_wait_command_commplete(50);
#ifdef ENABLE_EXTENDED_INQUIRY
    hci_write_inquiry_mode_command(0x02);
    bt_wait_command_commplete(50);
#endif
#ifdef ENABLE_SSP
    hci_write_simple_pairing_mode(0x01);
    bt_wait_command_commplete(50);
#endif
#ifdef ENABLE_FAST_DISCOVERABLE
    hci_write_inquiry_scan_activity(100, 30);
    bt_wait_command_commplete(50);
    hci_write_page_scan_activity(100, 30);
    bt_wait_command_commplete(50);
#endif

#ifdef HAVE_BLE

    hci_write_le_host_supported_command(1,0);
    bt_wait_command_commplete(50);
    hci_le_read_buffer_size_command();;
    bt_wait_command_commplete(50);
    {
        int datalen = 0;
        int namelen = 0;
        int flaglen = 3;
        memset(ble_advertising_data, 0, 31);
        namelen = strlen(bt_local_name);
        ble_advertising_data[0] = 2;
        ble_advertising_data[1] = 1; //ad flag
        ble_advertising_data[2] = 0x0A; //LE General Discoverable Mode & Simultaneous LE and BR/EDR
        datalen += 3;
        ble_advertising_data[0+datalen] = 3;
        ble_advertising_data[1+datalen] = 0x02;
        ble_advertising_data[2+datalen] = 0x01;
        ble_advertising_data[3+datalen] = 0x18;
#if 1
        ble_advertising_data[0+datalen] = namelen+1;
        ble_advertising_data[1+datalen] = 0x09; //
        memcpy(&ble_advertising_data[2+datalen],bt_local_name, namelen);
        datalen += namelen+2;
#else
        ble_advertising_data[0+datalen] = namelen+1+3;
        ble_advertising_data[1+datalen] = 0x09; //
        memcpy(&ble_advertising_data[2+datalen],bt_local_name, namelen);
        datalen += namelen+2;
        ble_advertising_data[0+datalen] = '_';
        ble_advertising_data[1+datalen] = 'L';
        ble_advertising_data[2+datalen] = 'E';
        datalen += 3;
#endif
        hci_le_set_response_data_command(datalen, ble_advertising_data);
        bt_wait_command_commplete(50);
    }

    {
        int datalen = 0;
        int namelen = 0;
        int flaglen = 3;
        memset(ble_advertising_data, 0, 31);
        namelen = strlen(bt_local_name);
        ble_advertising_data[0] = 2;
        ble_advertising_data[1] = 1; //ad flag
        ble_advertising_data[2] = 0x0A; //LE General Discoverable Mode & Simultaneous LE and BR/EDR
        datalen += 3;
        ble_advertising_data[0+datalen] = 3;
        ble_advertising_data[1+datalen] = 0x02;
        ble_advertising_data[2+datalen] = 0x01;
        ble_advertising_data[3+datalen] = 0x18;
        datalen += 4;
//            ble_advertising_data[0+datalen] = 4;
//            ble_advertising_data[1+datalen] = 0x0D;
//            ble_advertising_data[2+datalen] = 0x04;
//            ble_advertising_data[3+datalen] = 0x04;
//            ble_advertising_data[4+datalen] = 0x24;
//            datalen += 5;
#if 1
        ble_advertising_data[0+datalen] = namelen+1;
        ble_advertising_data[1+datalen] = 0x09; //
        memcpy(&ble_advertising_data[2+datalen],bt_local_name, namelen);
        datalen += namelen+2;
#else
        ble_advertising_data[0+datalen] = namelen+1+3;
        ble_advertising_data[1+datalen] = 0x09; //
        memcpy(&ble_advertising_data[2+datalen],bt_local_name, namelen);
        datalen += namelen+2;
        ble_advertising_data[0+datalen] = '_';
        ble_advertising_data[1+datalen] = 'L';
        ble_advertising_data[2+datalen] = 'E';
        datalen += 3;
#endif
        hci_le_set_advertising_data_command(datalen, ble_advertising_data);
        bt_wait_command_commplete(50);
    }


    {
        struct bd_addr Direct_Address;
        memset(&Direct_Address, 0, 6);
#ifdef ENABLE_BLE_MAC_SET
        hci_le_advertising_parameters_command(0x100,0x300,0,0x1,0x0, &Direct_Address, 7, 0);//anroid le enable
#else
        hci_le_advertising_parameters_command(0x100,0x300,0,0x0,0x1, &Direct_Address, 7, 0);//anroid le disable
#endif
        bt_wait_command_commplete(50);
    }

#ifdef ENABLE_BLE_MAC_SET

    {
        uint8 bt_le_mac[6];

        bt_le_mac[0] = 0xAA;
        bt_le_mac[1] = 0xBB;
        bt_le_mac[2] = 0xCC;
        bt_le_mac[3] = 0xDD;
        bt_le_mac[4] = 0xEE;
        bt_le_mac[5] = 0xEE;


        //memcpy(bt_le_mac, g_bt_mac, sizeof(g_bt_mac));

        //bt_le_mac[0] += 1;

        BT_DEBUG("\n ble:%02x-%02x-%02x-%02x-%02x-%02x\n", \
               bt_le_mac[0], bt_le_mac[1],bt_le_mac[2], bt_le_mac[3], bt_le_mac[4], bt_le_mac[5]);

        hci_le_set_random_address_command((struct bd_addr*)bt_le_mac);
        bt_wait_command_commplete(50);
    }

#endif
    hci_le_clear_white_list_command();
    bt_wait_command_commplete(50);

    hci_le_set_advertise_enable_command(1);
    bt_wait_command_commplete(50);
#endif

    hci_write_scan_enable(0x03);
    bt_wait_command_commplete(50);
    hci_cmd_complete(NULL); /* Initialization done, don't come back */
    if(bt_init_complete != NULL)
    {
        bt_init_complete();
    }
    hci_cmd_complete(command_complete); /* Initialization done, don't come back */
    //DEBUG("Leaving bt_start\n");
}


_ATTR_LWBT_INIT_CODE_
void bt_restart(void)
{
    pinCode_req_retry = 0;

    systickprev = GetSysTick();
    hci_reset_all();
    l2cap_reset_all();
    sdp_reset_all();
    if(bt_flag & ENABLE_A2DP)
    {
        avdtp_reset_all();
        avctp_reset_all();
    }

    hci_cmd_complete(NULL);
    hci_pin_req(pin_req);
    hci_link_key_not(link_key_not);
    hci_link_key_request(link_key_requset);
    hci_connection_complete(acl_conn_complete);
    hci_req_remote_name_complete(req_remote_name_complete);
    hci_auth_complete(auth_complete);
}


_ATTR_LWBT_CODE_
void bt_time_serve(void)
{
    uint systicknow;

    uint8 *devMac;
    systicknow = GetSysTick();

    if(systicknow >= systickprev+100)
    {
        systickprev = systicknow;

        l2cap_tmr();
    }

    if(is_sdp_disconnet)
    {
        devMac = get_last_connect_dev();
        if(isAuth_complete == 1)
        {
            is_sdp_disconnet = 0;
            isAuth_complete = 0;
            avdtp_connect((struct bd_addr *)devMac, connect_result);

            sdp_disconnet_timer_pos = GetSysTick();
        }
        else if(isAuth_complete == 0xFF)
        {
            is_sdp_disconnet = 0;
            isAuth_complete = 0;
            sdp_disconnet_timer_pos = GetSysTick();
            bt_disconnect((struct bd_addr *)devMac);

            if(connect_result)
            {
                connect_result(RETURN_FAIL);
            }
        }

        if(systicknow > sdp_disconnet_timer_pos+1000)
        {
            is_sdp_disconnet = 0;
            isAuth_complete = 0;
            sdp_disconnet_timer_pos = GetSysTick();
            bt_disconnect((struct bd_addr *)devMac);

            if(connect_result)
            {
                connect_result(RETURN_FAIL);
            }
        }
    }


    hci_tmr();
    avdtp_tmr();
#ifdef BT_HOST_SNIFF
    h5_hci_link_mode_manage();
#endif
}

_ATTR_LWBT_INIT_CODE_
void bt_init(uint32 flag)
{
//    inquiry_retry_time = 0;
    bt_flag = flag;
    //DEBUG("Enter bt_init\n");
    systickprev = GetSysTick();
    bt_debug_components = BT_BDBUG_COMPONENTS;
    bt_debug_level = _DBG_INFO_;
    lwbt_memp_init();
    hci_init();
    l2cap_init();
    sdp_init();
    if(flag & ENABLE_A2DP)
    {
        avdtp_init();
        avctp_init();
    }
    if(flag & ENABLE_CMD_COMPLETE)
    {
        hci_cmd_complete(command_complete_for_init);
    }
    else
    {
        hci_cmd_complete(NULL);
    }
#ifdef HAVE_BLE
    setup();
#endif
    hci_pin_req(pin_req);
    hci_link_key_not(link_key_not);
    hci_link_key_request(link_key_requset);
    hci_connection_complete(acl_conn_complete);
    hci_req_remote_name_complete(req_remote_name_complete);
    hci_auth_complete(auth_complete);
    hci_user_confirmation_request(user_confirmation_request);

    hci_set_io_capability(HCI_IO_CAPABILITY_NOINPUTNOOUTPUT);
    //DEBUG("Leaving bt_init\n");
    LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("bt_init\n"));
}

_ATTR_LWBT_INIT_CODE_
void bt_deinit(void)
{
    hci_deinit();
    l2cap_deinit();
    sdp_deinit();
    avctp_deinit();
    avdtp_deinit();
}

_ATTR_LWBT_INIT_CODE_
void bt_set_pin_code(char * pincode, int len)
{
    memset(bt_pin_code, 0, 5);
    memcpy(bt_pin_code, pincode, 4);
}

_ATTR_LWBT_CODE_
void bt_discoverable_enable(void)
{
    hci_write_scan_enable(0x03);
}

_ATTR_LWBT_CODE_
void bt_discoverable_disable(void)
{
    hci_write_scan_enable(0x00);

}



_ATTR_LWBT_INIT_CODE_
void bt_set_local_name(char * name, int namelen)
{
    int len;
    memset(bt_local_name, 0, BT_LOCAL_NAME_MAX_LEN+2);

    if(namelen > BT_LOCAL_NAME_MAX_LEN)
    {
        len = BT_LOCAL_NAME_MAX_LEN;
    }
    else
    {
        len = namelen;
    }

    memcpy(bt_local_name, name, len);
}

_ATTR_LWBT_CODE_
int bt_disconnect(struct bd_addr *bdaddr)
{
    return hci_linkdown(bdaddr, HCI_OTHER_END_TERMINATED_CONN_USER_ENDED, HCI_CONNECT_TYPE_ACL);
}

_ATTR_LWBT_CODE_
void bt_scan(void (* scan_result_hook)(struct hci_inq_res *ires, uint16 devCnt, int isComplete))
{
    if(scan_state == SCAN_WORKING)
    {
        return;
    }
    hci_inquiry(0x009E8B33, 0x04, 0x00, inquiry_complete); // 0x04 = 4*1.28s,  0x00 indicate that always scan before time out
    scan_result = scan_result_hook;
    is_stop_scan = 0;
    scan_state = SCAN_WORKING;

}

_ATTR_LWBT_CODE_
void bt_clean_scan_result(void)
{
    hci_clean_scan_result();
}

_ATTR_LWBT_CODE_
void bt_scan_cancel(void)
{
    if(scan_state == SCAN_WORKING)
    {
        hci_inquiry_cancel();
    }
    DelayMs(50);
    is_stop_scan = 1;
    if(pcb->iresAll)
    {
        hci_remote_name_request_cancle(&pcb->iresAll->bdaddr);
    }
    DelayMs(50);
    scan_state = SCAN_IDLE;
}


_ATTR_LWBT_DATA_
struct bt_state
{
    struct bd_addr bdaddr;
    uint8 cn;
} bt_ip_state;




_ATTR_LWBT_CODE_
void auth_complete(uint8 result)
{
    if(result == 0)
    {
        isAuth_complete = 1;
        SendMsg(MSG_BT_AUTH_OK);
    }
    else
    {
        isAuth_complete = 0xFF;   //认证失败 ，取消连接
        SendMsg(MSG_BT_AUTH_FAIL);
    }
}

_ATTR_LWBT_CODE_
void user_confirmation_request(struct bd_addr *bdaddr, uint32 Numeric_Value)
{
    LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("user_confirmation_request Numeric_Value = %d \n",Numeric_Value));
    g_remote_numeric_value = Numeric_Value;
    SendMsg(MSG_BT_USER_CONFIRMATION_REQUEST);
}


/*-----------------------------------------------------------------------------------*/
/*
 * pin_req():
 *
 * Called by HCI when a request for a PIN code has been received. A PIN code is
 * required to create a new link key.
 * Replys to the request with the given PIN code
 *
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t pin_req(void *arg, struct bd_addr *bdaddr)
{
    LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("pin_req\n"));
    SendMsg(MSG_BT_PIN_CODE_REQ);
}


/*-----------------------------------------------------------------------------------*/
/*
* link_key_not():
 *
 * Called by HCI when a new link key has been created for the connection
 * Writes the key to the Bluetooth host controller, where it can be stored for future
 * connection attempts.
  *
*/


_ATTR_LWBT_CODE_
err_t link_key_requset(void *arg, struct bd_addr *bdaddr)
{
    uint8 *key;
    key = get_link_key(bdaddr);


    if(key == NULL)
    {
        hci_link_key_request_negative_reply(bdaddr->addr);
    }
    else
    {
        hci_link_key_request_reply(bdaddr->addr, key);
    }

}




_ATTR_LWBT_CODE_
err_t link_key_not(void *arg, struct bd_addr *bdaddr, uint8 *key)
{
    LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("link_key_not\n"));

    save_key_to_sysconfig(bdaddr, key);
    {
        struct hci_inq_res  *res = NULL;
        updata_devname_to_sysconfig(bdaddr, NULL);
    }

    return hci_write_stored_link_key(bdaddr, key); /* Write link key to be stored in the
                                                    Bluetooth host controller */
}



/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_disconnected_cfm():
 *
 * Called by L2CAP to confirm that the L2CAP disconnection request was successful
 *
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t l2cap_disconnected_cfm(void *arg, struct l2cap_pcb *pcb)
{
    LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("l2cap_disconnected_cfm\n"));

    if(pcb->psm == SDP_PSM)
    {
        if(is_find_a2dp == 1)
        {
            is_find_a2dp = 0;
            //avdtp_connect((struct bd_addr *)&gSysConfig.BtConfig.LastConnectMac, connect_result);

            is_sdp_disconnet = 1;

            sdp_disconnet_timer_pos = GetSysTick();
        }
    }
    l2cap_close(pcb);
    return ERR_OK;
}


/*
 * get_rfcomm_cn():
 *
 * Parse the RFCOMM channel number from an SDP attribute list
 *
*/
_ATTR_LWBT_CODE_
uint16 get_profile_version(uint16 attribl_bc, char * attr_data, uint16 profile_uuid)
{
    uint8 i;
    for(i = 0; i < attribl_bc-1; i++)
    {
        if( (attr_data[i] == (profile_uuid >> 8)) && (attr_data[i+1] == (profile_uuid & 0x00FF)) )
        {
            return ((uint16)attr_data[i+3]) << 8 | (uint16)attr_data[i+4];
        }
    }
    return 0;
}

/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void avrcp_sdp_attributes_recv(void *arg, struct sdp_pcb *sdppcb, uint16 attribl_bc, struct pbuf *p)
{
    if(attribl_bc > 2)
    {
        uint16 avrcp_version;
        is_find_a2dp = 1;
        //avdtp_connect((struct bd_addr *)&gSysConfig.BtConfig.LastConnectMac);
        //dumpMemoryCharA(p->payload,attribl_bc );

        avrcp_version = get_profile_version(attribl_bc,p->payload, 0x110E);
        g_remote_avrcp_version = avrcp_version;
        updata_profileversion_to_sysconfig(&sdppcb->l2cappcb->remote_bdaddr,g_remote_a2dp_version,g_remote_avrcp_version );
        printf("avrcp_version = 0x%x\n",avrcp_version);
    }
    else
    {
        g_remote_avrcp_version = 0;
    }
    updata_profileversion_to_sysconfig(&sdppcb->l2cappcb->remote_bdaddr,g_remote_a2dp_version,g_remote_avrcp_version );
    is_find_a2dp = 1;
    l2ca_disconnect_req(sdppcb->l2cappcb, l2cap_disconnected_cfm);
    sdp_free(sdppcb);
}


/*-----------------------------------------------------------------------------------*/
/*
 * sdp_attributes_recv():
 *
 * Can be used as a callback by SDP when a response to a service attribute request or
 * a service search attribute request was received.
 * Disconnects the L2CAP SDP channel and connects to the RFCOMM one.
 * If no RFCOMM channel was found it initializes a search for other devices.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void sdp_attributes_recv(void *arg, struct sdp_pcb *sdppcb, uint16 attribl_bc, struct pbuf *p)
{
    struct l2cap_pcb *l2cappcb;
    #ifndef ENABLE_GET_DEV_PROFILE_VERSIONS
    l2ca_disconnect_req(sdppcb->l2cappcb, l2cap_disconnected_cfm);

    if(attribl_bc > 2)
    {
        is_find_a2dp = 1;
        //avdtp_connect((struct bd_addr *)&gSysConfig.BtConfig.LastConnectMac);
    }
    else
    {
        if(connect_result != NULL)
        {
            connect_result(RETURN_FAIL);
        }
    }
    sdp_free(sdppcb);
    #else
    uint8 ssp[] = {0x35, 0x03, 0x19, 0x11, 0x0E}; /* Service search pattern with AudioSink*/
    err_t ret;

    uint8 attrids[] = {0x35, 0x03, 0x09, 0x00, 0x09}; /* Attribute IDs to search for in data element*/
    if(attribl_bc > 2)
    {
        //is_find_a2dp = 1;
        //avdtp_connect((struct bd_addr *)&gSysConfig.BtConfig.LastConnectMac);
        int a2dp_version = 0;
        //dumpMemoryCharA(p->payload,attribl_bc );

        a2dp_version = get_profile_version(attribl_bc,p->payload, 0x110D );
        g_remote_a2dp_version = a2dp_version;
        printf("a2dp_version = 0x%x\n",a2dp_version );

        ret = sdp_service_search_attrib_req(sdppcb, 0xFFFF, ssp, sizeof(ssp), attrids, sizeof(attrids),avrcp_sdp_attributes_recv);
    }
    else
    {
        if(connect_result != NULL)
        {
            connect_result(RETURN_FAIL);
        }
        l2ca_disconnect_req(sdppcb->l2cappcb, l2cap_disconnected_cfm);
        sdp_free(sdppcb);
    }

    #endif
}


/*-----------------------------------------------------------------------------------*/
/*
* l2cap_disconnected_ind():
*
* Called by L2CAP to indicate that remote L2CAP protocol disconnected.
* Disconnects the RFCOMM protocol and the ACL link before it initializes a search for
* other devices.
*
*/
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t l2cap_disconnected_ind(void *arg, struct l2cap_pcb *pcb, err_t err)
{
    err_t ret = ERR_OK;

    LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("l2cap_disconnected_ind: L2CAP disconnected\n"));

    if(pcb->psm == SDP_PSM)
    {
        sdp_lp_disconnected(arg, pcb, err);
        return ret;
    }
    l2cap_close(pcb);
    return ret;
}

/*-----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------*/
/*
 * l2cap_connected():
 *
 * Called by L2CAP when a connection response was received.
 * Sends a L2CAP configuration request.
 * Initializes a search for other devices if the connection attempt failed.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t l2cap_connected(void *arg, struct l2cap_pcb *l2cappcb, uint16 result, uint16 status)
{
    struct sdp_pcb *sdppcb;
    //struct rfcomm_pcb *rfcommpcb;

    //uint8 ssp[] = {0x35, 0x03, 0x19, 0x11, 0x0A}; /* Service search pattern with AudioSource*/
    //uint8 ssp[] = {0x35, 0x03, 0x19, 0x11, 0x0B}; /* Service search pattern with AudioSink*/
    uint8 ssp[] = {0x35, 0x03, 0x19, 0x11, 0x0D}; /* Service search pattern with AudioSink*/
    err_t ret;

    uint8 attrids[] = {0x35, 0x03, 0x09, 0x00, 0x09}; /* Attribute IDs to search for in data element
                              sequence form */
    LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("l2cap_connected"));
    if(result == L2CAP_CONN_SUCCESS)
    {
        LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("l2cap_connected: L2CAP connected pcb->state = %d\n", l2cappcb->state));
        /* Tell L2CAP that we wish to be informed of a disconnection request */
        l2cap_disconnect_ind(l2cappcb, l2cap_disconnected_ind);
        switch(l2cap_psm(l2cappcb))
        {
            case SDP_PSM:
                LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("l2cap_connected: SDP L2CAP configured. Result = %d\n", result));

                if((sdppcb = sdp_new(l2cappcb)) == NULL)
                {
                    LWBT_DEBUGF(LWBT_BT_DT, _DBG_SERIOUS_, ("l2cap_connected: Failed to create a SDP PCB\n"));
                    return ERR_MEM;
                }

                l2cap_recv(l2cappcb, sdp_recv);

                ret = sdp_service_search_attrib_req(sdppcb, 0xFFFF, ssp, sizeof(ssp), attrids, sizeof(attrids),
                                                    sdp_attributes_recv);
                //ret = sdp_service_attrib_req(sdppcb, 0x0011,0xFFFF,attrids, sizeof(attrids),sdp_attributes_recv);
                return ret;

                return ERR_VAL;
        }
    }
    else
    {
        LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("l2cap_connected: L2CAP not connected. Redo inquiry\n"));
        l2cap_close(l2cappcb);

        if(connect_result != NULL)
        {
            connect_result(RETURN_FAIL);
        }
    }
    return ERR_OK;
}

_ATTR_LWBT_CODE_
err_t req_remote_name_complete(void *arg, struct hci_pcb *pcb,uint8 scan_status, struct bd_addr *bdaddr, uint8 * name, struct hci_inq_res *ires_header)
{
    int ret = 0;
    struct hci_inq_res  *res = NULL;
    struct hci_inq_res  *resWithName = NULL;
    struct hci_inq_res *tmpres = NULL;
    struct hci_inq_res *tmpres1 = NULL;
    int i = 0;

    if(is_stop_scan)
    {
        return ERR_OK;
    }

    if(scan_status)
    {
        //请求设备名出错， 从链表中删除该设备
        LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("req remote name fail,remove dev from scan list\n"));
        LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("fail dev mac:%02x-%02x-%02x-%02x-%02x-%02x\n", \
        bdaddr->addr[0],bdaddr->addr[1],bdaddr->addr[2],bdaddr->addr[3],bdaddr->addr[4],bdaddr->addr[5]));
        for(tmpres = pcb->iresAll; tmpres != NULL; tmpres = tmpres->next)
        {
            LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("scan list dev mac:%02x-%02x-%02x-%02x-%02x-%02x\n", \
        tmpres->bdaddr.addr[0],tmpres->bdaddr.addr[1],tmpres->bdaddr.addr[2],tmpres->bdaddr.addr[3], \
        tmpres->bdaddr.addr[4],tmpres->bdaddr.addr[5]));
            if(bd_addr_cmp(bdaddr, &(tmpres->bdaddr)))
            {
                LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("found a dev to remove\n"));
                break;

            }

        }

        if(tmpres)
        {
            HCI_RMV_COMMON(&pcb->iresAll,tmpres, tmpres1 );
            lwbt_memp_free(MEMP_HCI_INQ, tmpres);
        }
    }
    else
    {
        LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("dev name:%s,mac:%02x-%02x-%02x-%02x-%02x-%02x\n", \
        name,bdaddr->addr[0],bdaddr->addr[1],bdaddr->addr[2],bdaddr->addr[3],bdaddr->addr[4],bdaddr->addr[5]));
        hci_updata_devName(bdaddr,name);
    }

    resWithName = pcb->iresWithName;
    ires_header = pcb->iresAll;

    for(res = ires_header; res != NULL; res = res->next)
    {
        if(res->name[0] == 0)
        {
            ret = 1;
            LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("the scan list has other no name dev,send read name command\n"));
            hci_read_remote_name(&res->bdaddr,res->psrm,res->co);

            break;
        }
    }

    for(res = resWithName; res != NULL; res = res->next)
    {
        i++;
    }

    if(ret == 0)
    {
        if(scan_state == SCAN_COMPLETE)
        {
            if(scan_result != NULL)
            {
                scan_result(resWithName, i, 1);
            }
        }
        else
        {
            scan_state = SCAN_REQ_NAME_COMPLETE;

            if(scan_result != NULL)
            {
                scan_result(resWithName, i, 0);
            }
        }
    }
    else
    {
        if(scan_result != NULL)
        {
            scan_result(resWithName, i, 0);
        }
    }
}


/*-----------------------------------------------------------------------------------*/
/*
 * inquiry_complete():
 *
 * Called by HCI when a inquiry complete event was received.
 * Connects to the first device in the list.
 * Initializes a search for other devices if the inquiry failed.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t inquiry_complete(void *arg, struct hci_pcb *pcb, struct hci_inq_res *ires, uint16 result, uint16 complete)
{
    struct l2cap_pcb *l2cappcb;

    struct hci_inq_res *tmpres;
    struct hci_inq_res *tmpres1;
    struct hci_inq_res *tmpres2;
    int i;

    if(is_stop_scan)
    {
        return ERR_OK;
    }

    if(complete)
    {
        if(pcb->iresAll != NULL)
        {
            tmpres = pcb->iresAll;
            hci_read_remote_name(&tmpres->bdaddr, tmpres->psrm, tmpres->co);
            LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("inquiry_complete , has no name dev, now to read it\n"));
        }
        else
        {
            i = 0;
            for(tmpres1 = pcb->iresWithName; tmpres1 != NULL; tmpres1 = tmpres1->next)
            {
                i++;
            }
            if(scan_result)
            {
                scan_result(pcb->iresWithName, i, 1);
            }
        }

        scan_state = SCAN_COMPLETE;
    }
    else
    {
        if(result == HCI_SUCCESS)
        {
            LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("successful Inquiry\n"));
            if(ires != NULL)
            {
                //avdtp_connect(&ires->bdaddr);
                tmpres = ires;

                for(tmpres1 = pcb->iresAll; tmpres1 != NULL; tmpres1 = tmpres1->next)
                {
                    if(bd_addr_cmp(&(tmpres1->bdaddr), &(tmpres->bdaddr)))
                    {
                        break;
                    }
                }

                for(tmpres2 = pcb->iresWithName; tmpres2 != NULL; tmpres2 = tmpres2->next)
                {
                    if(bd_addr_cmp(&(tmpres2->bdaddr), &(tmpres->bdaddr)))
                    {
                        break;
                    }
                }

                if(tmpres1 == NULL && tmpres2 == NULL)
                {
#ifdef ENABLE_EXTENDED_INQUIRY
                    if(tmpres->name[0] > 0x20)
                    {
                        HCI_REG(&(pcb->iresWithName), tmpres);
                        i = 0;
                        for(tmpres1 = pcb->iresWithName; tmpres1 != NULL; tmpres1 = tmpres1->next)
                        {
                            i++;
                        }
                        if(scan_result != NULL)
                        {
                            scan_result(pcb->iresWithName, i, 0);
                        }
                    }
                    else
#endif
                    {
                        LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("dev name:%s,mac:%02x-%02x-%02x-%02x-%02x-%02x,cod:%02x-%02x-%02x\n", \
                        tmpres->name,tmpres->bdaddr.addr[0],tmpres->bdaddr.addr[1],tmpres->bdaddr.addr[2], \
                        tmpres->bdaddr.addr[3],tmpres->bdaddr.addr[4],tmpres->bdaddr.addr[5],tmpres->cod[0],\
                        tmpres->cod[1],tmpres->cod[2] \
                        ));


                        if(tmpres->name[0]== 0)
                        {
                            HCI_REG(&(pcb->iresAll), tmpres);
                        }
                        else
                        {
                            lwbt_memp_free(MEMP_HCI_INQ, tmpres);
                        }
                    }


                }
                else
                {
                    lwbt_memp_free(MEMP_HCI_INQ, tmpres);
                }

            }
            else
            {
                i = 0;
                for(tmpres1 = pcb->iresWithName; tmpres1 != NULL; tmpres1 = tmpres1->next)
                {
                    i++;
                }
                if(scan_result != NULL)
                {
                    scan_result(pcb->iresWithName, i, 0);
                }
            }
        }
        else
        {
            if(ires)
                lwbt_memp_free(MEMP_HCI_INQ, tmpres);
        }

    }

    return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
/*
 * acl_conn_complete():
 *
 * Called by HCI when a connection complete event was received.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
err_t acl_conn_complete(void *arg, struct bd_addr *bdaddr)
{
    uint8 *devMac;
    struct hci_link *link;
    updata_last_connect_dev(bdaddr);
    //hci_write_link_supervision_timeout(0, (uint32)BT_LINK_SUPERVISION_TIMEOUT*1000000/625,bdaddr);
    hci_write_link_supervision_timeout(0, (uint32)10*1000000/625,bdaddr);
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/*
 * read_bdaddr_complete():
 *
 * Called by HCI when a read local bluetooth device address complete event was received.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
err_t read_bdaddr_complete(void *arg, struct bd_addr *bdaddr)
{
    memcpy(&(bt_ip_state.bdaddr), bdaddr, 6);
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/*
 * command_complete():
 *
 * Called by HCI when an issued command has completed during the initialization of the
 * host controller.
 * Initializes a search for other devices when host controller initialization is
 * completed.
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_CODE_
void bt_a2dp_connect(struct bd_addr *bdaddr, void(*connect_result_hook)(int result))
{

    struct l2cap_pcb *l2cappcb;
    if((l2cappcb = l2cap_new()) == NULL)
    {
        LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("inquiry_complete: Could not alloc L2CAP pcb\n"));
        return;
    }

    if(scan_state == SCAN_WORKING)
    {
        bt_scan_cancel();
    }

    l2ca_connect_req(l2cappcb, bdaddr, SDP_PSM, HCI_ALLOW_ROLE_SWITCH, l2cap_connected);
    connect_result = connect_result_hook;
}

_ATTR_LWBT_CODE_
void bt_a2dp_disconnect(struct bd_addr *bdaddr, void(*disconnect_result_hook)(int result))
{
    //hci_linkdown(bdaddr, HCI_OTHER_END_TERMINATED_CONN_USER_ENDED, HCI_CONNECT_TYPE_ACL);
    BT_DEBUG("bt_a2dp_disconnect");
    avdtp_disconnect_request(bdaddr, disconnect_result_hook);
}


_ATTR_LWBT_INIT_CODE_
err_t command_complete_for_init(void *arg, struct hci_pcb *pcb, uint8 ogf, uint8 ocf, uint8 result)
{

    command_result = result;
    is_command_complete = 1;

    return command_result;
}

_ATTR_LWBT_UARTIF_CODE_
err_t command_complete(void *arg, struct hci_pcb *pcb, uint8 ogf, uint8 ocf, uint8 result)
{
    if(ogf == HCI_LINK_CONTROL && ocf == HCI_CREATE_CONNECTION_CANNEL)
    {
        if(result == HCI_SUCCESS)
        {
            LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("Send msg MSG_BT_HCI_CANNEL_CONNECT_OK\n"));
	        SendMsg(MSG_BT_HCI_CANNEL_CONNECT_OK);
        }
        else if(result == HCI_ACL_CONNECTION_EXISTS)
        {
            LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("Send msg MSG_BT_HCI_CANNEL_CONNECT_FAIL_CONNECED_EXIT\n"));
            SendMsg(MSG_BT_HCI_CANNEL_CONNECT_FAIL_CONNECED_EXIT);
        }
        else if(result == HCI_NO_CONNECTION)
        {
            LWBT_DEBUGF(LWBT_BT_DT, _DBG_INFO_, ("Send msg MSG_BT_HCI_CANNEL_CONNECT_FAIL_NO_CONNECTED\n"));
            SendMsg(MSG_BT_HCI_CANNEL_CONNECT_FAIL_NO_CONNECTED);
        }
    }

    return result;
}

#endif
#endif
/*-----------------------------------------------------------------------------------*/
