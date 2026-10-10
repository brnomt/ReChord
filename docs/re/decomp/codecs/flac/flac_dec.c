#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/codecs/flac/flac_dec.c
 * Reconstructed from Ghidra decompilation of:
 *   hifi_flac_dec    @ 0x030df64c
 *   hifi_flac_helper @ 0x030df52a
 *   flac_bitstream_getbits_u @ 0x030dd736
 *   flac_bitstream_getbits_s @ 0x030dd6f6
 *   Source: ..\..\Common\Codec\Audio\HIFI\flac\hifi_flacdec.c
 *
 * Fixed-point FLAC decoder from Rockchip HIFI library.
 * Uses bitstream reader and subframe decoder pattern.
 *
 * Decoder flow:
 *   1. Align bitstream reader (hifi_flac_helper)
 *   2. Read subframe type (0=constant, 1=verbatim, 8=subframe type + wasted bits)
 *   3. Read subframe. If type=0x1F: residual coding (Rice/Golomb)
 *   4. If type!=0x1F: fixed/lpc subframe decode
 *   5. Loop per subframe → per channel → per frame
 */

#include "../../firmware.h"
#include "../../firmware/os/os_api.h"
#include "../codec_api.h"

/* FLAC frame decode state */
typedef struct {
    uint32_t bitstream;       /* +0x20: pointer to bitstream buffer */
    uint32_t bit_offset;      /* +0x28: current bit position (global bit count) */
    uint32_t sample_rate;     /* +0x30: samples per channel */
} FLACFrameCtx;

/*
 * hifi_flac_helper() — align bitstream to byte boundary
 * @ 0x030df52a
 *
 * Aligns the bit reader to the next N-bit boundary.
 * Parameters:
 *   +0x20: bitstream ptr
 *   param2: alignment (6, 10, 100 = subframe)
 */
void hifi_flac_helper(FLACFrameCtx *ctx, int alignment) {
    if (alignment >= 0x100) {
        /* Byte-align the bitstream */
        ctx->bit_offset = (ctx->bit_offset + 7) & ~7;
    } else {
        /* Align to N-bit boundary */
        ctx->bit_offset = (ctx->bit_offset + alignment - 1) & ~(alignment - 1);
    }
}

/*
 * hifi_flac_dec() — decode FLAC frame to PCM
 * @ 0x030df64c
 *
 * Parameters:
 *   ctx:   FLACFrameCtx with bitstream + metadata
 *   param2: channel index (0 or 0x4800 stride)
 *   param3: sample count to produce
 *
 * Returns 0 on success, -1 on error.
 *
 * Uses residual coding with Rice parameters for compressed audio.
 */
