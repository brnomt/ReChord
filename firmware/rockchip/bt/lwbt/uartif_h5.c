
/*-----------------------------------------------------------------------------------*/
/* uartif.c
 *
 * Implementation of the HCI UART transport layer for Linux
 */
/*-----------------------------------------------------------------------------------*/

#include "lwbt.h"

#include "SysInclude.h"
#include "lwbtopts.h"
#include "phybusif.h"
#include "hci.h"
#include "lwbterr.h"
#include "uart.h"
#include "pbuf.h"
#include "delay.h"
#ifdef _BLUETOOTH_
#if BT_UART_INTERFACE_CONFIG == BT_UART_INTERFACE_H5
#ifdef _A2DP_SINK_
_ATTR_LWBT_DATA_ uint8 scoTxBuf[520];
#endif

_ATTR_LWBT_DATA_ uint8 isOutputComplete = 0;
_ATTR_LWBT_BSS_ uint8 HciSeverRequestCount;
_ATTR_LWBT_BSS_ uint8 HciServeDisableCount;

_ATTR_LWBT_BSS_ struct phybusif_cb g_cb;
_ATTR_LWBT_BSS_ struct h5_struct g_h5;

typedef void (*callback_func)(void);
_ATTR_LWBT_BSS_ callback_func g_callback_func;

#ifdef BT_HOST_SNIFF
_ATTR_LWBT_BSS_ uint8 is_bt_can_sniff;
_ATTR_LWBT_BSS_ uint32 hci_idle_time_tick;
_ATTR_LWBT_BSS_ uint8 hci_sniff_manage_enable;
_ATTR_LWBT_BSS_ uint8 need_sniff_wakeup;
_ATTR_LWBT_BSS_ uint8 hci_mode_state_machine;

typedef enum
{
    PHY_HCI_ACTIVE_MODE,
    PHY_HCI_HOLD_MODE,
    PHY_HCI_SINFF_MODE,
    PHY_HCI_PARK_MODE,
    PHY_HCI_WAIT_ACTIVE_MODE,
    PHY_HCI_WAIT_SNIFF_MODE,
}HCI_MODE_STATE_MACHINE;
#endif
static void h5_timed_event(void);
_ATTR_LWBT_DATA_
SYSTICK_LIST g_h5_timer_event =
{
    NULL,
    0,
    25,
    0,
    h5_timed_event,
};

void HciServeIsrRequest(void);
void  phybusif_h5_data_process();
void  phybusif_h5_data_send();
static int h5_recv(unsigned char data);
static PBUF *h5_dequeue(struct phybusif_cb *hu);
static void h5_complete_rx_pkt(struct phybusif_cb *hu,PBUF *skb);
void hci_uart_tx_wakeup(struct phybusif_cb *hu);

_ATTR_LWBT_UARTIF_CODE_
void phybusif_output_complete(void)
{
    g_cb.isOutputComplete = 1;

    hci_uart_tx_wakeup(&g_cb);
}

_ATTR_LWBT_UARTIF_CODE_
void phybusif_output_complete_callback(void)
{
    g_cb.isOutputComplete = 1;
    if (g_callback_func != NULL)
        g_callback_func();
    hci_uart_tx_wakeup(&g_cb);
}

/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_UARTIF_CODE_
int phybusif_reset(struct phybusif_cb *cb)
{
    /* Init new ctrl block */
    /* Alloc new pbuf. lwIP will handle dealloc */
    if((cb->p = pbuf_alloc(PBUF_RAW, PHY_FRAME_MAX_SIZE, PBUF_POOL)) == NULL)
    {

        //while(1);
        cb->q = cb->p; /* Make p the pointer to the head of the pbuf chain and q to the tail */

        cb->tot_recvd = 0;
        cb->recvd = 0;

        cb->state = W4_WAIT_BUF;
        return ERR_MEM; /* Could not allocate memory for pbuf */

    }
    cb->q = cb->p; /* Make p the pointer to the head of the pbuf chain and q to the tail */

    cb->tot_recvd = 0;
    cb->recvd = 0;

    cb->state = W4_PACKET_TYPE;
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/* Initializes the physical bus interface
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_INIT_CODE_
void phybusif_init(bt_hw_control_t * hw_control)
{
    memset(&g_cb, 0, sizeof(struct phybusif_cb));
    isOutputComplete = 0;
    HciSeverRequestCount = 0;
    HciServeDisableCount = 0;

    IntPendingClear(BT_H5_TX_INT_ID);
    IntRegister(BT_H5_TX_INT_ID , phybusif_h5_data_send);
    IntEnable(BT_H5_TX_INT_ID);

    pbuf_init();
    phybusif_reset(&g_cb);
    BT_Recv_Register(h5_recv);
    BT_Tx_Complete_Register(phybusif_output_complete);
    g_cb.hw_conrtol = hw_control;

    if(h5_open(&g_cb))
    {
        BT_DEBUG("h5 open failed!");
    }
}

_ATTR_LWBT_INIT_CODE_
err_t phybusif_deinit(bt_hw_control_t * hw_control)
{
    h5_close(&g_cb);
    memset(&g_cb, 0, sizeof(struct phybusif_cb));
}

_ATTR_LWBT_UARTIF_CODE_
void phybusif_waitsend_pbuf_input(struct pbuf *p)
{
    struct pbuf *q;

    if(g_cb.waitsend== NULL)
    {
        g_cb.waitsend = p;
    }
    else
    {
        q = g_cb.waitsend;

        while(q->next)
        {
            q = q->next;
        }

        q->next = p;
    }
}

_ATTR_LWBT_UARTIF_CODE_
void  phybusif_h5_data_process()
{
    struct h5_struct *h5 = &g_h5;
    PBUF *skb;
    //DEBUG("phybusif_data_process_task");
    while ((skb = pbuf_dequeue((PBUF**)&g_cb.h5_unread)) != NULL)
    {
        h5_complete_rx_pkt(&g_cb, skb);
    }
#ifdef BT_HOST_SNIFF
    hci_idle_time_tick = GetSysTick();
#endif
}

_ATTR_LWBT_UARTIF_CODE_
void  phybusif_h5_data_send()
{
    PBUF *skb;
//    if(hci_sniff_manage_enable)
//    {
//        uint mode;
//        mode = hci_get_link_mode(get_last_connect_dev(), HCI_CONNECT_TYPE_ACL);
//        if(mode != 0)
//        {
//            need_sniff_wakeup = 1;
//            //return;
//        }
//    }

    if (get_send_state())
        return;
    else if ((skb = h5_dequeue(&g_cb)) != NULL)
    {
        //DEBUG("phybusif write skb->len = %d\n",skb->buflen);

        if (NULL == skb->callback)
            BT_Tx_Complete_Register(phybusif_output_complete);
        else
        {
            g_callback_func = skb->callback;
            BT_Tx_Complete_Register(phybusif_output_complete_callback);
        }
        BT_UARTSend(BT_UART_CH, skb->payload, skb->buflen);
        pbuf_free(skb);
#ifdef BT_HOST_SNIFF
        hci_idle_time_tick = GetSysTick();
#endif
    }
}


/*-----------------------------------------------------------------------------------*/
/* HciServeIsrRequest():
 *
 * Called by main loop
 *
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_UARTIF_CODE_
void HciServeIsrRequest(void)
{
    DisableIntMaster();

    if (HciServeDisableCount == 0)
    {
        IntPendingSet(BT_HCI_SERVER_INT_ID);
    }
    else
    {
        HciSeverRequestCount = 1;
    }

    EnableIntMaster();
}

_ATTR_LWBT_UARTIF_CODE_
uint32 HciServeIsrDisable(void)
{
    DisableIntMaster();

    HciServeDisableCount++;

    EnableIntMaster();

    return 0;
}

_ATTR_LWBT_UARTIF_CODE_
void HciServeIsrEnable(void)
{
    DisableIntMaster();

    if (HciServeDisableCount > 0)
    {
        HciServeDisableCount--;
    }

    if ((HciServeDisableCount == 0) && (HciSeverRequestCount > 0))
    {
        HciSeverRequestCount = 0;
        IntPendingSet(BT_HCI_SERVER_INT_ID);
    }

    EnableIntMaster();
}
extern uint16  bufFreeCnt;
_ATTR_LWBT_UARTIF_CODE_
void _phybusif_output(struct pbuf *p, uint16 len, void (*func)(void))
{
    struct pbuf *q;
    /* Send pbuf on UART */
    //dumpMemoryCharA(p->payload,p->len);

    if(bufFreeCnt < 3)
    {
        printf("warning bt buf low level");
        return;
    }

    if(g_cb.hw_conrtol != NULL && g_cb.hw_conrtol->get_power_status)
    {
        if(g_cb.hw_conrtol->get_power_status() == POWER_STATUS_SLEEP)
        {
            if(g_cb.hw_conrtol->wake)
            {
                pbuf_ref(p);
                phybusif_waitsend_pbuf_input(p);
                g_cb.hw_conrtol->wake(NULL);
            }

            return;
        }
    }
    q = p;
    pbuf_ref(q);
    q->pkt_type = ((char *)q->payload)[0];
    pbuf_header(q, -1); // remove h4 header
    q->callback = func;


    h5_enqueue(&g_cb, q);
    hci_uart_tx_wakeup(&g_cb);
}

