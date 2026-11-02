/* Host test for audio_scope: sine in a wrapped ring, silence, decay. */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "audio_scope.h"

#define RING_BYTES 65536u
static int32_t ring[RING_BYTES / 4];
static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

static void fill_sine(uint32_t rd, int frames, double amp, int right_inv)
{
    int i;
    for (i = 0; i < frames; i++) {
        double v = sin(2 * 3.14159265358979 * i / 256.0) * amp * 2147483647.0;
        uint32_t o = ((rd + (uint32_t)i * 8u) & (RING_BYTES - 1u)) >> 2;
        ring[o] = (int32_t)v;
        ring[o + 1] = (int32_t)(right_inv ? -v : v);
    }
}

int main(void)
{
    audio_scope_t s;
    int i, mn = 0, mx = 0;

    audio_scope_reset(&s);
    /* window starts near the end so the read wraps around the ring */
    fill_sine(RING_BYTES - 1024u * 4u, AUDIO_SCOPE_FRAMES, 0.5, 1);
    audio_scope_feed(&s, ring, RING_BYTES - 1024u * 4u, RING_BYTES - 1u,
                     AUDIO_SCOPE_FRAMES, 50);
    for (i = 0; i < AUDIO_SCOPE_N; i++) {
        if (s.wave_l[i] < mn) mn = s.wave_l[i];
        if (s.wave_l[i] > mx) mx = s.wave_l[i];
        CHECK(s.wave_r[i] == -s.wave_l[i] || s.wave_r[i] == -s.wave_l[i] + 1 ||
              s.wave_r[i] == -s.wave_l[i] - 1);
    }
    CHECK(mx > 400 && mx <= 500);          /* 0.5 FS sine, bucket-averaged */
    CHECK(mn < -400 && mn >= -500);
    CHECK(s.lvl_l > 300 && s.lvl_l <= 1000);
    CHECK(s.peak_l >= s.lvl_l);

    /* silence: levels fall, never jump up; peak holds longer than level */
    memset(ring, 0, sizeof ring);
    audio_scope_feed(&s, ring, 0, RING_BYTES - 1u, AUDIO_SCOPE_FRAMES, 50);
    CHECK(s.lvl_l > 0 && s.lvl_l < 1000);
    CHECK(s.peak_l >= s.lvl_l);
    for (i = 0; i < 40; i++)
        audio_scope_idle(&s, 50);
    CHECK(s.lvl_l == 0 && s.peak_l == 0 && s.wave_l[3] == 0);

    /* too few frames behaves as idle */
    audio_scope_feed(&s, ring, 0, RING_BYTES - 1u, 3, 50);
    CHECK(s.lvl_l == 0);

    if (fails == 0) printf("test_audio_scope: OK\n");
    return fails != 0;
}
