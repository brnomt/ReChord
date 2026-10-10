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
#ifndef __LWIP_DEBUG_H__
#define __LWIP_DEBUG_H__

//#include "lwip/arch.h"
#ifdef _IN_BT_DT_IP
#define BT_EXT
#else
#define BT_EXT extern
#endif

#ifndef BIT
#define BIT(x)      ((uint32)0x00000001 << (x))
#endif

BT_EXT int bt_debug_components;
BT_EXT int bt_debug_level;

#define PHYBUSIF_DEBUG  BIT(1)
#define HCI_DEBUG       BIT(2)
#define HCI_EV_DEBUG    BIT(3)
#define L2CAP_DEBUG     BIT(4)
#define SDP_DEBUG       BIT(5)
#define RFCOMM_DEBUG    BIT(6)
#define LWBT_PPP_DEBUG  BIT(7)
#define NAT_DEBUG       BIT(8)
#define AVCTP_DEBUG     BIT(9)
#define AVRCP_DEBUG     BIT(10)
#define LWBT_BT_DT      BIT(11)
#define AVDVT_DEBUG     BIT(10)



#define BT_BDBUG_COMPONENTS (PHYBUSIF_DEBUG |HCI_DEBUG |HCI_EV_DEBUG| L2CAP_DEBUG| SDP_DEBUG|AVCTP_DEBUG|AVRCP_DEBUG|AVDVT_DEBUG|LWBT_BT_DT)
/** lower two bits indicate debug level
 * - 0 off
 * - 1 info
 * - 2 warning
 * - 3 serious
 * - 4 severe
 */

#define _DBG_DUMP_    0x00
#define _DBG_INFO_    0x01
#define _DBG_WARNING_ 0x02 /* bad checksums, dropped packets, ... */
#define _DBG_SERIOUS_ 0x03 /* memory allocation failures, ... */
#define _DBG_SEVERE_  0x04
#define _DBG_MAX_LEVEL  0x04

#define LWIP_NOASSERT
#define LWIP_ERROR

/** flag for LWIP_DEBUGF to enable that debug message */
#define LWIP_DBG_ON            0x80U
/** flag for LWIP_DEBUGF to disable that debug message */
#define LWIP_DBG_OFF           0x00U

/** flag for LWIP_DEBUGF indicating a tracing message (to follow program flow) */
#define LWIP_DBG_TRACE         0x40U
/** flag for LWIP_DEBUGF indicating a state debug message (to follow module states) */
#define LWIP_DBG_STATE         0x20U
/** flag for LWIP_DEBUGF indicating newly added code, not thoroughly tested yet */
#define LWIP_DBG_FRESH         0x10U
/** flag for LWIP_DEBUGF to halt after printing this debug message */
#define LWIP_DBG_HALT          0x08U

#ifndef LWIP_NOASSERT
#define LWIP_ASSERT(message, assertion) do { if(!(assertion)) \
  LWIP_PLATFORM_ASSERT(message); } while(0)
#else  /* LWIP_NOASSERT */
#define LWIP_ASSERT(message, assertion)
#endif /* LWIP_NOASSERT */

/** if "expression" isn't true, then print "message" and execute "handler" expression */
#ifndef LWIP_ERROR
#define LWIP_ERROR(message, expression, handler) do { if (!(expression)) { \
  LWIP_PLATFORM_ASSERT(message); handler;}} while(0)
#endif /* LWIP_ERROR */
void bt_printf(const char * fmt,...);
#define LWIP_PLATFORM_DIAG(x) bt_printf x
#ifdef LWIP_DEBUG
/** print debug message only if debug message type is enabled...
 *  AND is of correct type AND is at least LWIP_DBG_LEVEL
 */

#define LWIP_DEBUGF(components,level, message) do { \
                               if ( \
                                   ((components) & bt_debug_components) && \
                                   (( level >= bt_debug_level))){ \
                                 LWIP_PLATFORM_DIAG(message); \
                                 } \
                             } while(0)

#else  /* LWIP_DEBUG */
#define LWIP_DEBUGF(debug, message)
#endif /* LWIP_DEBUG */


#ifdef _BT_DEBUG_
#define LWBT_DEBUGF(components,level, message) do { \
                               if ( \
                                   ((components) & bt_debug_components) && \
                                   (level >= bt_debug_level)){ \
                                 bt_printf("\n"); \
                                 bt_printf("%s,L:%d  ",__MODULE__, __LINE__); \
                                 LWIP_PLATFORM_DIAG(message); \
                                 } \
                             } while(0)
#else
#define LWBT_DEBUGF(components,level, message)
#endif
#endif /* __LWIP_DEBUG_H__ */