_ATTR_LWBT_UARTIF_CODE_
void phybusif_send_waitlist()
{
    struct pbuf *q;
    struct pbuf *p;
    unsigned char *ptr;
    if(g_cb.waitsend == NULL)
        return;

    q = g_cb.waitsend;

    while(q != NULL)
    {
        ptr = q->payload;
        g_cb.isOutputComplete = 0;
        while(get_send_state());

        BT_Tx_Complete_Register(phybusif_output_complete);
        BT_UARTSend(BT_UART_CH, ptr,q->len);

        wait_phybusif_output_complete(100000);
        p= q;
        q = q->next;
        p->next = NULL;
        pbuf_free(p);

    }

    g_cb.waitsend = NULL;
}

_ATTR_LWBT_UARTIF_CODE_
int wait_phybusif_output_complete(int timeoutUs)
{
    while(--timeoutUs)
    {
        if(g_cb.isOutputComplete)
        {
            g_cb.isOutputComplete = 0;
            return ERR_OK;
        }

        DelayUs(1);
    }
    return ERR_TIMEOUT;
}


#define VERSION "1.0"

_ATTR_LWBT_DATA_ static int txcrc = 0;
#ifdef CONFIG_RTL8723BS_MODULE
_ATTR_LWBT_DATA_ static int need_check_bt_state = 0;
#else
_ATTR_LWBT_DATA_ static int need_check_bt_state = 1;
#endif

#define H5_BTSTATE_CHECK_CONFIG_PATH      "/system/etc/firmware/rtl8723as/need_check_bt_state"

#define H5_TXWINSIZE    8
#define HCI_COMMAND_PKT 0x01
#define HCI_ACLDATA_PKT 0x02
#define HCI_SCODATA_PKT 0x03
#define HCI_EVENT_PKT   0x04
#define H5_ACK_PKT  0x00
#define H5_LE_PKT       0x0F
#define H5_VDRSPEC_PKT  0x0E

//sleep check begin
struct h5_sleep_info
{
    int  wake_sleep; //1: can sleep, value 0 need wake up
    void *uport;
    //pSemaphore wake_lock;
};

/*
 *  Global variable
 */
#define TX_TIMER_INTERVAL      2

#define STACK_TXDATA       0x01
#define STACK_SLEEP        0x02
/** Global state flags */
_ATTR_LWBT_BSS_ static unsigned long flags;
_ATTR_LWBT_BSS_ static struct h5_sleep_info *hsi;

//static int h5_parse_hci_event(struct notifier_block *this, unsigned long event, void *data);

/** Notifier block for HCI events */
//struct notifier_block hci_event_nblock =
//{
//    .notifier_call = h5_parse_hci_event,
//};


//static void h5_sleep_work(struct work_struct *work);
//DECLARE_DELAYED_WORK(sleep_workqueue, h5_sleep_work);
//sleep check end
struct h5_struct
{
//    PBUF unack;  /* Unack'ed packets queue */
//    PBUF rel;    /* Reliable packets queue */
//    PBUF unrel;  /* Unreliable packets queue */

    PBUF *unack;
    PBUF *rel;
    PBUF *unrel;
    unsigned long rx_count;
    PBUF *rx_skb;
    uint8  rxseq_txack;        /* rxseq == txack. */
    uint8  rxack;          /* Last packet sent by us that the peer ack'ed */
    //struct  timer_list th5;
    //SYSTICK_LIST  *th5;
    uint8 is_checking;

    enum
    {
        H5_W4_PKT_DELIMITER,
        H5_W4_PKT_START,
        H5_W4_HDR,
        H5_W4_DATA,
        H5_W4_CRC
    } rx_state;

    enum
    {
        H5_ESCSTATE_NOESC,
        H5_ESCSTATE_ESC
    } rx_esc_state;

    uint8  use_crc;
    uint16 message_crc;
    uint8  txack_req;      /* Do we need to send ack's to the peer? */

    /* Reliable packet sequence number - used to assign seq to each rel pkt. */
    uint8  msgq_txseq;
    uint16 buf_used_cnt;

    uint8 sync_resp_recved;
    uint8 conf_resp_recved;
};

_ATTR_LWBT_BSS_ struct phybusif_cb* hci_uart_info;
//static void h5_bt_state_err_worker(struct work_struct *private_);
//static void h5_bt_state_check_worker(struct work_struct *private_);
//static DECLARE_DELAYED_WORK(bt_state_err_work, h5_bt_state_err_worker);
//static DECLARE_DELAYED_WORK(bt_state_check_work, h5_bt_state_check_worker);


/* ---- H5 CRC calculation ---- */

/* Table for calculating CRC for polynomial 0x1021, LSB processed first,
initial value 0xffff, bits shifted in reverse order. */
_ATTR_LWBT_UARTIF_DATA_
static uint16 crc_table[] =
{
    0x0000, 0x1081, 0x2102, 0x3183,
    0x4204, 0x5285, 0x6306, 0x7387,
    0x8408, 0x9489, 0xa50a, 0xb58b,
    0xc60c, 0xd68d, 0xe70e, 0xf78f
};

