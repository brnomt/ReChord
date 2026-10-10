
#include "SysInclude.h"
#include "bd_addr.h"
#include "hci.h"

#ifdef _BLUETOOTH_

#ifdef _A2DP_SINK_
#define _ATTR_LWBT_CODE_         
#define _ATTR_LWBT_DATA_         
#define _ATTR_LWBT_BSS_          

#else
#define _ATTR_LWBT_CODE_         
#define _ATTR_LWBT_DATA_         
#define _ATTR_LWBT_BSS_          
#endif

//__packed
//struct hci_inq_res
//{
//    struct hci_inq_res *next; /* For the linked list */
//
//    struct bd_addr bdaddr; /* Bluetooth address of a device found in an inquiry */
//    uint8 cod[3]; /* Class of the remote device */
//    uint8 psrm; /* Page scan repetition mode */
//    uint8 psm; /* Page scan mode */
//    uint8 rssi;/*rssi*/
//    uint16 co; /* Clock offset */
//    uint8 name[32+1]; /*name*/
//};
void show_bt_link_info(void);

_ATTR_LWBT_BSS_  struct hci_inq_res BtWaitConnectRes;


_ATTR_LWBT_CODE_
int  save_key_to_sysconfig(struct bd_addr *bdaddr, uint8 *key)
{
#ifdef _BLUETOOTH_
    uint8 index;
    uint8 i;
    uint8 find;
    index = gSysConfig.BtConfig.KeyIndex;
    find = 0;
    SendMsg(MSG_BLUETOOTH_SVAE_PAIR_DEV);
    memcpy(gSysConfig.BtConfig.LastConnectMac,bdaddr->addr, 6);
    if(gSysConfig.BtConfig.KeyIndex < BT_LINK_KEY_MAX_NUM)
    {
        //for(i=0; i< BT_LINK_KEY_MAX_NUM; i++)
        for(i=0; i< gSysConfig.BtConfig.PairedDevCnt; i++)
        {
            if(memcmp(gSysConfig.BtConfig.BtLinkKey[i].BdAddr, bdaddr->addr, 6) == 0)
            {
                find = 1;
                break;
            }
        }
        if(find)
        {
            index = i;
        }
        else
        {
            gSysConfig.BtConfig.KeyIndex++;
            if(gSysConfig.BtConfig.KeyIndex == BT_LINK_KEY_MAX_NUM)
            {
                gSysConfig.BtConfig.KeyIndex = 0;
            }
            gSysConfig.BtConfig.PairedDevCnt++;
            if(gSysConfig.BtConfig.PairedDevCnt >= BT_LINK_KEY_MAX_NUM)
            {
                gSysConfig.BtConfig.PairedDevCnt = BT_LINK_KEY_MAX_NUM;
            }


        }
        memcpy(gSysConfig.BtConfig.BtLinkKey[index].BdAddr, bdaddr->addr, 6);
        memcpy(gSysConfig.BtConfig.BtLinkKey[index].LinkKey,key, 16);

#ifdef _A2DP_SOUCRE_
        if(bd_addr_cmp(bdaddr,&BtWaitConnectRes.bdaddr))
        {
            memcpy(gSysConfig.BtConfig.BtLinkKey[index].name, BtWaitConnectRes.name, 32);
        }
#endif
    }

#endif

    return 0;


}

#ifdef _A2DP_SOUCRE_

_ATTR_LWBT_CODE_
int  updata_devname_to_sysconfig(struct bd_addr *bdaddr, uint8 *name)
{
    int i;
    SendMsg(MSG_BLUETOOTH_LINK_KEY_NOTIFY);

    if(bd_addr_cmp(bdaddr,&BtWaitConnectRes.bdaddr))
    {
        for(i=0; i< gSysConfig.BtConfig.PairedDevCnt; i++)
        {
            if(bd_addr_cmp((struct bd_addr *)gSysConfig.BtConfig.BtLinkKey[i].BdAddr, bdaddr))
            {
                memcpy(gSysConfig.BtConfig.BtLinkKey[i].name, BtWaitConnectRes.name, 32);
                return 0;
            }
        }
    }

    return 0;
}

_ATTR_LWBT_CODE_
int  updata_profileversion_to_sysconfig(struct bd_addr *bdaddr, uint16 a2dpverion, uint16 avrcpversion)
{
    int i;
    SendMsg(MSG_BLUETOOTH_LINK_KEY_NOTIFY);

    if(bd_addr_cmp(bdaddr,&BtWaitConnectRes.bdaddr))
    {
        for(i=0; i< gSysConfig.BtConfig.PairedDevCnt; i++)
        {
            if(bd_addr_cmp((struct bd_addr *)gSysConfig.BtConfig.BtLinkKey[i].BdAddr, bdaddr))
            {
                gSysConfig.BtConfig.BtLinkKey[i].A2dpVersion = a2dpverion;
                gSysConfig.BtConfig.BtLinkKey[i].A2dpVersion = avrcpversion;
                return 0;
            }
        }
    }

    return 0;
}

