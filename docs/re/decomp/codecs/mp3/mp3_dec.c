#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * firmware/codecs/mp3/mp3_dec.c
 * Reconstructed from Ghidra decompilation of:
 *   mp3_dec_internal @ 0x0306e5de
 *   Source: ..\..\Common\Codec\Audio\Mp3\libMad\mp3_decinternal.c
 *
 * Fixed-point MP3 decoder based on libMAD (MPEG Audio Decoder).
 * Works with MAD stream struct (mad_stream), synth struct (mad_synth),
 * and frame struct (mad_frame).
 *
 * Decoder architecture:
 *   1. mp3_id3v2_handler — skip ID3v2 tag at file start
 *   2. mad_stream_init — initialize bitstream reader
 *   3. mad_frame_decode — decode one MP3 frame
 *   4. mad_synth_frame — synthesize PCM samples
 *   5. Loop until EOF or error
 *
 * MAD stream struct (offset-based):
 *   +0x00: buffer pointer (next byte to process)
 *   +0x04: bufend pointer (end of buffer)
 *   +0x10: error code
 *   +0x14: sync pointer (where sync was found)
 *   +0x1C: this_frame pointer
 *   +0x20: next_frame pointer
 *   +0x30: bitrate
 *   +0x34: samplerate
 *   +0x38: framelength
 */

#include "../../firmware.h"
#include "../../firmware/os/os_api.h"
#include "../codec_api.h"

/* MAD API (simulated from decompilation) */
#define MAD_ERROR_NONE      0x00
#define MAD_ERROR_SYNC      0x10
#define MAD_ERROR_BADDATA   0x11
#define MAD_ERROR_LOSTSYNC  0x20

typedef struct {
    uint8_t *buffer;
    uint8_t *bufend;
    uint32_t error;
    uint32_t bitrate;
    uint32_t samplerate;
    uint32_t framelength;
    uint16_t channels;
    uint8_t  reserved[0x30];
} MADStream;

typedef struct {
    uint16_t status;         /* 0x00: 0=idle, 1=playing, 0xFF00=error */
    uint32_t pcm_length;     /* 0x04: total PCM samples decoded */
    int16_t  pcm_buf[0x1200];/* 0x08: left channel PCM */
    int16_t  pcm_buf_r[0x1200]; /* right channel */
} MADSynth;

typedef struct {
    uint32_t next_frame;     /* 0x14 */
    uint32_t frame_offset;   /* 0x18 */
    uint32_t main_data_begin;/* 0x1C - callback to MAD decoder */
    uint32_t decode_cb;      /* 0x1C - actual decode function ptr */
} MADFrame;

/*
 * mp3_dec_internal() — decode MP3 file to PCM
 * @ 0x0306e5de
 *
 * Parameters:
 *   param1: 0 = normal, 1 = skip ID3 first
 *
 * Returns 1 on success, 0 on error.
 *
 * Output: PCM data in g_mp3_decode_buf (left + right channels, 16-bit)
 */