/* Initialise the crc calculator */
#define H5_CRC_INIT(x) x = 0xffff
_ATTR_LWBT_UARTIF_DATA_
uint8 byte_rev_table[256] =
{
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};
_ATTR_LWBT_UARTIF_CODE_
uint16 bitrev16(uint16 x)
{
    return (byte_rev_table[x & 0xff] << 8) | byte_rev_table[x >> 8];
}

/*
   Update crc with next data byte

   Implementation note
        The data byte is treated as two nibbles.  The crc is generated
        in reverse, i.e., bits are fed into the register from the top.
*/
_ATTR_LWBT_UARTIF_CODE_
static void h5_crc_update(uint16 *crc, uint8 d)
{
    uint16 reg = *crc;

    reg = (reg >> 4) ^ crc_table[(reg ^ d) & 0x000f];
    reg = (reg >> 4) ^ crc_table[(reg ^ (d >> 4)) & 0x000f];

    *crc = reg;
}

_ATTR_LWBT_UARTIF_CODE_
char *skb_put(PBUF * skb, int len)
{
    skb->buflen += len;
    return (char*)skb->payload+(skb->buflen-len);
}
_ATTR_LWBT_UARTIF_CODE_
void skb_trim(PBUF * skb, int len)
{
    skb->buflen = len;
}
_ATTR_LWBT_UARTIF_CODE_
void skb_to_pbuf(PBUF * skb)
{
    skb->len = skb->buflen;
    skb->tot_len = skb->buflen;
}

_ATTR_LWBT_UARTIF_CODE_
void pbuf_to_skb(PBUF * skb)
{
    skb->buflen = skb->len;
}
/* ---- H5 core ---- */
_ATTR_LWBT_UARTIF_CODE_
static void h5_slip_msgdelim(PBUF *skb)
{
    const char pkt_delim = 0xc0;

    memcpy(skb_put(skb, 1), &pkt_delim, 1);
}

//static void h5_slip_one_byte(struct sk_buff *skb, uint8 c)
_ATTR_LWBT_UARTIF_CODE_
static void h5_slip_one_byte(PBUF *skb, uint8 c)
{
    const char esc_c0[2] = { 0xdb, 0xdc };
    const char esc_db[2] = { 0xdb, 0xdd };
    const char esc_11[2] = { 0xdb, 0xde };
    const char esc_13[2] = { 0xdb, 0xdf };

    switch (c)
    {
        case 0xc0:
            memcpy(skb_put(skb, 2), &esc_c0, 2);
            break;
        case 0xdb:
            memcpy(skb_put(skb, 2), &esc_db, 2);
            break;
        case 0x11:
            memcpy(skb_put(skb, 2), &esc_11, 2);
            break;
        case 0x13:
            memcpy(skb_put(skb, 2), &esc_13, 2);
            break;
        default:
            memcpy(skb_put(skb, 1), &c, 1);
            break;
    }
}
_ATTR_LWBT_UARTIF_CODE_
int h5_enqueue(struct phybusif_cb *hu, PBUF *skb)
{
    struct h5_struct *h5 = &g_h5;
//    if (skb->buflen > 0xFFF)   //Pkt length must be less than 4095 bytes
//    {
//        BT_DEBUG("Packet too long");
//        pbuf_free(skb);
//        return 0;
//    }
    pbuf_to_skb(skb);
    //DEBUG("Enter h5 enqueue, pkt type=0x%x", skb->pkt_type);
    switch (skb->pkt_type)
    {
        case HCI_ACLDATA_PKT:
        case HCI_COMMAND_PKT:
            pbuf_queue_tail(&h5->rel, skb);
#ifdef BT_HOST_SNIFF
            if(skb->pkt_type == HCI_ACLDATA_PKT)
            {
                if(hci_sniff_manage_enable)
                {
                    uint mode;
                    mode = hci_get_link_mode(get_last_connect_dev(), HCI_CONNECT_TYPE_ACL);
                    if(mode != 0 && hci_mode_state_machine != PHY_HCI_WAIT_ACTIVE_MODE)
                    {
                        need_sniff_wakeup = 1;
                        //return;
                        {
                            struct hci_link * link;
                            link = hci_get_link((struct bd_addr *)get_last_connect_dev(), HCI_CONNECT_TYPE_ACL);
                            BT_DEBUG("hci_exit_sniff_mode\n");
                            hci_exit_sniff_mode(link->conhdl);
                            hci_mode_state_machine = PHY_HCI_WAIT_ACTIVE_MODE;
                        }
                    }
                }
            }
#endif
            break;

        case HCI_SCODATA_PKT:
            pbuf_queue_tail(&h5->unrel, skb);
            break;
        case H5_LE_PKT:
        case H5_ACK_PKT:
        case H5_VDRSPEC_PKT:
            pbuf_queue_tail(&h5->unrel, skb);    /* 3-wire LinkEstablishment*/
            break;

        default:
            BT_DEBUG("Unknown packet type");
            pbuf_free(skb);
            break;
    }

    return 0;
}
_ATTR_LWBT_UARTIF_CODE_
static PBUF *h5_prepare_pkt(struct h5_struct *h5, uint8 *data,
                            int len, int pkt_type)
{
    PBUF *nskb;
    uint8 hdr[4], chan;
    uint16 H5_CRC_INIT(h5_txmsg_crc);
    int rel, i;
    switch (pkt_type)
    {
        case HCI_ACLDATA_PKT:
            chan = 2;   /* 3-wire ACL channel */
            rel = 1;    /* reliable channel */
            break;
        case HCI_COMMAND_PKT:
            chan = 1;   /* 3-wire cmd channel */
            rel = 1;    /* reliable channel */
            break;
        case HCI_EVENT_PKT:
            chan = 4;   /* 3-wire cmd channel */
            rel = 1;    /* reliable channel */
            break;
        case HCI_SCODATA_PKT:
            chan = 3;   /* 3-wire SCO channel */
            rel = 0;    /* unreliable channel */
            break;
        case H5_LE_PKT:
            chan = 15;  /* 3-wire LinkEstablishment channel */
            rel = 0;    /* unreliable channel */
            break;
        case H5_ACK_PKT:
            chan = 0;   /* 3-wire ACK channel */
            rel = 0;    /* unreliable channel */
            break;
        case H5_VDRSPEC_PKT:
            chan = 14;  /* 3-wire Vendor Specific channel */
            rel = 0;    /* unreliable channel */
            break;
        default:
            BT_DEBUG("Unknown packet type");
            return NULL;
    }


    /* Max len of packet: (original len +4(h5 hdr) +2(crc))*2
       (because bytes 0xc0 and 0xdb are escaped, worst case is
       when the packet is all made of 0xc0 and 0xdb :) )
       + 2 (0xc0 delimiters at start and end). */
    //nskb = alloc_skb((len + 6) * 2 + 2, GFP_ATOMIC);
    nskb = pbuf_alloc(PBUF_RAW, PHY_FRAME_MAX_SIZE, PBUF_POOL);
    if (!nskb)
        return NULL;

    //bt_cb(nskb)->pkt_type = pkt_type;
    nskb->pkt_type = pkt_type;

    h5_slip_msgdelim(nskb);

    hdr[0] = h5->rxseq_txack << 3;
    h5->txack_req = 0;
    //BT_DEBUG("We request packet no %u to card", h5->rxseq_txack);

    if (rel)
    {
        hdr[0] |= 0x80 + h5->msgq_txseq;
        //printf("-%u\n", h5->msgq_txseq);
        h5->msgq_txseq = (h5->msgq_txseq + 1) & 0x07;
    }

    if (h5->use_crc)
        hdr[0] |= 0x40;

    hdr[1] = ((len << 4) & 0xff) | chan;
    hdr[2] = len >> 4;
    hdr[3] = ~(hdr[0] + hdr[1] + hdr[2]);

    /* Put H5 header */
    for (i = 0; i < 4; i++)
    {
        h5_slip_one_byte(nskb, hdr[i]);

        if (h5->use_crc)
            h5_crc_update(&h5_txmsg_crc, hdr[i]);
    }

    /* Put payload */
    for (i = 0; i < len; i++)
    {
        h5_slip_one_byte(nskb, data[i]);

        if (h5->use_crc)
            h5_crc_update(&h5_txmsg_crc, data[i]);
    }

    /* Put CRC */
    if (h5->use_crc)
    {
        h5_txmsg_crc = bitrev16(h5_txmsg_crc);
        h5_slip_one_byte(nskb, (uint8) ((h5_txmsg_crc >> 8) & 0x00ff));
        h5_slip_one_byte(nskb, (uint8) (h5_txmsg_crc & 0x00ff));
    }

    h5_slip_msgdelim(nskb);

    return nskb;
}

