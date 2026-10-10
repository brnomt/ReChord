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
//#include "BlueToothControl.h"
#ifndef __LWIP_INET_H__
#define __LWIP_INET_H__

/* For compatibility with BSD code */
typedef __packed struct _IN_ADDR
{
    uint32 s_addr;

} IN_ADDR;

#define INADDR_NONE         ((uint32)0xffffffffUL)  /* 255.255.255.255 */
#define INADDR_LOOPBACK     ((uint32)0x7f000001UL)  /* 127.0.0.1 */
#define INADDR_ANY          ((uint32)0x00000000UL)  /* 0.0.0.0 */
#define INADDR_BROADCAST    ((uint32)0xffffffffUL)  /* 255.255.255.255 */

uint32 inet_addr(const char *cp);
int inet_aton(const char *cp, IN_ADDR *addr);
char *inet_ntoa(IN_ADDR addr); /* returns ptr to static buffer; not reentrant! */

#ifdef htons
#undef htons
#endif /* htons */
#ifdef htonl
#undef htonl
#endif /* htonl */
#ifdef ntohs
#undef ntohs
#endif /* ntohs */
#ifdef ntohl
#undef ntohl
#endif /* ntohl */

#ifndef LWIP_PLATFORM_BYTESWAP
#define LWIP_PLATFORM_BYTESWAP 0
#endif

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif

#define BYTE_ORDER  LITTLE_ENDIAN

#if BYTE_ORDER == BIG_ENDIAN
#define htons(x) (x)
#define ntohs(x) (x)
#define htonl(x) (x)
#define ntohl(x) (x)
#else /* BYTE_ORDER != BIG_ENDIAN */
uint16 htons(uint16 x);
uint16 ntohs(uint16 x);
uint32 htonl(uint32 x);
uint32 ntohl(uint32 x);
#endif /* LWIP_PLATFORM_BYTESWAP */

#endif /* __LWIP_INET_H__ */
