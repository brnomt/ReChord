/*
 * Copyright (c) 2003 EISLAB, Lulea University of Technology.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwBT Bluetooth stack.
 *
 * Author: Conny Ohult <conny@sm.luth.se>
 *
 */

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
#include "BlueToothWin.h"

#include "fsinclude.h"
#include "SysReservedOperation.h"
#ifdef _BLUETOOTH_
_ATTR_LWBT_DATA_ uint8 isOutputComplete = 0;

#ifdef _A2DP_SINK_
_ATTR_LWBT_DATA_ uint8 scoTxBuf[520];
#endif

//struct phybusif_cb *g_cbP;
_ATTR_LWBT_BSS_ struct phybusif_cb g_cb;

_ATTR_LWBT_BSS_ uint8 HciSeverRequestCount;

_ATTR_LWBT_BSS_ uint8 HciServeDisableCount;


void phybusif_output_complete(void);
void HciServeIsrRequest(void);

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

    pbuf_init();
    phybusif_reset(&g_cb);
    BT_Recv_Register(phybusif_input);
    BT_Tx_Complete_Register(phybusif_output_complete);
    g_cb.hw_conrtol = hw_control;

}

#if 0
_ATTR_LWBT_UARTIF_CODE_
int is_get_hci_event(void)
{
    if(g_cb.isGetEvent)
    {
        g_cb.isGetEvent = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}
_ATTR_LWBT_UARTIF_CODE_
int is_get_hci_acl(void)
{
    if(g_cb.isGetACL)
    {
        g_cb.isGetACL = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif


_ATTR_LWBT_UARTIF_CODE_
void hcievent_pbuf_input(struct pbuf *p)
{
    struct pbuf *q;
    if(g_cb.hci_event_unread == NULL)
    {
        g_cb.hci_event_unread = p;
    }
    else
    {
        q = g_cb.hci_event_unread;
        while(q->next)
        {
            q = q->next;
        }

        q->next = p;
    }

}
_ATTR_LWBT_UARTIF_CODE_
struct pbuf * get_hci_event()
{
    struct pbuf *p;

    if((g_cb.hci_event_unread == NULL))
    {
        return NULL;
    }
    else
    {
        IntDisable(BT_UART_INT_ID);
        p = g_cb.hci_event_unread;
        g_cb.hci_event_unread = g_cb.hci_event_unread->next;
        p->next = NULL;
        IntEnable(BT_UART_INT_ID);

        return p;
    }
}
_ATTR_LWBT_UARTIF_CODE_
struct pbuf * get_hci_acl()
{
    struct pbuf *p;

    if((g_cb.hci_acl_unread == NULL))
    {
        return NULL;
    }
    else
    {
        IntDisable(BT_UART_INT_ID);
        p = g_cb.hci_acl_unread;
        g_cb.hci_acl_unread = g_cb.hci_acl_unread->next;
        p->next = NULL;
        IntEnable(BT_UART_INT_ID);

        return p;
    }
}
_ATTR_LWBT_UARTIF_CODE_
void hciacl_pbuf_input(struct pbuf *p)
{
    struct pbuf *q;

    if(g_cb.hci_acl_unread == NULL)
    {
        g_cb.hci_acl_unread = p;
    }
    else
    {
        q = g_cb.hci_acl_unread;

        while(q->next)
        {
            q = q->next;
        }

        q->next = p;
    }
}

#ifdef _A2DP_SINK_
_ATTR_LWBT_UARTIF_CODE_
struct pbuf * get_hci_sco()
{
    struct pbuf *p;

    if((g_cb.hci_sco_unread == NULL))
    {
        return NULL;
    }
    else
    {
        IntDisable(BT_UART_INT_ID);
        p = g_cb.hci_sco_unread;
        g_cb.hci_sco_unread = g_cb.hci_sco_unread->next;
        p->next = NULL;
        IntEnable(BT_UART_INT_ID);

        return p;
    }
}
_ATTR_LWBT_UARTIF_CODE_
void hcisco_pbuf_input(struct pbuf *p)
{
    struct pbuf *q;

    if(g_cb.hci_sco_unread == NULL)
    {
        g_cb.hci_sco_unread = p;
    }
    else
    {
        q = g_cb.hci_sco_unread;

        while(q->next)
        {
            q = q->next;
        }

        q->next = p;
    }
}
#endif

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
void phybusif_output_complete(void)
{
    g_cb.isOutputComplete = 1;
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
_ATTR_LWBT_UARTIF_CODE_
int phybusif_input(unsigned char c)
{
    struct phybusif_cb *cb;
    unsigned char n;

    cb = &g_cb;

    switch(cb->state)
    {
        case W4_WAIT_BUF:
            phybusif_reset(cb);
            break;
        case W4_PACKET_TYPE:
            switch(c)
            {
                case HCI_ACL_DATA_PACKET:
                    cb->state = W4_ACL_HDR;
                    break;
                case HCI_EVENT_PACKET:
                    cb->state = W4_EVENT_HDR;
                    break;

                case HCI_SCO_DATA_PACKET:
                    cb->state = W4_SCO_HDR;
                    break;


                default:
                    if(cb->hw_conrtol)
                    {
                        if(cb->hw_conrtol->vendor_uart_recv)
                        {
                            cb->hw_conrtol->vendor_uart_recv(&c, 1);
                        }
                    }
                    break;
            }
            break;

        case W4_SCO_HDR:

            ((uint8 *)cb->q->payload)[cb->recvd] = c;
            cb->tot_recvd++;
            cb->recvd++;
            if(cb->recvd == HCI_SCO_HDR_LEN)
            {
                cb->scohdr = cb->p->payload;
                pbuf_header(cb->p, -HCI_SCO_HDR_LEN);
                cb->recvd = cb->tot_recvd = 0;
                if(cb->scohdr->len > 0)
                {
                    cb->state = W4_SCO_DATA;
                }
                else
                {
#ifdef _A2DP_SINK_
                    hcisco_pbuf_input(cb->p);
#else
                    pbuf_free(cb->p);
#endif
                    phybusif_reset(cb);
                    HciServeIsrRequest();
                    return ERR_OK; /* Since there most likley won't be any more data in the input buffer */
                }
            }
            break;

            break;

        case W4_SCO_DATA:


            ((uint8 *)cb->q->payload)[cb->recvd] = c;
            cb->tot_recvd++;
            cb->recvd++;

            if(cb->recvd == cb->q->len)
            {
                /* Pbuf full. alloc and add new tail to chain */
                cb->recvd = 0;
                pbuf_free(cb->p);
                phybusif_reset(cb);

            }

            if(cb->tot_recvd == cb->scohdr->len)
            {
#ifdef _A2DP_SINK_
                hcisco_pbuf_input(cb->p);
#else
                pbuf_free(cb->p);
#endif
                phybusif_reset(cb);
                HciServeIsrRequest();
                return ERR_OK; /* Since there most likley won't be any more data in the input buffer */
            }

            break;

        case W4_EVENT_HDR:
            ((uint8 *)cb->q->payload)[cb->recvd] = c;
            cb->tot_recvd++;
            cb->recvd++;
            if(cb->recvd == HCI_EVENT_HDR_LEN)
            {
                cb->evhdr = cb->p->payload;
                pbuf_header(cb->p, -HCI_EVENT_HDR_LEN);
                cb->recvd = cb->tot_recvd = 0;
                if(cb->evhdr->len > 0)
                {
                    cb->state = W4_EVENT_PARAM;
                }
                else
                {
                    hcievent_pbuf_input(cb->p);
                    phybusif_reset(cb);
                    HciServeIsrRequest();

                    return ERR_OK; /* Since there most likley won't be any more data in the input buffer */
                }
            }
            break;

        case W4_EVENT_PARAM:
            ((uint8 *)cb->q->payload)[cb->recvd] = c;
            cb->tot_recvd++;
            cb->recvd++;
            if(cb->recvd == cb->q->len)
            {
                /* Pbuf full. alloc and add new tail to chain */
                cb->recvd = 0;
                pbuf_free(cb->p);
                phybusif_reset(cb);
            }
            if(cb->tot_recvd == cb->evhdr->len)
            {

                hcievent_pbuf_input(cb->p);
                phybusif_reset(cb);
                HciServeIsrRequest();

                return ERR_OK; /* Since there most likley won't be any more data in the input buffer */
            }
            break;

        case W4_ACL_HDR:
            ((uint8 *)cb->q->payload)[cb->recvd] = c;
            cb->tot_recvd++;
            cb->recvd++;
            if(cb->recvd == HCI_ACL_HDR_LEN)
            {
                cb->aclhdr = cb->p->payload;
                pbuf_header(cb->p, -HCI_ACL_HDR_LEN);
                cb->recvd = cb->tot_recvd = 0;
                if(cb->aclhdr->len > 0)
                {
                    cb->state = W4_ACL_DATA;
                }
                else
                {
                    hciacl_pbuf_input(cb->p);
                    phybusif_reset(cb);
                    HciServeIsrRequest();
                    return ERR_OK; /* Since there most likley won't be any more data in the input buffer */
                }
            }
            break;

        case W4_ACL_DATA:
            ((uint8 *)cb->q->payload)[cb->recvd] = c;
            cb->tot_recvd++;
            cb->recvd++;

            if(cb->recvd == cb->q->len)
            {
                /* Pbuf full. alloc and add new tail to chain */
                cb->recvd = 0;
                pbuf_free(cb->p);
                phybusif_reset(cb);

            }

            if(cb->tot_recvd == cb->aclhdr->len)
            {
                hciacl_pbuf_input(cb->p);
                phybusif_reset(cb);
                HciServeIsrRequest();
                return ERR_OK; /* Since there most likley won't be any more data in the input buffer */
            }
            break;

        default:
            break;
    }

    return ERR_OK;
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

/*-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------*/
/* hci_serve():
 *
 * Called by main loop
 *
 */
/*-----------------------------------------------------------------------------------*/

_ATTR_LWBT_BSS_ uint8 hci_serve_busy;

_ATTR_LWBT_UARTIF_CODE_
void hci_serve(void)
{
    struct pbuf * eventP = NULL;
    struct pbuf * aclP = NULL;
    struct pbuf * scoP = NULL;
    if(hci_serve_busy)
    {
        return;
    }
    hci_serve_busy = 1;

    while(1)
    {
        eventP = get_hci_event();
        if(eventP)
        {
            hci_event_input(eventP);

            pbuf_free(eventP);

            //eventP = get_hci_event();
        }

        if(hci_can_send_packet_now())
        {
            aclP = get_hci_acl();
        }
        else
        {
            aclP = NULL;
        }
        if(aclP)
        {
            hci_acl_input(aclP);

            //aclP = get_hci_acl();
        }

#ifdef _A2DP_SINK_
        scoP = get_hci_sco();

        if(scoP)
        {
            hci_sco_input(scoP);

            //scoP = get_hci_sco();
        }
#endif
        if(eventP == NULL && aclP == NULL && scoP == NULL)
        {
            break;
        }

    }


    hci_serve_busy = 0;
}

/*-----------------------------------------------------------------------------------*/
/* hci_serve_pro_int():
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

_ATTR_LWBT_UARTIF_CODE_
void _phybusif_output(struct pbuf *p, uint16 len, void (*func)(void))
{
    struct pbuf *q;
    unsigned char *ptr;
    /* Send pbuf on UART */

    if(g_cb.hw_conrtol!= NULL && g_cb.hw_conrtol->get_power_status)
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
#if 1

    for(q = p; q != NULL; q = q->next)
    {
        ptr = q->payload;
        g_cb.isOutputComplete = 0;
        while(get_send_state());
        if(func == NULL)
        {
            BT_Tx_Complete_Register(phybusif_output_complete);
            BT_UARTSend(BT_UART_CH, ptr,q->len);
            wait_phybusif_output_complete(100000);
        }
        else
        {
            BT_Tx_Complete_Register(func);
            BT_UARTSend(BT_UART_CH, ptr,q->len);
        }


    }
#else
    ptr = p->payload;
    g_cb.isOutputComplete = 0;

    BT_UARTSend(ptr,p->len);

    wait_phybusif_output_complete(100000);

#endif

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




#ifdef _A2DP_SINK_
_ATTR_LWBT_UARTIF_CODE_
void phybusif_sco_output(uint8 *scohdr, uint8 *data, uint16 datalen, void (*func)(void))
{

    memcpy(scoTxBuf, scohdr, 4);
    memcpy(scoTxBuf+4, data, datalen);

    while(get_send_state());
    BT_Tx_Complete_Register(func);
    BT_UARTSend(BT_UART_CH, scoTxBuf,datalen+4);
}

#endif


#endif