_ATTR_LWBT_UARTIF_CODE_
static void h5_unack_lock_data_send()
{
    //IntDisable(BT_UART_INT_ID);
    IntDisable(FAULT_ID15_SYSTICK);
    IntDisable(BT_HCI_SERVER_INT_ID);
}

_ATTR_LWBT_UARTIF_CODE_
static void h5_unack_unlock_data_send()
{
    //IntEnable(BT_UART_INT_ID);
    IntEnable(FAULT_ID15_SYSTICK);
    IntEnable(BT_HCI_SERVER_INT_ID);
}

_ATTR_LWBT_UARTIF_CODE_
int h5_unack_len()
{
    return pbuf_queue_len(&(g_h5.unack));
}

/* This is a rewrite of pkt_avail in AH5 */
_ATTR_LWBT_UARTIF_CODE_
static PBUF *h5_dequeue(struct phybusif_cb *hu)
{
    struct h5_struct *h5 = &g_h5;
    unsigned long flags;
    PBUF *skb;
    //printf("\nh5_dequeue %d", pbuf_queue_len(&h5->unack));
    /* First of all, check for unreliable messages in the queue,
       since they have priority */
    if ((skb = pbuf_dequeue(&h5->unrel)) != NULL)
    {
        //BT_DEBUG("skb = %04x",(uint32)skb);
        PBUF *nskb = h5_prepare_pkt(h5, (uint8 *)skb->payload, (int)skb->buflen, (int)skb->pkt_type);
        if (nskb)
        {
            nskb->callback = skb->callback;
            pbuf_free(skb);
            return nskb;
        }
        else
        {
            pbuf_queue_head(&h5->unrel, skb);
            BT_DEBUG("Could not dequeue pkt because alloc_skb failed");
        }
    }

    /* Now, try to send a reliable pkt. We can only send a
       reliable packet if the number of packets sent but not yet ack'ed
       is < than the winsize */
    //spin_lock_irqsave_nested(&h5->unack.lock, flags, SINGLE_DEPTH_NESTING);//wp
    h5_unack_lock_data_send();
    if (pbuf_queue_len(&h5->unack) < H5_TXWINSIZE && (skb = pbuf_dequeue(&h5->rel)) != NULL)
    {
        PBUF * nskb = h5_prepare_pkt(h5, (uint8 *)skb->payload, (int)skb->buflen, skb->pkt_type);
        if (nskb)
        {
            nskb->callback = skb->callback;
            pbuf_queue_tail(&h5->unack, skb);
            //spin_unlock_irqrestore(&h5->unack.lock, flags);
            h5_unack_unlock_data_send();
            SystickTimerStart(&g_h5_timer_event);
            return nskb;
        }
        else
        {
            pbuf_queue_head(&h5->rel, skb);
            BT_DEBUG("Could not dequeue pkt because alloc_skb failed");
        }
    }
    h5_unack_unlock_data_send();
    //spin_unlock_irqrestore(&h5->unack.lock, flags); //wp

    /* We could not send a reliable packet, either because there are
       none or because there are too many unack'ed pkts. Did we receive
       any packets we have not acknowledged yet ? */
    if (h5->txack_req)
    {
        /* if so, craft an empty ACK pkt and send it on H5 unreliable
           channel 0 */
        PBUF*nskb = h5_prepare_pkt(h5, NULL, 0, H5_ACK_PKT);
        return nskb;
    }
    /*
        if (need_check_bt_state) {
            schedule_delayed_work(&bt_state_check_work, HZ * 15);
        }
    */
    /* We have nothing to send */
    return NULL;
}
_ATTR_LWBT_UARTIF_CODE_
static int h5_flush(struct phybusif_cb * hu)
{
    BT_DEBUG("hu %p", hu);
    return 0;
}

/* Remove ack'ed packets */
_ATTR_LWBT_UARTIF_CODE_
static void h5_pkt_cull(struct h5_struct *h5)
{
    unsigned long flags;
    int i, pkts_to_be_removed;
    int j;
    uint8 seqno;
    PBUF * temp;

    //spin_lock_irqsave(&h5->unack.lock, flags);//wp

    pkts_to_be_removed = pbuf_queue_len(&h5->unack);
    seqno = h5->msgq_txseq;

    while (pkts_to_be_removed)
    {
        if (h5->rxack == seqno)
            break;
        pkts_to_be_removed--;
        seqno = (seqno - 1) & 0x07;
    }

    if (h5->rxack != seqno)
        BT_DEBUG("Peer acked invalid packet,h5->msgq_txseq = %d,h5->rxack = %d, unack = %d, seqno=%d\n",
              h5->msgq_txseq,h5->rxack,pbuf_queue_len(&h5->unack), seqno);

    //printf("Removing %u pkts out of %u, up to seqno %u\n",
    //        pkts_to_be_removed, pbuf_queue_len(&h5->unack), (seqno - 1) & 0x07);
    i = 0;
#if 0

    skb_queue_walk_safe(&h5->unack, skb, tmp)
    {
        if (i >= pkts_to_be_removed)
            break;
        i++;

        __skb_unlink(skb, &h5->unack);
        pbuf_free(skb);
    }
#endif
    j = pkts_to_be_removed;
    while(j)
    {
        j--;
        temp = pbuf_dequeue_head(&h5->unack);
        if(temp)
        {
            pbuf_free(temp);
        }
        else
        {
            break;
        }
    }


    if (pbuf_queue_empty(&h5->unack))
    {
        SystickTimerStop(&g_h5_timer_event);
    }
    //spin_unlock_irqrestore(&h5->unack.lock, flags);//wp

    //if (j != pkts_to_be_removed)
    //    BT_DEBUG("Removed only %u out of %u pkts", i, pkts_to_be_removed);
}

/* Handle H5 link-establishment packets. When we
   detect a "sync" packet, symptom that the BT module has reset,
   we do nothing :) (yet) */
