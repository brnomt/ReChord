/*
 * ALAC encoder and decoder common data
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



#include <stdio.h>
#include "../hifi_get_bits.h"
#include "codec.h"

#define HIFI_ACC
#define ALAC_MAX_CHANNELS 8

#define MAX_SAMPLE_PER_FRAME 4096
 
#define BYTESWAP(a) (((a >> 24) & 0x000000ff) | ((a >> 8) & 0x0000ff00) | ((a << 8) & 0x00ff0000) | (a << 24))

#define WORDSWAP(a) (((a >> 8) & 0x00ff) | (a << 8))

#define FFSIGN(a) ((a) > 0 ? 1 : -1)

#define sign_only( v) 	(v ? FFSIGN(v) : 0)

#define FFMIN(a,b) ((a) > (b) ? (b) : (a))

#define av_log2(x) (31 - my_bui_clz((x)|1))

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef signed char  int8_t;
typedef signed short int16_t;
typedef signed int   int32_t;
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

typedef unsigned int size_t;
typedef signed long long   int64_t;
typedef unsigned long long uint64_t;



typedef struct {    
    GetBitContext gb;
    int channels;
    uint32_t  average_bitrate;
	uint32_t samplerate ;
#if 1
    int32_t *predict_error_buffer[2];
    int32_t *output_samples_buffer[2];
    int32_t *extra_bits_buffer[2];
#else
   int32_t predict_error_buffer[2][MAX_SAMPLE_PER_FRAME];
   int32_t output_samples_buffer[2][MAX_SAMPLE_PER_FRAME];
   int32_t extra_bits_buffer[2][MAX_SAMPLE_PER_FRAME];

#endif 
    uint32_t max_samples_per_frame;
    
    uint8_t  sample_size;
    uint8_t  rice_history_mult;
    uint8_t  rice_initial_history;
    uint8_t  rice_limit;
    int extra_bits;     /**< number of extra bits beyond 16-bit */
    int nb_samples;     /**< number of samples in the current frame */

    int direct_output;
} ALACContext;

#if 0
typedef struct AVPacket {  
    unsigned char *data;
    int   size; 
} AVPacket;
#endif 
typedef struct {
	uint32_t size;
	uint32_t sign;
	uint32_t version;	
	uint32_t max_samples_per_frame;
    uint8_t  compatible_version;
	uint8_t  sample_size;
	uint8_t  rice_history_mult;
	uint8_t  rice_initial_history;
	uint8_t  rice_limit;
	uint8_t  channels;
	uint16_t  maxRun;
	uint32_t  max_frame_size;
	uint32_t  average_bitrate;
	uint32_t samplerate ;
} ALACInfo;



enum AlacRawDataBlockType {
    /* At the moment, only SCE, CPE, LFE, and END are recognized. */
    TYPE_SCE,
    TYPE_CPE,
    TYPE_CCE,
    TYPE_LFE,
    TYPE_DSE,
    TYPE_PCE,
    TYPE_FIL,
    TYPE_END
};


extern  uint8_t ff_alac_channel_layout_offsets[ALAC_MAX_CHANNELS][ALAC_MAX_CHANNELS];
extern int alac_decode_init(ALACContext *alac);
extern int alac_decode_frame(ALACContext *alac, void *data, AVPacket *avpkt);
