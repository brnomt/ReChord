#include "usb_dac_sink.h"

#include <stddef.h>
#include <string.h>

enum {
    FRAMES_PER_TARGET = 50,   /* 20 ms of cushion                        */
    FRAMES_PER_GUARD = 250,   /* 4 ms dead band                          */
    CHANNELS = 2,
    WORDS_PER_FRAME = 2
};

static uint32_t sample_word(const uint8_t *bytes, uint8_t count)
{
    switch (count)
    {
    case 2:
        /* 16-bit source -> left-justified 32-bit I2S slot. */
        return ((uint32_t)bytes[0] | (uint32_t)bytes[1] << 8) << 16;
    case 3:
        return (uint32_t)bytes[0] << 8 | (uint32_t)bytes[1] << 16 |
               (uint32_t)bytes[2] << 24;
    default:
        return (uint32_t)bytes[0] | (uint32_t)bytes[1] << 8 |
               (uint32_t)bytes[2] << 16 | (uint32_t)bytes[3] << 24;
    }
}

void usb_dac_sink_init(usb_dac_sink_t *sink, ringbuf_t *ring)
{
    memset(sink, 0, sizeof *sink);
    sink->ring = ring;
}

void usb_dac_sink_start(usb_dac_sink_t *sink, uint32_t rate, uint8_t sample_bytes)
{
    sink->rate = rate;
    sink->sample_bytes = sample_bytes >= 2 && sample_bytes <= 4 ? sample_bytes : 0;
    sink->target = rate / FRAMES_PER_TARGET;
    sink->guard = rate / FRAMES_PER_GUARD;
    sink->started = sample_bytes >= 2 && sample_bytes <= 4;
    sink->packets = sink->frames = sink->dropped = sink->repeated = 0;
}

void usb_dac_sink_prefill(usb_dac_sink_t *sink)
{
    uint32_t silence[WORDS_PER_FRAME * FRAMES_PER_TARGET];

    if (sink == NULL || sink->ring == NULL)
        return;
    memset(silence, 0, sizeof silence);
    for (uint32_t left = sink->target; left != 0; )
    {
        uint32_t frames = left < FRAMES_PER_TARGET ? left : FRAMES_PER_TARGET;
        uint32_t bytes = frames * WORDS_PER_FRAME * (uint32_t)sizeof(uint32_t);

        (void)rb_write(sink->ring, silence, bytes);
        left -= frames;
    }
}

uint32_t usb_dac_sink_fill(const usb_dac_sink_t *sink)
{
    if (sink == NULL || sink->ring == NULL)
        return 0;
    return (uint32_t)(rb_used(sink->ring) /
                      (WORDS_PER_FRAME * sizeof(uint32_t)));
}

uint32_t usb_dac_sink_write(usb_dac_sink_t *sink, const uint8_t *bytes,
                            uint32_t length)
{
    uint32_t frame_bytes;
    uint32_t frames;
    /* +1 frame: the drift-repeat path appends one past the packet, and a
     * MAX-sized packet (USB_DAC_MAX_FRAMES) is accepted. The old array had
     * exactly MAX frames, so that repeat wrote two words past the end. */
    uint32_t words[WORDS_PER_FRAME * (USB_DAC_MAX_FRAMES + 1u)];
    uint32_t fill, free_frames;

    if (sink == NULL || sink->ring == NULL || bytes == NULL || sink->sample_bytes == 0)
        return 0;

    frame_bytes = CHANNELS * sink->sample_bytes;
    frames = length / frame_bytes;
    if (frames == 0)
        return 0;
    if (frames > USB_DAC_MAX_FRAMES)
        frames = USB_DAC_MAX_FRAMES;

    for (uint32_t index = 0; index < frames; index++)
    {
        const uint8_t *frame = bytes + index * frame_bytes;

        words[WORDS_PER_FRAME * index] = sample_word(frame, sink->sample_bytes);
        words[WORDS_PER_FRAME * index + 1] =
            sample_word(frame + sink->sample_bytes, sink->sample_bytes);
    }

    fill = usb_dac_sink_fill(sink);
    if (fill > sink->target + sink->guard && frames > 1)
    {
        frames--;                       /* host ahead: drop one frame   */
        sink->dropped++;
    }
    else if (fill + sink->guard < sink->target)
    {
        /* Host behind: repeat the last frame so the DAC never starves. */
        words[WORDS_PER_FRAME * frames] = words[WORDS_PER_FRAME * (frames - 1)];
        words[WORDS_PER_FRAME * frames + 1] =
            words[WORDS_PER_FRAME * (frames - 1) + 1];
        frames++;
        sink->repeated++;
    }

    free_frames = (uint32_t)(rb_free(sink->ring) /
                             (WORDS_PER_FRAME * sizeof(uint32_t)));
    if (frames > free_frames)
        frames = free_frames;
    if (frames == 0)
    {
        sink->packets++;
        return 0;
    }

    (void)rb_write(sink->ring, words,
                   frames * WORDS_PER_FRAME * (uint32_t)sizeof(uint32_t));
    sink->packets++;
    sink->frames += frames;
    return frames;
}
