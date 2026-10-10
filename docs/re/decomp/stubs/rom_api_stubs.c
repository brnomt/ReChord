#include "decomp_support.h"
#include "decomp_globals.h"
/*
 * Weak ROM API stubs for host compile / link tests.
 * Not used on device when calls resolve to boot ROM @ 0x02FE0000.
 */

#include "../rom_api.h"

#define STUB0(name) \
    __attribute__((weak)) void name(void) { }

#define STUB1(name, t1) \
    __attribute__((weak)) void name(t1 a) { (void)a; }

#define STUB2(name, t1, t2) \
    __attribute__((weak)) void name(t1 a, t2 b) { (void)a; (void)b; }

#define STUB3(name, t1, t2, t3) \
    __attribute__((weak)) void name(t1 a, t2 b, t3 c) { (void)a; (void)b; (void)c; }

#define STUB4(name, t1, t2, t3, t4) \
    __attribute__((weak)) void name(t1 a, t2 b, t3 c, t4 d) { (void)a; (void)b; (void)c; (void)d; }

#define STUB_RET(name, ret) \
    __attribute__((weak)) ret name(void) { return (ret)0; }

#define STUB_RET1(name, ret, t1) \
    __attribute__((weak)) ret name(t1 a) { (void)a; return (ret)0; }

#define STUB_RET2(name, ret, t1, t2) \
    __attribute__((weak)) ret name(t1 a, t2 b) { (void)a; (void)b; return (ret)0; }

#define STUB_RET3(name, ret, t1, t2, t3) \
    __attribute__((weak)) ret name(t1 a, t2 b, t3 c) { (void)a; (void)b; (void)c; return (ret)0; }

#define STUB_RET4(name, ret, t1, t2, t3, t4) \
    __attribute__((weak)) ret name(t1 a, t2 b, t3 c, t4 d) { \
        (void)a; (void)b; (void)c; (void)d; return (ret)0; \
    }

__attribute__((weak)) void *rom_alloc(uint32_t size) { (void)size; return (void *)0; }
STUB1(rom_hw_init, uint32_t)
STUB1(rom_hw_init2, uint32_t)
STUB0(rom_early_init)

STUB3(rom_memcpy, void *, const void *, uint32_t)
STUB2(rom_memzero, uint32_t, uint32_t)
STUB_RET2(rom_mem_alloc, void *, uint32_t, uint32_t)
STUB_RET3(rom_memcmp, int, const void *, const void *, int)
STUB_RET3(rom_memcmp_n, int, const void *, const void *, int)
STUB_RET3(rom_memmem, int, const void *, const void *, int)
STUB_RET2(rom_strcmp, int, const char *, const char *)
STUB2(rom_dma_or_copy, uint32_t, uint32_t)

STUB0(rom_audio_cleanup)
STUB0(rom_peripheral_reset)
STUB0(rom_i2s_master_config)
STUB4(rom_i2s_master_init, int, int, int, int)
STUB0(rom_dma_buffer_config)
STUB2(rom_dma_config, int, int)
STUB2(rom_dma_channel_set, int, int)
STUB1(rom_dac_gain_set, uint32_t)
STUB2(rom_dac_gain_set_channel, int, uint32_t)
STUB2(rom_buffer_calculate, uint32_t *, uint32_t *)
STUB_RET1(rom_buffer_ready, int, int)
STUB3(rom_dma_set_source, uint32_t, uint32_t, uint32_t)
STUB0(rom_clock_divider_set)
STUB0(rom_pll_reconfig)
STUB1(rom_audio_path_set_441, uint8_t)
STUB1(rom_dac_output_enable, uint8_t)
STUB3(rom_audio_path_set, int, int, int)
STUB3(rom_audio_path_enable, int, int, int)
STUB3(rom_audio_path_select, int, int, int)
STUB2(rom_audio_path_disable, int, int)
STUB4(rom_playback_start, int, int, int, uint32_t)
STUB4(rom_dma_start, int, uint32_t, uint32_t, uint32_t)
STUB0(rom_bt_playback_init)
STUB0(rom_volume_sync)
STUB2(rom_dac_unmute, int, int)
STUB2(rom_dac_mute, int, int)
STUB0(rom_audio_clock_off)
STUB4(rom_sample_rate_set, int, int, int, uint32_t)
STUB2(rom_dsp_param_set, uint32_t, uint32_t)
STUB0(rom_dsp_start)
STUB0(rom_dsp_bypass_disable)
STUB2(rom_i2s_channel_setup, int, uint32_t)
STUB2(rom_i2s_format_set, int, int)
STUB2(rom_i2s_dma_start, int, int)
STUB_RET2(rom_bt_stream_open, int, void *, int)
STUB1(rom_dac_volume_set, uint8_t)
STUB1(rom_dac_output_config, uint32_t)
STUB1(rom_dac_commit, uint8_t)
STUB3(rom_spi_write_reg, uint32_t, uint32_t, uint32_t)
STUB1(rom_spi_send, int)
STUB1(rom_display_update, void *)
STUB1(rom_ogg_picture_init, const char *)