int hifi_flac_dec(FLACFrameCtx *ctx, int channel_stride, int sample_count) {
    int i, j;
    uint32_t bps, subframe_type, wasted_bits;
    uint32_t *output;
    
    /* Align to byte boundary + 6-bit */
    hifi_flac_helper(ctx, 6);  /* align to 6-bit */
    
    /* Read subframe header (2 bits: 0=constant, 1=verbatim, 8+ = coded) */
    int residual_blk = rom_bitstream_read(ctx->bitstream + 0x20, 2);
    
    if (residual_blk > 1) {
        debug_printf("FILE: %s, LINE: %d: #  Error #",
            "..\\..\\Common\\Codec\\Audio\\HIFI\\flac\\hifi_flacdec.c", 0x150);
        return -1;
    }
    
    /* Read bits-per-sample */
    bps = rom_bitstream_read(ctx->bitstream + 0x20, 4);
    
    /* Decode residual sample count (shifted down by bps bits) */
    int residual_samples = ctx->sample_rate >> (bps & 0xFF);
    
    if (residual_samples < sample_count) {
        debug_printf("FILE: %s, LINE: %d: #  Error #",
            "..\\..\\Common\\Codec\\Audio\\HIFI\\flac\\hifi_flacdec.c", 0x159);
        return -1;
    }
    
    /* Calculate samples per subframe block */
    int subframe_count = 0;
    int block_size = 1 << (bps & 0xFF);
    
    if (block_size <= 0) return 0;
    
    /* For each sub-block */
    output = (uint32_t *)((uint8_t *)ctx + channel_stride * 0x4800 + 0x48);
    
    for (j = 0; j < block_size; j++) {
        int cur_sample = 0;
        
        /* Align to 10-bit for subframe type */
        hifi_flac_helper(ctx, 10);
        
        /* Determine subframe type bits */
        int type_bits;
        if (residual_blk == 0) {
            type_bits = 4;
        } else {
            type_bits = 5;
        }
        
        /* Read subframe type */
        subframe_type = rom_bitstream_read(ctx->bitstream + 0x20, type_bits);
        
        /* Maximum subframe type check */
        uint32_t max_type;
        if (residual_blk == 0) {
            max_type = 0xF;   /* 4 bits */
        } else {
            max_type = 0x1F;  /* 5 bits */
        }
        
        if (subframe_type == max_type) {
            /* Residual coding mode (type=0xF or 0x1F indicates residual) */
            uint32_t order = rom_bitstream_read(ctx->bitstream + 0x20, 5);
            
            /* Read residual values for all samples in this subframe */
            for (; cur_sample < residual_samples; cur_sample++) {
                hifi_flac_helper(ctx, order);
                
                /* Read signed Rice-coded residual */
                uint32_t value = rom_bitstream_read_signed(ctx->bitstream + 0x20, order);
                output[cur_sample * 4] = value;
            }
            
        } else {
            /* Fixed or LPC subframe */
            
            /* Read warm-up samples or LPC coefficients */
            for (; cur_sample < residual_samples; cur_sample++) {
                hifi_flac_helper(ctx, 0x100);  /* byte-align */
                
                /* Read next bit from stream */
                uint32_t bit_pos = ctx->bit_offset;
                uint32_t byte_val = rom_peek_byte(
                    ctx->bitstream + (bit_pos >> 3)
                );
                uint32_t bit_val = byte_val << (bit_pos & 7);
                
                /* Count leading zeros (Rice parameter) */
                int clz = rom_clz(bit_val | 1);
                int rice_param = 31 - clz;
                
                /* Calculate residual value with Rice/Golomb coding */
                int rice_bits = rice_param - subframe_type;
                uint32_t residual;
                
                if (rice_bits < 7) {
                    /* Short residual — direct shift */
                    residual = (bit_val >> (rice_bits & 0xFF)) + 
                               ((30 - rice_param) << (subframe_type & 0xFF));
                    ctx->bit_offset = (subframe_type - rice_param) + bit_pos + 32;
                } else {
                    /* Long residual — read extra bits */
                    int extra_bits = 32 - rice_param;
                    
                    if ((extra_bits != 0x7FFFFFFF) && (extra_bits >= 0)) {
                        /* Read extra bits one byte at a time */
                        int extra_count = 0;
                        uint32_t val = bit_val;
                        
                        do {
                            if ((int)val < 0) break;
                            bit_pos++;
                            val = rom_peek_byte(ctx->bitstream + (bit_pos >> 3));
                            val = val << (bit_pos & 7);
                            extra_count++;
                        } while (extra_count < 0x7FFFFFFF);
                        
                        bit_pos++;
                        val = val << 1;
                        
                        if (extra_count == 0x7FFFFFFF) {
                            val = 0xFFFFFFFF;
                        }
                        
                        if (extra_count < 0x7FFFFFFE) {
                            if (subframe_type == 0) {
                                val = 0;
                            } else {
                                val = val >> (32 - subframe_type);
                                bit_pos += subframe_type;
                            }
                            ctx->bit_offset = bit_pos;
                            val = val + (extra_count << (subframe_type & 0xFF));
                        } else if (extra_count == 0x7FFFFFFE) {
                            val = 1;
                            ctx->bit_offset = bit_pos;
                        }
                        
                        residual = val;
                    } else {
                        /* Compact residual */
                        residual = (bit_val >> (rice_bits & 0xFF)) + 
                                   (30 - rice_param) << (subframe_type & 0xFF);
                        ctx->bit_offset = (subframe_type - rice_param) + bit_pos + 32;
                    }
                }
                
                /* Decode signed value from Rice/Golomb-coded residual */
                output[cur_sample * 4] = -(residual & 1) ^ ((int)residual >> 1);
            }
        }
        
        /* Reset sample counter for next subframe */
        cur_sample = 0;
        subframe_count++;
    }
    
    return 0;
}

/*
 * flac_bitstream_getbits_u @ 0x030dd736 — unsigned N-bit pull from FLAC bank reader.
 * flac_bitstream_getbits_s @ 0x030dd6f6 — signed sibling (sign-extend).
 */
uint32_t flac_bitstream_getbits_u(void *br, int nbits)
{
    extern uint32_t rom_bitstream_read(uint32_t ctx, int nbits);
    return rom_bitstream_read((uint32_t)(uintptr_t)br, nbits);
}

int32_t flac_bitstream_getbits_s(void *br, int nbits)
{
    uint32_t u = flac_bitstream_getbits_u(br, nbits);
    if (nbits == 0)
        return 0;
    uint32_t sign = 1u << (nbits - 1);
    if (u & sign)
        return (int32_t)(u | (~0u << nbits));
    return (int32_t)u;
}