#define H5_RECV_BUF  ((char*)h5->rx_skb->payload)
#define H5_PROC_BUF  ((char*)skb->payload)
_ATTR_LWBT_UARTIF_CODE_
static void h5_handle_le_pkt(struct phybusif_cb *hu, PBUF *skb)
{
    struct h5_struct *h5 = &g_h5;
    uint8 conf_pkt[2]     = { 0x03, 0xfc};
    uint8 conf_rsp_pkt[3] = { 0x04, 0x7b, 0x00};
    uint8 sync_pkt[2]     = { 0x01, 0x7e};
    uint8 sync_rsp_pkt[2] = { 0x02, 0x7d};

    uint8 wakeup_pkt[2]   = { 0x05, 0xfa};
    uint8 woken_pkt[2]    = { 0x06, 0xf9};
    uint8 sleep_pkt[2]    = { 0x07, 0x78};
    uint8 h5_sync_resp_pkt[8] = {0xc0, 0x00, 0x2F, 0x00, 0xD0, 0x02, 0x7D, 0xc0};
    uint8 h5_conf_resp_pkt_to_Ctrl[8] = {0xc0, 0x00, 0x2F, 0x00, 0xD0, 0x04, 0x7B, 0xc0};
    //PBUF *skb = h5->rx_skb;
    /* spot "conf" pkts and reply with a "conf rsp" pkt */
    if (H5_PROC_BUF[1] >> 4 == 2 && H5_PROC_BUF[2] == 0 &&
            !memcmp(&H5_PROC_BUF[4], conf_pkt, 2))
    {
        /*
        PBUF *nskb = pbuf_alloc(PBUF_RAW, PHY_FRAME_MAX_SIZE, PBUF_POOL);

        BT_DEBUG("Found a LE conf pkt");
        if (!nskb)
            return;

        conf_rsp_pkt[2] |= txcrc << 0x4; //crc check enable, version no = 0. needed to be as avariable.
        memcpy(skb_put(nskb, 3), conf_rsp_pkt, 3);
        nskb->pkt_type = H5_LE_PKT;

        pbuf_queue_head(&h5->unrel, nskb);
        hci_uart_tx_wakeup(hu);//wp 2015/10/16
        */
        //DEBUG("Found a LE conf pkt");
        //g_cb.hw_conrtol->write(h5_conf_resp_pkt_to_Ctrl, 8);
        while(get_send_state());
        BT_UARTSend(BT_UART_CH, h5_conf_resp_pkt_to_Ctrl, 8);
        wait_phybusif_output_complete(100000);
    }
    /* spot "conf resp" pkts*/
    else if (/*H5_RECV_BUF[1] >> 4 == 2 && */H5_PROC_BUF[2] == 0 &&
            !memcmp(&H5_PROC_BUF[4], conf_rsp_pkt, 2))
    {
        //DEBUG("Found a LE conf resp pkt, device go into active state");
        txcrc = (H5_PROC_BUF[6] >> 0x4) & 0x1;
        h5->conf_resp_recved = 1;
    }

    /* Spot "sync" pkts. If we find one...disaster! */
    else if (H5_PROC_BUF[1] >> 4 == 2 && H5_PROC_BUF[2] == 0 &&
             !memcmp(&H5_PROC_BUF[4], sync_pkt, 2))
    {
        //DEBUG("Found a LE sync pkt, card has reset");
        //DO Something here
        //g_cb.hw_conrtol->write(h5_sync_resp_pkt, 8);
        while(get_send_state());
        BT_UARTSend(BT_UART_CH, h5_sync_resp_pkt, 8);
        wait_phybusif_output_complete(100000);
    }
    /* Spot "sync resp" pkts. If we find one...disaster! */
    else if (H5_PROC_BUF[1] >> 4 == 2 && H5_PROC_BUF[2] == 0 &&
             !memcmp(&H5_PROC_BUF[4], sync_rsp_pkt, 2))
    {
        //DEBUG("Found a LE sync resp pkt, device go into initialized state");
        //      DO Something here
        h5->sync_resp_recved = 1;
    }
    /* Spot "wakeup" pkts. reply woken message when in active mode */
    else if (H5_PROC_BUF[1] >> 4 == 2 && H5_PROC_BUF[2] == 0 &&
             !memcmp(&H5_PROC_BUF[4], wakeup_pkt, 2))
    {
        PBUF *nskb = pbuf_alloc(PBUF_RAW, PHY_FRAME_MAX_SIZE, PBUF_POOL);

        BT_DEBUG("Found a LE Wakeup pkt, and reply woken message");
        //      DO Something here

        memcpy(skb_put(nskb, 2), woken_pkt, 2);
        nskb->pkt_type = H5_LE_PKT;

        pbuf_queue_head(&h5->unrel, nskb);
        hci_uart_tx_wakeup(hu);
    }
    /* Spot "woken" pkts. receive woken message from device */
    else if (H5_PROC_BUF[1] >> 4 == 2 && H5_PROC_BUF[2] == 0 &&
             !memcmp(&H5_PROC_BUF[4], woken_pkt, 2))
    {
        BT_DEBUG("Found a LE woken pkt from device");
        //      DO Something here
    }
    /* Spot "Sleep" pkts*/
    else if (H5_PROC_BUF[1] >> 4 == 2 && H5_PROC_BUF[2] == 0 &&
             !memcmp(&H5_PROC_BUF[4], sleep_pkt, 2))
    {
        BT_DEBUG("Found a LE Sleep pkt");
        //      DO Something here
    }

}
_ATTR_LWBT_UARTIF_CODE_
static void h5_unslip_one_byte(struct h5_struct *h5, unsigned char byte)
{
    const uint8 c0   = 0xc0, db   = 0xdb;
    const uint8 oof1 = 0x11, oof2 = 0x13;

    switch (h5->rx_esc_state)
    {
        case H5_ESCSTATE_NOESC:
            switch (byte)
            {
                case 0xdb:
                    h5->rx_esc_state = H5_ESCSTATE_ESC;
                    break;
                default:
                    memcpy(skb_put(h5->rx_skb, 1), &byte, 1);
                    if ((H5_RECV_BUF[0] & 0x40) != 0 &&
                            h5->rx_state != H5_W4_CRC)
                        h5_crc_update(&h5->message_crc, byte);
                    h5->rx_count--;
            }
            break;

        case H5_ESCSTATE_ESC:
            switch (byte)
            {
                case 0xdc:
                    memcpy(skb_put(h5->rx_skb, 1), &c0, 1);
                    if ((H5_RECV_BUF[0] & 0x40) != 0 &&
                            h5->rx_state != H5_W4_CRC)
                        h5_crc_update(&h5-> message_crc, 0xc0);
                    h5->rx_esc_state = H5_ESCSTATE_NOESC;
                    h5->rx_count--;
                    break;

                case 0xdd:
                    memcpy(skb_put(h5->rx_skb, 1), &db, 1);
                    if ((H5_RECV_BUF[0] & 0x40) != 0 &&
                            h5->rx_state != H5_W4_CRC)
                        h5_crc_update(&h5-> message_crc, 0xdb);
                    h5->rx_esc_state = H5_ESCSTATE_NOESC;
                    h5->rx_count--;
                    break;

                case 0xde:
                    memcpy(skb_put(h5->rx_skb, 1), &oof1, 1);
                    if ((H5_RECV_BUF[0] & 0x40) != 0 && h5->rx_state != H5_W4_CRC)
                        h5_crc_update(&h5-> message_crc, oof1);
                    h5->rx_esc_state = H5_ESCSTATE_NOESC;
                    h5->rx_count--;
                    break;

                case 0xdf:
                    memcpy(skb_put(h5->rx_skb, 1), &oof2, 1);
                    if ((H5_RECV_BUF[0] & 0x40) != 0 && h5->rx_state != H5_W4_CRC)
                        h5_crc_update(&h5-> message_crc, oof2);
                    h5->rx_esc_state = H5_ESCSTATE_NOESC;
                    h5->rx_count--;
                    break;

                default:
                    BT_DEBUG ("Invalid byte %02x after esc byte", byte);
                    pbuf_free(h5->rx_skb);
                    h5->rx_skb = NULL;
                    h5->rx_state = H5_W4_PKT_DELIMITER;
                    h5->rx_count = 0;
            }
    }
}
_ATTR_LWBT_UARTIF_CODE_
void hci_recv_frame(PBUF *skb)
{
    //DEBUG("HCI recive frame, pkt type=0x%x", skb->pkt_type);
    switch (skb->pkt_type)
    {
        case HCI_ACLDATA_PKT:
            pbuf_header(skb, -HCI_ACL_HDR_LEN);
            hci_acl_input(skb);

            break;
        case HCI_COMMAND_PKT:

            break;
        case HCI_EVENT_PKT:
            pbuf_header(skb, -HCI_EVENT_HDR_LEN);
            hci_event_input(skb);
            pbuf_free(skb);
            break;
        case HCI_SCODATA_PKT:
#ifdef _A2DP_SINK_
            pbuf_header(skb, -HCI_SCO_HDR_LEN);
            hci_sco_input(skb);
#endif
            break;
        case H5_LE_PKT:
            pbuf_free(skb);
            break;
        case H5_ACK_PKT:
            pbuf_free(skb);
            break;
        case H5_VDRSPEC_PKT:
            pbuf_free(skb);
            break;
        default:
            BT_DEBUG("Unknown packet type");
            pbuf_free(skb);
            break ;
    }

    //HciServeIsrRequest();
}
_ATTR_LWBT_UARTIF_CODE_
static void h5_complete_rx_pkt(struct phybusif_cb *hu,PBUF *skb)
{
    struct h5_struct *h5 = &g_h5;
    int pass_up;
#if 1
    if (H5_PROC_BUF[0] & 0x80)     /* reliable pkt */
    {
        //BT_DEBUG("Received seqno %u from card", h5->rxseq_txack);
//        h5->rxseq_txack++;
//        h5->rxseq_txack %= 0x8;
//        h5->txack_req    = 1;

        /* If needed, transmit an ack pkt */
        hci_uart_tx_wakeup(hu); //wp 20151016
    }
#endif
    h5->rxack = (H5_PROC_BUF[0] >> 3) & 0x07;
    //printf("+%u\n", h5->rxack);
    //printf("in <<:");
    //debug_hex(skb->payload, skb->buflen, 16);
    h5_pkt_cull(h5);

    if ((H5_PROC_BUF[1] & 0x0f) == 2 &&
            H5_PROC_BUF[0] & 0x80)
    {
        skb->pkt_type = HCI_ACLDATA_PKT;
        pass_up = 1;
    }
    else if ((H5_PROC_BUF[1] & 0x0f) == 4 &&
             H5_PROC_BUF[0] & 0x80)
    {
        skb->pkt_type = HCI_EVENT_PKT;
        pass_up = 1;
        //BT_DEBUG("pkt_type = HCI_EVENT_PKT");
        if (need_check_bt_state)
        {
            /*
             * after having received packets from controller, del hungup timer
            */
            if (h5->is_checking)
            {
                h5->is_checking = FALSE;
                BT_DEBUG("cancle state err work\n");
                //cancel_delayed_work(&bt_state_check_work);
                //cancel_delayed_work(&bt_state_err_work); //wp 20151016
                //mutex_unlock(&sem_exit);
                //rkos_semaphore_give(sem_exit);
            }
        }

    }
    else if ((H5_PROC_BUF[1] & 0x0f) == 3)
    {
        skb->pkt_type = HCI_SCODATA_PKT;
        pass_up = 1;
    }
    else if ((H5_PROC_BUF[1] & 0x0f) == 15 &&
             !(H5_PROC_BUF[0] & 0x80))
    {
        h5_handle_le_pkt(hu,skb);//Link Establishment Pkt
        pass_up = 0;
    }
    else if ((H5_PROC_BUF[1] & 0x0f) == 1 &&
             H5_PROC_BUF[0] & 0x80)
    {
        skb->pkt_type = HCI_COMMAND_PKT;
        pass_up = 1;
    }
    else if ((H5_PROC_BUF[1] & 0x0f) == 14)
    {
        skb->pkt_type = H5_VDRSPEC_PKT;
        pass_up = 1;
    }
    else
        pass_up = 0;

    //if (!pass_up)
    if (pass_up)
    {
        //struct hci_event_hdr hdr;
        uint8 desc = (H5_PROC_BUF[1] & 0x0f);

        if (desc != H5_ACK_PKT && desc != H5_LE_PKT)
        {

            skb_to_pbuf(skb);
            pbuf_header(skb, -4);
            //printf("skb = %x\n",(uint32)skb);
            hci_recv_frame(skb);//this is a hci frame //wp
            //printf("f out\n");
        }
        else
        {
            /* Pull out H5 hdr */
            //skb_pull(h5->rx_skb, 4);
            //DEBUG("H5_ACK_PKT || H5_LE_PKT");
            skb_to_pbuf(skb);
            pbuf_header(skb, -4);
            if (need_check_bt_state)
            {
                //schedule_delayed_work(&bt_state_check_work, HZ * 60); //wp 20151016
            }
            hci_recv_frame(skb);//this is a hci frame //wp
            //pbuf_free(skb);
        }

        //h5->rx_state = H5_W4_PKT_DELIMITER;
        //h5->rx_skb = NULL;
    }
    else
    {
        pbuf_free(skb);
        //h5->rx_state = H5_W4_PKT_DELIMITER;
        //h5->rx_skb = NULL;
    }
}

