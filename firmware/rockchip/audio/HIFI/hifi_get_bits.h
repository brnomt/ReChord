/*
 * copyright (c) 2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * bitstream reader API header.
 */

#ifndef GET_BITS_H_
#define GET_BITS_H_

//#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define INT_MAX       2147483647    /* maximum (signed) int value */
/* bit input */
/* buffer, buffer_end and size_in_bits must be present and used by every reader */
typedef struct GetBitContext {
     unsigned char *buffer, *buffer_end;
    int index;
    int size_in_bits;
} GetBitContext;

#define VLC_TYPE short

typedef struct VLC {
    int bits;
    VLC_TYPE (*table)[2]; ///< code, bits
    int table_size, table_allocated;
} VLC;

typedef struct RL_VLC_ELEM {
    short level;
    char len;
    unsigned char run;
} RL_VLC_ELEM;



#define MIN_CACHE_BITS 25

  unsigned short swap16(unsigned short value);
  unsigned long swap32(unsigned long value);


#define NEG_SSR32(a,s) (((int)(a))>>(32-(s)))
#define NEG_USR32(a,s) (((unsigned int)(a))>>(32-(s)))

#define betoh32(x) swap32(x)

  unsigned int unaligned32( void *v);


  int sign_extend(int val, unsigned int bits);

  int unaligned32_be( void *v);

#define SKIP_OPEN_READER(name, gb)\
		int name##_index= (gb)->index;\

#define OPEN_READER(name, gb)\
        int name##_index= (gb)->index;\
        int name##_cache= 0;\

#define CLOSE_READER(name, gb)\
        (gb)->index= name##_index;\

#define UPDATE_CACHE(name, gb)\
        name##_cache= unaligned32_be( (( unsigned char *)(gb)->buffer)+(name##_index>>3) ) << (name##_index&0x07);\

#define SKIP_CACHE(name, gb, num)\
        name##_cache <<= (num);


// FIXME name?
#define SKIP_COUNTER(name, gb, num)\
        name##_index += (num);\

#define SKIP_BITS(name, gb, num)\
        {\
            SKIP_CACHE(name, gb, num)\
            SKIP_COUNTER(name, gb, num)\
        }\

#define LAST_SKIP_BITS(name, gb, num) SKIP_COUNTER(name, gb, num)
#define LAST_SKIP_CACHE(name, gb, num) ;

#define SHOW_UBITS(name, gb, num)\
        NEG_USR32(name##_cache, num)

#define SHOW_SBITS(name, gb, num)\
        NEG_SSR32(name##_cache, num)

#define GET_CACHE(name, gb)\
        ((unsigned int)name##_cache)

  int get_bits_count( GetBitContext *s);

  void skip_bits_long(GetBitContext *s, int n);
  int get_sbits(GetBitContext *s, int n);

/**
 * Read 1-25 bits.
 */
  unsigned int get_bits(GetBitContext *s, int n);

/**
 * Shows 1-25 bits.
 */
  unsigned int show_bits(GetBitContext *s, int n);

  void skip_bits(GetBitContext *s, int n);

  unsigned int get_bits1(GetBitContext *s);

  unsigned int show_bits1(GetBitContext *s);

  void skip_bits1(GetBitContext *s);

/**
 * reads 0-32 bits.
 */
  unsigned int get_bits_long(GetBitContext *s, int n);
  int get_sbits_long(GetBitContext *s, int n) ;

  int init_get_bits(GetBitContext *s, unsigned char *buffer, int bit_size);

  void align_get_bits(GetBitContext *s);
  #if 1

  int decode012(GetBitContext *gb);

  int decode210(GetBitContext *gb);

  int get_bits_left(GetBitContext *gb);
   int init_get_bits8(GetBitContext *s,   unsigned char *buffer,int byte_size);
    int get_unary_0_9(GetBitContext *gb);
	 int my_bui_clz(unsigned int a);
  #endif 
#endif