#endif

_ATTR_LWBT_CODE_
uint8* get_link_key(struct bd_addr *bdaddr)
{
#ifdef _BLUETOOTH_
    int i;

    if(gSysConfig.BtConfig.KeyIndex < BT_LINK_KEY_MAX_NUM)
    {
        for(i=0; i< gSysConfig.BtConfig.PairedDevCnt; i++)
        {
            if(memcmp(gSysConfig.BtConfig.BtLinkKey[i].BdAddr, bdaddr->addr, 6) == 0)
            {
                return gSysConfig.BtConfig.BtLinkKey[i].LinkKey;
            }
        }
    }
#endif
    return NULL;

}

_ATTR_LWBT_CODE_
uint8* get_last_connect_dev(void)
{
#ifdef _BLUETOOTH_
    if(gSysConfig.BtConfig.PairedDevCnt == 0)
    {
        return NULL;
    }
    else
    {
        return &gSysConfig.BtConfig.LastConnectMac[0];
    }
#else
    return NULL;
#endif
}

_ATTR_LWBT_CODE_
void updata_last_connect_dev(struct bd_addr *bdaddr)
{
#ifdef _BLUETOOTH_
    memcpy(&gSysConfig.BtConfig.LastConnectMac[0],bdaddr->addr, 6);
#endif
}


_ATTR_LWBT_CODE_
void BlueToothDeletePairedDve(struct bd_addr *bdaddr)
{
#ifdef _BLUETOOTH_
    int i;
    struct bd_addr tmp_addr;

    //backup delete paried device mac addr.
    memcpy(tmp_addr.addr,bdaddr->addr,sizeof(struct bd_addr));

    if(gSysConfig.BtConfig.PairedDevCnt == 0)
        return;

    for(i=0; i< BT_LINK_KEY_MAX_NUM; i++)
    {
        if(bd_addr_cmp((struct bd_addr *)(&gSysConfig.BtConfig.BtLinkKey[i].BdAddr), &tmp_addr))
        {
            break;
        }
    }

    if(i == BT_LINK_KEY_MAX_NUM)
    {
        return;
    }

    hci_delete_link_key(&tmp_addr, 0x01);//delete all key

    if(i == (gSysConfig.BtConfig.PairedDevCnt-1))
    {
        memset(&gSysConfig.BtConfig.BtLinkKey[i], 0x00, sizeof(BT_LINK_KEY));
    }
    else
    {
        for(; i< (gSysConfig.BtConfig.PairedDevCnt-1) ; i++)
        {
            memcpy(&gSysConfig.BtConfig.BtLinkKey[i],&gSysConfig.BtConfig.BtLinkKey[i+1], sizeof(BT_LINK_KEY) ) ;
        }
    }

    if(memcmp(gSysConfig.BtConfig.LastConnectMac, tmp_addr.addr,6) == 0)
    {
        memset(gSysConfig.BtConfig.LastConnectMac, 0, 6);
    }

    if(gSysConfig.BtConfig.PairedDevCnt > 0)
    {
        gSysConfig.BtConfig.PairedDevCnt--;
    }

    gSysConfig.BtConfig.KeyIndex = gSysConfig.BtConfig.PairedDevCnt;

#endif

}


#if 0
void show_bt_link_info(void)
{
    int i=0;
    printf("PairedDevCnt = %d\n",gSysConfig.BtConfig.PairedDevCnt);

    for(i=0; i< BT_LINK_KEY_MAX_NUM; i++)
    {
        printf("dev info ID = %d:\n", i);
        printf("name =%s",gSysConfig.BtConfig.BtLinkKey[i].name);
        printf("mac = %02x-%02x-%02x-%02x-%02x-%02x\n",gSysConfig.BtConfig.BtLinkKey[i].BdAddr[0]
            ,gSysConfig.BtConfig.BtLinkKey[i].BdAddr[1]
            ,gSysConfig.BtConfig.BtLinkKey[i].BdAddr[2]
            ,gSysConfig.BtConfig.BtLinkKey[i].BdAddr[3]
            ,gSysConfig.BtConfig.BtLinkKey[i].BdAddr[4]
            ,gSysConfig.BtConfig.BtLinkKey[i].BdAddr[5]
            );
    }
}
#endif

#endif