_ATTR_LWBT_UARTIF_CODE_
static uint16 bscp_get_crc(struct h5_struct *h5)
{
    uint16 i = H5_RECV_BUF[h5->rx_skb->buflen - 2];
    uint16 j = H5_RECV_BUF[h5->rx_skb->buflen - 1];
    //return get_unaligned_be16(&h5->rx_skb->buf[h5->rx_skb->len - 2]);
    return  (j<< 8 | i);
}

/* Recv data */
_ATTR_LWBT_UARTIF_CODE_
static int h5_recv(unsigned char data)
{
    struct h5_struct *h5 = &g_h5;
    struct phybusif_cb *hu = &g_cb;
    unsigned char *ptr;
    int count = 1;
    //DEBUG("hu %p count %d rx_state %d rx_count %ld",hu, count, h5->rx_state, h5->rx_count);

    ptr = &data;
    while (count)
    {
        if (h5->rx_count)
        {
            if (*ptr == 0xc0)
            {
                //BT_DEBUG("Short H5 packet");
                pbuf_free(h5->rx_skb);
                h5->rx_state = H5_W4_PKT_START;
                h5->rx_count = 0;
            }
            else
                h5_unslip_one_byte(h5, *ptr);

            ptr++;
            count--;
            continue;
        }

        switch (h5->rx_state)
        {
            case H5_W4_HDR:
                if ((0xff & (uint8) ~ (H5_RECV_BUF[0] + H5_RECV_BUF[1] +
                                       H5_RECV_BUF[2])) != H5_RECV_BUF[3])
                {
                    BT_DEBUG("Error in H5 hdr checksum");
                    pbuf_free(h5->rx_skb);
                    h5->rx_state = H5_W4_PKT_DELIMITER;
                    h5->rx_count = 0;
                    continue;
                }
                if ( ( H5_RECV_BUF[0] & 0x80)  /* reliable pkt */
                        && (H5_RECV_BUF[0] & 0x07) != h5->rxseq_txack)
                {
                    //BT_DEBUG ("Out-of-order packet arrived, got %u expected %u",
                    //          H5_RECV_BUF[0] & 0x07, h5->rxseq_txack);
                    BT_DEBUG("Out-of-order, got %u expected %u\n",
                          H5_RECV_BUF[0] & 0x07, h5->rxseq_txack);
                    h5->txack_req = 1;
                    hci_uart_tx_wakeup(hu);
                    pbuf_free(h5->rx_skb);
                    h5->rx_state = H5_W4_PKT_DELIMITER;
                    h5->rx_count = 0;
                    continue;
                }
                h5->rx_state = H5_W4_DATA;
                h5->rx_count = (H5_RECV_BUF[1] >> 4) +
                               (H5_RECV_BUF[2] << 4); /* May be 0 */
                continue;

            case H5_W4_DATA:
                if (H5_RECV_BUF[0] & 0x40)     /* pkt with crc */
                {
                    h5->rx_state = H5_W4_CRC;
                    h5->rx_count = 2;
                }
                else
                {
                    //h5_complete_rx_pkt(hu);
                    pbuf_queue_tail((PBUF**)&hu->h5_unread,h5->rx_skb);
                    if (H5_RECV_BUF[0] & 0x80)     /* reliable pkt */
                    {
                        //BT_DEBUG("Received seqno %u from card", h5->rxseq_txack);
                        h5->rxseq_txack++;
                        h5->rxseq_txack %= 0x8;
                        h5->txack_req    = 1;

                        /* If needed, transmit an ack pkt */
                        //hci_uart_tx_wakeup(&g_cb); //wp 20151016
                    }
                    h5->rx_skb = NULL;
                    h5->rx_state = H5_W4_PKT_DELIMITER;

                    HciServeIsrRequest();
                }
                continue;

            case H5_W4_CRC:
                if (bitrev16(h5->message_crc) != bscp_get_crc(h5))
                {
                    BT_DEBUG ("Checksum failed: computed %04x received %04x",
                           bitrev16(h5->message_crc),
                           bscp_get_crc(h5));

                    pbuf_free(h5->rx_skb);
                    h5->rx_state = H5_W4_PKT_DELIMITER;
                    h5->rx_count = 0;
                    continue;
                }
                skb_trim(h5->rx_skb, h5->rx_skb->buflen - 2);
                //h5_complete_rx_pkt(hu);
                pbuf_queue_tail((PBUF**)&hu->h5_unread,h5->rx_skb);
                if (H5_RECV_BUF[0] & 0x80)     /* reliable pkt */
                {
                    //BT_DEBUG("Received seqno %u from card", h5->rxseq_txack);
                    h5->rxseq_txack++;
                    h5->rxseq_txack %= 0x8;
                    h5->txack_req    = 1;

                    /* If needed, transmit an ack pkt */
                    //hci_uart_tx_wakeup(&g_cb); //wp 20151016
                }
                h5->rx_skb = NULL;
                h5->rx_state = H5_W4_PKT_DELIMITER;

                HciServeIsrRequest();
                continue;

            case H5_W4_PKT_DELIMITER:
                switch (*ptr)
                {
                    case 0xc0:
                        h5->rx_state = H5_W4_PKT_START;
                        break;
                    default:
                        break;
                }
                ptr++;
                count--;
                break;

            case H5_W4_PKT_START:
                switch (*ptr)
                {
                    case 0xc0:
                        ptr++;
                        count--;
                        break;

                    default:
                        h5->rx_state = H5_W4_HDR;
                        h5->rx_count = 4;
                        h5->rx_esc_state = H5_ESCSTATE_NOESC;
                        H5_CRC_INIT(h5->message_crc);
                        //BT_DEBUG("H5_W4_PKT_START");
                        /* Do not increment ptr or decrement count
                         * Allocate packet. Max len of a H5 pkt=
                         * 0xFFF (payload) +4 (header) +2 (crc) */
                        //h5->rx_skb = bt_skb_alloc(0x1005, GFP_ATOMIC);
                        h5->rx_skb = pbuf_alloc(PBUF_RAW, PHY_FRAME_MAX_SIZE, PBUF_POOL);
                        if (!h5->rx_skb)
                        {
                            BT_DEBUG("Can't allocate mem for new packet");
                            h5->rx_state = H5_W4_PKT_DELIMITER;
                            h5->rx_count = 0;
                            return 0;
                        }
                        //h5->rx_skb->dev = (void *) hu->hdev; //wp
                        break;
                }
                break;
        }
    }
    return count;
}

