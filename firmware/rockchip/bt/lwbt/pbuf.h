/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include "SysConfig.h"
//#include "BlueToothWin.h"
//#include "BlueToothControl.h"
#ifndef __LWIP_PBUF_H__
#define __LWIP_PBUF_H__


#define PBUF_TRANSPORT_HLEN 20
#define PBUF_IP_HLEN        20
#define PBUF_PHY_HLEN       20
#define PBUF_ETHNET_HLEN    14
typedef enum
{
    PBUF_TRANSPORT,
    PBUF_IP,
    PBUF_ETHNET,
    PBUF_PHY,
    PBUF_RAW

} pbuf_layer;

typedef enum
{
    PBUF_POOL, /* pbuf payload refers to RAM */
    PBUF_RAM
} pbuf_type;

#ifdef _A2DP_SINK_
#if BT_UART_INTERFACE_CONFIG == BT_UART_INTERFACE_H5
#define PHY_FRAME_MAX_SIZE (1064+64)*2 //20 + 20 + 14 + 4 + 20 + 8+8, h5 esc code
#else
#define PHY_FRAME_MAX_SIZE (1064+64) //20 + 20 + 14 + 4 + 20 + 8+8
#endif
//#define PHY_FRAME_MAX_SIZE (1064+64) //20 + 20 + 14 + 4 + 20 + 8+8
//#define PHY_FRAME_MAX_SIZE (672+64) //20 + 20 + 14 + 4 + 20 + 8+8

#else
//#define PHY_FRAME_MAX_SIZE (672+64) //20 + 20 + 14 + 4 + 20 + 8+8
#if BT_UART_INTERFACE_CONFIG == BT_UART_INTERFACE_H5
#define PHY_FRAME_MAX_SIZE (672+64)*2 //20 + 20 + 14 + 4 + 20 + 8+8, h5 esc code
#else
#define PHY_FRAME_MAX_SIZE (672+64) //20 + 20 + 14 + 4 + 20 + 8+8
#endif
#endif

#define PBUF_POOL_BUFSIZE  PHY_FRAME_MAX_SIZE

typedef __packed  struct pbuf
{
    /** next pbuf in singly linked pbuf chain */
    struct pbuf *next;

    /** pointer to the actual data in the buffer */
    void *payload;

    /** length of this buffer */

    uint16 tcplen;
    uint16 len;
    uint16 tot_len;
	uint16 buflen;
    /** byte stream entry in this block**/
    uint16 streamStart;

    /** byte stream size in this block**/
    uint16 streamlen;

    /** misc flags */

    uint8 ref;
    uint8 flags;
    uint8 pkt_type;
// TCP_HDR *tcphdr;


    void (*callback)(void);
    uint8 buf[PHY_FRAME_MAX_SIZE];


} PBUF;


/** indicates this packet's data should be immediately passed to the application */
#define PBUF_FLAG_PUSH 0x01U

/* add __packed, jan@2009-11-24 */
extern void *FrameBuf();
extern void  pbuf_init(void);
extern PBUF *pbuf_alloc(pbuf_layer l, uint16 size, pbuf_type type);
extern uint8 pbuf_free(PBUF *p);
extern void  pbuf_cat(PBUF *h, PBUF *t);
extern void pbuf_chain(struct pbuf *h, struct pbuf *t);
extern struct pbuf * pbuf_dechain(struct pbuf *p);
extern void pbuf_ref(struct pbuf *h);
extern void pbuf_ncopy(struct pbuf *p_to, struct pbuf *p_from, int len);
extern int pbuf_queue_len(struct pbuf **head);
extern int pbuf_queue_empty(struct pbuf **head);
extern void pbuf_queue_head(struct pbuf **head,struct pbuf *p);
extern void pbuf_queue_tail(struct pbuf **head,struct pbuf *p);
extern PBUF* pbuf_dequeue_head(struct pbuf **head);
extern PBUF* pbuf_dequeue_tail(struct pbuf **head);

#define  pbuf_dequeue(h) pbuf_dequeue_head(h)
#define  pbuf_queue_purge(p) pbuf_free(p)

#endif /* __LWIP_PBUF_H__ */
