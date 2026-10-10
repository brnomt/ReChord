/*
 * dsp_chain.h — ordered chain of DSP effect modules with per-module bypass.
 *
 * The chain is the user-configurable "effects stack": modules are appended
 * in user order, each with its own state buffer (caller-owned, no heap) and
 * a bypass flag. dsp_chain_process() runs the whole stack over one block.
 *
 * Lookup is by module NAME for config files ("set-by-name") and by INDEX for
 * UI code; when the same module type appears twice, name lookup returns the
 * first instance — use the index API for full control.
 */
#ifndef DSP_CHAIN_H
#define DSP_CHAIN_H

#include "dsp_module.h"

#define DSP_CHAIN_MAX_MODULES 8

typedef struct
{
    const dsp_module_t *mod;
    void               *state;
    int                 bypass;
} dsp_chain_slot_t;

typedef struct
{
    dsp_chain_slot_t slot[DSP_CHAIN_MAX_MODULES];
    int              count;
    int              sample_rate;
} dsp_chain_t;

/* Prepare an empty chain. Call before adding modules. */
void dsp_chain_init(dsp_chain_t *c, int sample_rate);

/*
 * Append a module instance.
 *   state        caller-owned buffer for the module's state (see
 *                DSP_STATE_STORAGE); must hold at least mod->state_size
 *                bytes and be 8-byte aligned (contains doubles);
 *   state_bytes  size of that buffer, for the bounds check.
 * The module's init() runs immediately with the chain's sample rate.
 * Returns the slot index (>= 0) or < 0 on error:
 *   -1 NULL/full, -2 state too small, -3 state misaligned, -4 init failed.
 */
int dsp_chain_add(dsp_chain_t *c, const dsp_module_t *mod,
                  void *state, int state_bytes);

/*
 * Run every non-bypassed module over the block, in chain order.
 * Returns 0, or < 0 if a module rejected the block.
 */
int dsp_chain_process(dsp_chain_t *c, int16_t *samples, int n);

/* Bypass control: 1 = skip the module (audio passes through untouched). */
int dsp_chain_set_bypass(dsp_chain_t *c, const char *module_name, int bypass);
int dsp_chain_set_bypass_at(dsp_chain_t *c, int index, int bypass);
int dsp_chain_get_bypass_at(const dsp_chain_t *c, int index);

/* Parameter access: by name (config files) and by index (UI). */
int dsp_chain_set_param(dsp_chain_t *c, const char *module_name,
                        int param, int value);
int dsp_chain_set_param_at(dsp_chain_t *c, int index, int param, int value);
int dsp_chain_get_param(dsp_chain_t *c, const char *module_name, int param);
int dsp_chain_get_param_at(dsp_chain_t *c, int index, int param);

/* Introspection. */
int          dsp_chain_count(const dsp_chain_t *c);
const char  *dsp_chain_name_at(const dsp_chain_t *c, int index);
int          dsp_chain_find(const dsp_chain_t *c, const char *module_name);

#endif /* DSP_CHAIN_H */