STUB_RET3(rom_file_open, int, uint32_t, uint32_t, int)
STUB1(rom_file_close, int)
STUB_RET2(rom_file_load_block, int, int, void *)
STUB_RET2(rom_file_read_sync, int, int, uint32_t)
STUB3(rom_file_seek, int, int, int)
STUB_RET3(rom_file_read, int, void *, int, int)
STUB_RET1(rom_file_size, int, int)
STUB_RET1(rom_file_tell, int, int)

STUB_RET2(rom_usb_stream_open, int, void *, int)
STUB_RET(rom_usb_query, int)
STUB1(rom_usb_set_mode, uint8_t)
STUB2(rom_usb_status, int, int)

STUB_RET(rom_get_input_event, int)
__attribute__((weak)) int rom_gui_check_area(int x1, int y1, int x2, int y2, int unused)
{
    (void)x1; (void)y1; (void)x2; (void)y2; (void)unused;
    return 0;
}
STUB_RET1(rom_event_pending, int, int)
STUB1(rom_event_clear_id, int)
STUB1(rom_post_event, int)
STUB2(rom_ui_cmd, int, void *)
STUB3(rom_draw_glyph, void *, void *, void *)
STUB1(rom_lcd_refresh, void *)
STUB_RET1(rom_event_clear_bit, int, int)
STUB2(rom_status_led, int, int)
__attribute__((weak)) void debug_printf(const char *fmt, ...) { (void)fmt; }

STUB_RET2(rom_storage_cmp, int, void *, void *)
STUB2(rom_storage_copy, void *, void *)
STUB0(rom_key_remap_apply)
STUB_RET1(rom_set_key_mode, char, uint8_t)
STUB3(rom_draw_dialog, void *, void *, void *)
STUB0(rom_key_remap_done)

STUB_RET1(rom_audio_save, uint32_t, int)
STUB_RET1(rom_dsp_save, uint32_t, int)
STUB_RET1(rom_bt_event_check, int, int)
STUB1(rom_bt_processing_start, int)
STUB1(rom_bt_display_lock, int)
__attribute__((weak)) void rom_bt_display_default(int a, int b, int c, int d, int e, int f)
{
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f;
}
STUB_RET1(rom_bt_metadata_ready, int, int)
STUB3(rom_bt_copy_track_name, void *, int, void *)
STUB3(rom_bt_display_set, int, int, int)
STUB_RET1(rom_bt_string_len, uint32_t, const char *)
STUB1(rom_bt_display_error, int)
STUB4(rom_bt_display_complete, int *, int, void *, int)
STUB1(rom_bt_display_clear, int)
STUB4(rom_bt_display_next, int *, int *, const char *, int)
STUB1(rom_bt_display_commit, void *)
STUB1(rom_bt_display_unlock, int)

STUB_RET2(rom_record_file_exists, int, void *, uint32_t)
STUB4(rom_record_start, void *, void *, void *, void *)

STUB_RET2(rom_bitstream_read, uint32_t, uint32_t, int)
STUB_RET2(rom_bitstream_read_signed, int32_t, uint32_t, int)
STUB_RET2(rom_peek_byte, uint32_t, uint32_t, int)
STUB_RET1(rom_clz, int, uint32_t)

__attribute__((weak)) void rom_mad_init(void *a, void *b, void *c, void *d,
                                        void *e, void *f, void *g, void *h)
{
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h;
}
STUB0(rom_mad_header_init)
STUB1(rom_mad_layer3_init, void *)
STUB1(rom_mad_huffman_init, void *)
STUB2(rom_mad_start, void *, void *)
STUB_RET2(rom_mad_decode_frame, int, void *, void *)
STUB_RET4(rom_mad_bitstream_fill, int, void *, void *, void *, void *)
STUB_RET4(rom_mad_header_sync, int, void *, void *, void *, void *)
STUB0(rom_mad_cleanup)
STUB_RET2(rom_mad_bitrate_calc, uint32_t, uint32_t, uint32_t)
