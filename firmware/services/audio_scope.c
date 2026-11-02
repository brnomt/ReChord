/* audio_scope — see audio_scope.h. Pure integer maths, host-testable. */
#include "audio_scope.h"

#define LEVEL_GAIN     3      /* mean|x| of music is ~0.1-0.3 FS: scale up   */
#define DECAY_MS       160    /* level falls ~63 % per this time             */
#define PEAK_FALL_MS   1000   /* peak hold falls from full scale in this     */

static int clamp1000(int v)
{
    return v > 1000 ? 1000 : (v < -1000 ? -1000 : v);
}

static int to_k(int32_t x)    /* int32 left-justified -> -1000..1000 */
{
    return (int)(((long long)x * 1000) >> 31);
}

static int decay(int cur, int target, int dt_ms)
{
    if (target >= cur)
        return target;                         /* instant attack */
    if (dt_ms >= DECAY_MS)
        return target;
    {
        int d = (cur - target) * dt_ms / DECAY_MS;

        return cur - (d < 1 ? 1 : d);          /* never stall above target */
    }
}

static int peak_step(int peak, int now, int dt_ms)
{
    int fall = 1000 * dt_ms / PEAK_FALL_MS;

    if (now >= peak)
        return now;
    peak -= fall;
    return peak < now ? now : peak;
}

void audio_scope_reset(audio_scope_t *s)
{
    int i;

    s->lvl_l = s->lvl_r = s->peak_l = s->peak_r = 0;
    for (i = 0; i < AUDIO_SCOPE_N; i++)
        s->wave_l[i] = s->wave_r[i] = 0;
}

static int32_t left_at(const volatile int32_t *ring, uint32_t rd,
                       uint32_t mask, int i)
{
    return ring[((rd + (uint32_t)i * 8u) & mask) >> 2];
}

/* First rising zero crossing of L with a real slope (not noise), else 0. */
static int find_trigger(const volatile int32_t *ring, uint32_t rd,
                        uint32_t mask, int search)
{
    int i;

    for (i = 1; i < search; i++) {
        int32_t a = left_at(ring, rd, mask, i - 1);
        int32_t b = left_at(ring, rd, mask, i);

        if (a < 0 && b >= 0 &&
            left_at(ring, rd, mask, i + 4) > b + (1 << 20))
            return i;
    }
    return 0;
}

void audio_scope_feed(audio_scope_t *s, const volatile int32_t *ring,
                      uint32_t rd, uint32_t ring_mask, int avail, int dt_ms)
{
    long long sum_l = 0, sum_r = 0, acc_l = 0, acc_r = 0;
    int per, i, b = 0, in_b = 0, frames = avail;

    if (avail > AUDIO_SCOPE_FRAMES + 8) {
        int room = avail - AUDIO_SCOPE_FRAMES - 8;

        if (room > AUDIO_SCOPE_TRIG)
            room = AUDIO_SCOPE_TRIG;
        rd += (uint32_t)find_trigger(ring, rd, ring_mask, room) * 8u;
    }
    if (frames > AUDIO_SCOPE_FRAMES)
        frames = AUDIO_SCOPE_FRAMES;
    if (frames < AUDIO_SCOPE_N) {
        audio_scope_idle(s, dt_ms);
        return;
    }
    per = frames / AUDIO_SCOPE_N;
    for (i = 0; i < per * AUDIO_SCOPE_N; i++) {
        const volatile int32_t *f =
            ring + (((rd + (uint32_t)i * 8u) & ring_mask) >> 2);
        int32_t l = f[0], r = f[1];

        sum_l += l < 0 ? -(long long)l : l;
        sum_r += r < 0 ? -(long long)r : r;
        acc_l += l;
        acc_r += r;
        if (++in_b == per) {
            s->wave_l[b] = (int16_t)clamp1000(to_k((int32_t)(acc_l / per)));
            s->wave_r[b] = (int16_t)clamp1000(to_k((int32_t)(acc_r / per)));
            b++;
            in_b = 0;
            acc_l = acc_r = 0;
        }
    }
    {
        int n = per * AUDIO_SCOPE_N;
        int ll = clamp1000(to_k((int32_t)(sum_l / n)) * LEVEL_GAIN);
        int lr = clamp1000(to_k((int32_t)(sum_r / n)) * LEVEL_GAIN);

        s->lvl_l = decay(s->lvl_l, ll, dt_ms);
        s->lvl_r = decay(s->lvl_r, lr, dt_ms);
        s->peak_l = peak_step(s->peak_l, ll, dt_ms);
        s->peak_r = peak_step(s->peak_r, lr, dt_ms);
    }
}

void audio_scope_idle(audio_scope_t *s, int dt_ms)
{
    int i;

    s->lvl_l = decay(s->lvl_l, 0, dt_ms);
    s->lvl_r = decay(s->lvl_r, 0, dt_ms);
    s->peak_l = peak_step(s->peak_l, 0, dt_ms);
    s->peak_r = peak_step(s->peak_r, 0, dt_ms);
    for (i = 0; i < AUDIO_SCOPE_N; i++) {
        s->wave_l[i] = (int16_t)(s->wave_l[i] * 3 / 4);
        s->wave_r[i] = (int16_t)(s->wave_r[i] * 3 / 4);
    }
}