int mp3_dec_internal(int skip_id3) {
    MADStream *stream = &g_mp3_stream;
    MADSynth  *synth  = &g_mp3_synth;
    MADFrame  *frame  = &g_mp3_frame;
    
    /* Initialize state */
    synth->status = 0;
    synth->pcm_length = 0;
    frame->main_data_begin = 1;
    stream->next_frame = 0;
    stream->frame_offset = 0;
    
    /* Allocate PCM buffers (left + right, 0x1200 samples each) */
    int16_t *pcm_left  = (int16_t *)rom_mem_alloc(g_mp3_buf_base + g_mp3_buf_size * 2, 0x1200);
    int16_t *pcm_right = (int16_t *)rom_mem_alloc(g_mp3_buf_base + g_mp3_buf_size * 2 + 0x1200, 0x1200);
    
    /* Save PCM buffer pointers to decode context */
    g_mp3_decode_ctx = pcm_left;
    synth->pcm_buf = pcm_left;
    
    /* Seek to start of file */
    rom_file_seek(g_mp3_fd, 0);
    
    /* Skip ID3v2 tag if present */
    if (skip_id3 == 1) {
        if (mp3_id3v2_handler(stream) == 0) {
            debug_printf("FILE: %s, LINE: %d: ",
                "..\\..\\Common\\Codec\\Audio\\Mp3\\libMad\\mp3_decinternal.c", 0x16D);
            return 0;
        }
    }
    
    /* Log start position */
    debug_printf("mp3 start pos =%d", g_mp3_start_offset);
    rom_file_seek(g_mp3_fd, g_mp3_start_offset);
    
    /* Initialize MAD stream */
    g_mp3_stream_buffer = g_mp3_stream_base;
    g_mp3_stream_size = 0x800;  /* 2048 bytes sync buffer */
    
    /* Initialize MAD decoder callback chain */
    g_mp3_callbacks[0] = g_mp3_cb_ptr;   /* output handler */
    /* Set up MAD decode pipeline */
    rom_mad_init(g_mp3_callbacks + 2, g_mp3_callbacks, 
                 0x0306e499,              /* header decode */
                 NULL, NULL,              
                g_mp3_decode_cb,          /* frame decode callback */
                g_mp3_synth_cb,           /* synth callback */
                NULL);
    
    /* Check if decoder initialized */
    if (stream->frame_offset == 0) {
        return 0;
    }
    
    /* Resolve function pointers from stream */
    uint32_t decode_fn = stream->main_data_begin;
    if (decode_fn == 0) {
        decode_fn = 0x0306e513;  /* default decoder */
    }
    frame->decode_cb  = decode_fn;
    
    uint32_t frame_data_ptr = g_mp3_frame_data;
    if (decode_fn != 0) {
        frame_data_ptr = stream->decode_cb;
    }
    frame->next_frame = frame_data_ptr;
    
    /* Set up stream input callback */
    uint32_t input_cb = g_mp3_input_ptr;
    stream->main_data_begin = input_cb;
    frame->decode_cb   = input_cb;
    
    /* Configure PCM output pointers */
    synth->pcm_buf     = input_cb + 0x40;
    synth->pcm_buf_r   = input_cb + 0x84;
    
    /* Initialize MAD sub-decoders */
    rom_mad_header_init();    /* FUN_030706e8 */
    rom_mad_layer3_init(synth->pcm_buf);     /* FUN_0306f392 */
    rom_mad_huffman_init(synth->pcm_buf_r);  /* FUN_03070ab2 */
    
    /* Copy stream config to decoder state */
    g_mp3_state.cfg = stream->samplerate;
    rom_mad_start(stream->decode_cb, stream->decode_cb);
    
    /* Main decode loop */
    int retry = 0x400;  /* 1024 iterations max */
    uint32_t stream_buf = g_mp3_buf_base + g_mp3_buf_size * 2;
    frame->decode_cb  = stream_buf;
    frame->next_frame = stream_buf;
    
    /* Set buffer pointers in synth */
    *(uint32_t *)(g_mp3_synth_ptr + 0x2C) = stream_buf;
    *(uint32_t *)(g_mp3_synth_ptr + 0x30) = stream_buf;
    *(uint32_t *)(g_mp3_synth_ptr + 0x34) = 0;
    *(uint32_t *)(g_mp3_synth_ptr + 0x38) = 0;
    
    do {
        while (synth->status != 1 ||
               rom_mad_decode_frame(stream->decode_cb, stream->decode_cb) == 0) {
            /* Wait for data */
            do {
                retry--;
                if (retry == 0) goto decode_error;
                
                int result = rom_mad_bitstream_fill(
                    g_mp3_synth_ptr,
                    g_mp3_state_ptr
                );
                
                if (result != -1) {
                    frame->frame_offset = 0;
                    goto frame_decoded;
                }
                
                uint16_t status = synth->status;
                if (status == 1) break;
                
                if ((status & 0xFF00) == 0) goto decode_error;
                
                result = rom_mad_header_sync(
                    frame->decode_cb,
                    frame->next_frame,
                    g_mp3_state_ptr,
                    g_mp3_synth_ptr
                );
                
                if (result != 0) {
                    if (result == MAD_ERROR_BADDATA) goto frame_decoded;
                    if (result == MAD_ERROR_LOSTSYNC) goto decode_error;
                    if (result != MAD_ERROR_SYNC) goto frame_decoded;
                }
            } while (true);
        }
        
        /* Frame successfully decoded */
        if (frame->decode_cb == MAD_ERROR_BADDATA) break;
        if (frame->decode_cb == MAD_ERROR_LOSTSYNC) {
decode_error:
            rom_mad_cleanup();  /* FUN_0306f378 */
            return 0;
        }
    } while (synth->status == 1);
    
frame_decoded:
    /* Extract decoded metadata */
    uint32_t synth_ptr = g_mp3_synth_ptr;
    g_mp3_decode_ctx = *(uint32_t *)(synth_ptr + 0x10);
    uint32_t total_samples = *(uint32_t *)(synth_ptr + 0x0C);
    g_mp3_frame_count = total_samples;
    
    /* Determine channel count */
    uint8_t channels;
    if (*(uint8_t *)(synth_ptr + 1) == 0) {
        channels = 1;  /* mono */
    } else {
        channels = 2;  /* stereo */
    }
    g_mp3_channels = channels;
    
    /* Calculate bitrate */
    uint32_t bitrate;
    if (total_samples < 1) {
        bitrate = 60000;  /* unknown bitrate sentinel */
    } else {
        uint32_t file_size = rom_file_tell(g_mp3_fd);
        bitrate = rom_mad_bitrate_calc(file_size * 8000, total_samples);
    }
    g_mp3_bitrate = bitrate;
    
    /* Update global state with first track metadata */
    if (g_mp3_state.stereo_mode == 0) {
        g_mp3_state.stereo_mode = g_mp3_decode_ctx;
    }
    
    if (g_mp3_state.total_samples == 0) {
        g_mp3_state.total_samples = total_samples;
    }
    
    if (g_mp3_state.channels == 0) {
        g_mp3_state.channels = channels;
    }
    
    /* Calculate duration */
    uint32_t duration;
    if (g_mp3_state.total_samples == 0) {
        duration = 60000;
        g_mp3_state.duration_ms = 60000;
    } else if (g_mp3_state.vbr_flag == 0) {
        /* VBR file */
        uint32_t file_size = rom_file_tell(g_mp3_fd);
        duration = rom_mad_bitrate_calc(file_size * 8000, g_mp3_state.total_samples);
        g_mp3_state.duration_ms = duration;
    } else {
        /* CBR file — use bitrate */
        uint32_t encoded_size = g_mp3_state.vbr_flag * 8000;
        duration = rom_mad_bitrate_calc(encoded_size, 0);
        g_mp3_state.duration_ms = duration;
    }
    
    /* Log decode results */
    debug_printf("length=%d fileLen=%d bitrate=%d",
        g_mp3_state.vbr_flag,
        rom_file_tell(g_mp3_fd),
        g_mp3_state.total_samples);
    
    /* Set default buffer: 0x480 = 1152 samples per frame */
    if (g_mp3_buf_size_param == 0) {
        g_mp3_buf_size_param = 0x480;
    }
    
    /* Store final decode state */
    g_mp3_decode_ctx  = 0x480;
    g_mp3_state.frame_size = 0x480;
    g_mp3_state.frame_offset = 0x480;
    
    g_mp3_decode_ctx = g_mp3_state.stereo_mode;
    g_mp3_frame_count = g_mp3_state.total_samples;
    g_mp3_channels = g_mp3_state.channels;
    g_mp3_bitrate = g_mp3_state.duration_ms;
    g_mp3_buf_size_param = 0x480;
    
    return 1;
}
