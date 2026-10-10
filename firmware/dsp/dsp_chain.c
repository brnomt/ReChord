/*
 * dsp_chain.c — DSP effect chain: ordered modules, per-module bypass.
 *
 * Deliberately allocation-free: the chain struct is a plain value and each
 * module's state lives in a caller-owned buffer, so the whole audio path has
 * static, auditable memory use (important on the Cortex-M3, and it makes the
 * host tests exercise exactly the target's memory model).
 */
#include <string.h>

#include "dsp_chain.h"

/* ASCII case-insensitive compare ("EQ" and "eq" are the same module). */
static int name_eq(const char *a, const char *b)
{
    if (a == 0 || b == 0)
        return 0;
    while (*a && *b) {
        int ca = (unsigned char)*a++;
        int cb = (unsigned char)*b++;
        if (ca >= 'a' && ca <= 'z') ca -= 'a' - 'A';
        if (cb >= 'a' && cb <= 'z') cb -= 'a' - 'A';
        if (ca != cb)
            return 0;
    }
    return *a == '\0' && *b == '\0';
}

void dsp_chain_init(dsp_chain_t *c, int sample_rate)
{
    if (c == 0)
        return;
    memset(c, 0, sizeof *c);
    c->sample_rate = (sample_rate > 0) ? sample_rate : 48000;
}

int dsp_chain_add(dsp_chain_t *c, const dsp_module_t *mod,
                  void *state, int state_bytes)
{
    int idx;

    if (c == 0 || mod == 0 || state == 0)
        return -1;
    if (mod->init == 0 || mod->process == 0)
        return -1;
    if (c->count >= DSP_CHAIN_MAX_MODULES)
        return -1;
    if (state_bytes < mod->state_size)
        return -2;
    /* Doubles in module states need 8-byte alignment; catching it here beats
     * a hard fault (or slow unaligned traps) deep inside the audio path. */
    if (((uintptr_t)state & 7u) != 0)
        return -3;

    idx = c->count;
    if (mod->init(state, c->sample_rate) != 0)
        return -4;

    c->slot[idx].mod = mod;
    c->slot[idx].state = state;
    c->slot[idx].bypass = 0;
    c->count = idx + 1;
    return idx;
}

int dsp_chain_process(dsp_chain_t *c, int16_t *samples, int n)
{
    int i;

    if (c == 0 || samples == 0 || n <= 0)
        return -1;

    for (i = 0; i < c->count; i++) {
        const dsp_chain_slot_t *s = &c->slot[i];
        if (s->bypass)
            continue;   /* bypass = byte-exact passthrough, no processing */
        if (s->mod->process(s->state, samples, n) != 0)
            return -2;
    }
    return 0;
}

int dsp_chain_find(const dsp_chain_t *c, const char *module_name)
{
    int i;

    if (c == 0 || module_name == 0)
        return -1;
    for (i = 0; i < c->count; i++)
        if (name_eq(c->slot[i].mod->name, module_name))
            return i;
    return -1;
}

int dsp_chain_set_bypass(dsp_chain_t *c, const char *module_name, int bypass)
{
    int idx = dsp_chain_find(c, module_name);
    if (idx < 0)
        return -1;
    return dsp_chain_set_bypass_at(c, idx, bypass);
}

int dsp_chain_set_bypass_at(dsp_chain_t *c, int index, int bypass)
{
    if (c == 0 || index < 0 || index >= c->count)
        return -1;
    c->slot[index].bypass = (bypass != 0);
    return 0;
}

int dsp_chain_get_bypass_at(const dsp_chain_t *c, int index)
{
    if (c == 0 || index < 0 || index >= c->count)
        return -1;
    return c->slot[index].bypass;
}

int dsp_chain_set_param(dsp_chain_t *c, const char *module_name,
                        int param, int value)
{
    int idx = dsp_chain_find(c, module_name);
    if (idx < 0)
        return -1;
    return dsp_chain_set_param_at(c, idx, param, value);
}

int dsp_chain_set_param_at(dsp_chain_t *c, int index, int param, int value)
{
    if (c == 0 || index < 0 || index >= c->count)
        return -1;
    return c->slot[index].mod->set_param(c->slot[index].state, param, value);
}

int dsp_chain_get_param(dsp_chain_t *c, const char *module_name, int param)
{
    int idx = dsp_chain_find(c, module_name);
    if (idx < 0)
        return INT_MIN;
    return dsp_chain_get_param_at(c, idx, param);
}

int dsp_chain_get_param_at(dsp_chain_t *c, int index, int param)
{
    if (c == 0 || index < 0 || index >= c->count)
        return INT_MIN;
    return c->slot[index].mod->get_param(c->slot[index].state, param);
}

int dsp_chain_count(const dsp_chain_t *c)
{
    return c ? c->count : 0;
}

const char *dsp_chain_name_at(const dsp_chain_t *c, int index)
{
    if (c == 0 || index < 0 || index >= c->count)
        return 0;
    return c->slot[index].mod->name;
}