_ATTR_LWBT_UARTIF_CODE_
void hci_uart_tx_wakeup(struct phybusif_cb *hu)
{
    //send data
    IntPendingSet(BT_H5_TX_INT_ID);
}

/* Arrange to retransmit all messages in the relq. */
_ATTR_LWBT_UARTIF_CODE_
static void h5_timed_event(void)
{
    struct h5_struct *h5 = &g_h5;
    PBUF *skb;
    unsigned long flags;

    //BT_DEBUG("hu %p retransmitting %u pkts", hu, h5->unack.qlen);
    BT_DEBUG("h5->unack = 0x%04x\n", (uint32)h5->unack);
    //spin_lock_irqsave_nested(&h5->unack.lock, flags, SINGLE_DEPTH_NESTING); //wp
    while ((skb = pbuf_dequeue_tail(&h5->unack)) != NULL)
    {
        h5->msgq_txseq = (h5->msgq_txseq - 1) & 0x07;
        pbuf_queue_head(&h5->rel, skb);
    }
    //spin_unlock_irqrestore(&h5->unack.lock, flags); //wp

    hci_uart_tx_wakeup(&g_cb);// wp maybe to send the uart data
}

#define H5_RETRY 3
_ATTR_LWBT_UARTIF_CODE_
int h5_open(struct phybusif_cb *hu)
{
    struct h5_struct *h5 = &g_h5;
    PBUF *sync;
    PBUF *conf;
    PBUF *pure_ack;
    uint8 conf_pkt[3]     = { 0x03, 0xfc,5};
    uint8 sync_pkt[2]     = { 0x01, 0x7e};
    int retry = H5_RETRY;
    int timeout = 15; //150ms
    int ret = 0;

    if (need_check_bt_state)
    {
        hci_uart_info = hu;
    }

    h5->rx_state = H5_W4_PKT_DELIMITER;
    if (txcrc)
        h5->use_crc = 1;

syncretry:

    timeout = 15; //150ms
    sync = h5_prepare_pkt(h5, sync_pkt,2, H5_LE_PKT);
    BT_DEBUG("sync->buflen %d", sync->buflen);
    while(get_send_state());
    BT_UARTSend(BT_UART_CH, sync->payload, sync->buflen);
    ret = wait_phybusif_output_complete(100000);
    if (ERR_TIMEOUT == ret)
        BT_DEBUG("send sync timeout!\n");

    while(1)
    {
        if(h5->sync_resp_recved)
        {
            break;
        }
        DelayMs(10);
        if(timeout-- == 0)
        {
            if(retry--)
            {
                goto syncretry;
            }
            else
            {
                goto err;
            }
        }
    }
    retry = H5_RETRY;
confretry:

    timeout = 15; //150ms

    conf = h5_prepare_pkt(h5, conf_pkt,3, H5_LE_PKT);
    BT_DEBUG("conf->buflen = %d", conf->buflen);
    while(get_send_state());
    BT_UARTSend(BT_UART_CH, conf->payload, conf->buflen);
    wait_phybusif_output_complete(100000);

    pbuf_free(conf);

    while(1)
    {
        if(h5->conf_resp_recved)
        {
            break;
        }
        DelayMs(10);
        if(timeout-- == 0)
        {
            if(retry--)
            {
                goto confretry;
            }
            else
            {
                goto err;
            }
        }
    }

    pure_ack = h5_prepare_pkt(h5, NULL,0, H5_ACK_PKT);
    BT_DEBUG("pure_ack->buflen = %d", pure_ack->buflen);
    while(get_send_state());
    BT_UARTSend(BT_UART_CH, pure_ack->payload, pure_ack->buflen);
    wait_phybusif_output_complete(100000);

    pbuf_free(pure_ack);
    return 0;

err:
    pbuf_queue_purge(h5->unack);
    pbuf_queue_purge(h5->rel);
    pbuf_queue_purge(h5->unrel);
    return ERR_TIMEOUT;
}
_ATTR_LWBT_UARTIF_CODE_
int h5_close(struct phybusif_cb *hu)
{
    struct h5_struct *h5 = &g_h5;

    BT_DEBUG("hu %p", hu);

    pbuf_queue_purge(h5->unack);
    pbuf_queue_purge(h5->rel);
    pbuf_queue_purge(h5->unrel);
    if (need_check_bt_state)
    {
        int ret = 0;
//        cancel_delayed_work(&bt_state_check_work);   //wp
//        ret = mutex_lock_interruptible(&sem_exit);  //wait work queue perform completely //del by wp
//        if (ret != 0)
//        {
//            BT_DEBUG("Realtek mutex unlocked:%x, %s()\n", ret, __func__);
//        }
        hci_uart_info = NULL;
    }
    return 0;
}

_ATTR_LWBT_UARTIF_CODE_
int h5_init(void)
{
    int err =0;
    //h5_sleep_init();
    return err;
}
_ATTR_LWBT_UARTIF_CODE_
int h5_deinit(void)
{
    //for sleeep check
    //h5_sleep_exit();
    return 0;
}

#ifdef BT_HOST_SNIFF
_ATTR_LWBT_UARTIF_CODE_
int h5_hci_link_mode_manage(void)
{

    if(hci_sniff_manage_enable)
    {
        uint8 mode = hci_get_link_mode(get_last_connect_dev(), HCI_CONNECT_TYPE_ACL);
        if(hci_mode_state_machine == PHY_HCI_ACTIVE_MODE)
        {
            if(mode == PHY_HCI_ACTIVE_MODE)
            {
                if(GetSysTick() > hci_idle_time_tick + 700)
                {
                    BT_DEBUG("hci_sniff_mode\n");
                    hci_sniff_mode((struct bd_addr *)get_last_connect_dev(), 200, 100, 10, 10);
                    hci_mode_state_machine = PHY_HCI_WAIT_SNIFF_MODE;
                }
            }
            else if(mode == PHY_HCI_SINFF_MODE)
            {
                hci_mode_state_machine = PHY_HCI_SINFF_MODE;
            }

            return;
        }

        if(hci_mode_state_machine == PHY_HCI_WAIT_SNIFF_MODE)
        {
            if(mode == PHY_HCI_SINFF_MODE)
            {
                hci_mode_state_machine = PHY_HCI_SINFF_MODE;
            }

            return;
        }

        if(hci_mode_state_machine == PHY_HCI_SINFF_MODE)
        {
            if(mode == PHY_HCI_ACTIVE_MODE)
            {
                hci_mode_state_machine = PHY_HCI_ACTIVE_MODE;
                need_sniff_wakeup = 0;
                hci_idle_time_tick = GetSysTick();
            }
            #if 0
            if(mode == PHY_HCI_SINFF_MODE && need_sniff_wakeup == 1)
            {
                struct hci_link * link;
                link = hci_get_link((struct bd_addr *)get_last_connect_dev(), HCI_CONNECT_TYPE_ACL);
                DEBUG("hci_exit_sniff_mode\n");
                hci_exit_sniff_mode(link->conhdl);
                hci_mode_state_machine = PHY_HCI_WAIT_ACTIVE_MODE;

            }
            #endif
            return;
        }

        if(hci_mode_state_machine == PHY_HCI_WAIT_ACTIVE_MODE)
        {
            if(mode == PHY_HCI_ACTIVE_MODE)
            {
                hci_mode_state_machine = PHY_HCI_ACTIVE_MODE;
                hci_idle_time_tick = GetSysTick();
                if(need_sniff_wakeup)
                {
                    need_sniff_wakeup = 0;
                    hci_uart_tx_wakeup(NULL);
                }
            }
            return;
        }
    }
    else
    {

    }
    return 0;
}

_ATTR_LWBT_UARTIF_CODE_
void h5_hci_link_mode_manage_enable(void)
{
    hci_sniff_manage_enable = 1;
    hci_idle_time_tick = GetSysTick();
}

_ATTR_LWBT_UARTIF_CODE_
void h5_hci_link_mode_manage_disable(void)
{
    hci_sniff_manage_enable = 0;
}
#endif

_ATTR_LWBT_INIT_CODE_
void phybusif_reinit(bt_hw_control_t * hw_control)
{
    memset(&g_cb, 0, sizeof(struct phybusif_cb));
    isOutputComplete = 0;
    HciSeverRequestCount = 0;
    HciServeDisableCount = 0;

    pbuf_init();
    phybusif_reset(&g_cb);
    BT_Recv_Register(h5_recv);
    BT_Tx_Complete_Register(phybusif_output_complete);
    g_cb.hw_conrtol = hw_control;

    IntPendingClear(BT_H5_TX_INT_ID);
    IntRegister(BT_H5_TX_INT_ID , phybusif_h5_data_send);
    IntEnable(BT_H5_TX_INT_ID);
    h5_close(&g_cb);
    //memset(&g_h5, 0, sizeof(struct h5_struct));
}
#endif//BT_UART_INTERFACE_CONFIG == BT_UART_INTERFACE_H5
#endif

