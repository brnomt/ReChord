/*
 * ipc_b_mock.h — HOST-ONLY test hooks for the simulated B core.
 *
 * ipc_b_mock.c implements the real ipc_b.h contract; this header adds the
 * knobs the host tests need (consumption rate, sink capture, fault
 * injection). Never included by target code.
 */
#ifndef RECHORD_IPC_B_MOCK_H
#define RECHORD_IPC_B_MOCK_H

#include <stdint.h>

/* Frames the simulated B core consumes per ipc_b_poll_state() call
 * (default 256). Set to 0 to freeze consumption. */
void ipc_b_mock_set_frames_per_tick(int frames);

/* Called with every chunk of PCM the simulated B core consumes — the
 * "mock audio sink", so tests can verify streamed content end-to-end.
 * Samples are int32, left-justified to 32 bits (the pipeline format). */
typedef void (*ipc_b_mock_sink_fn)(const int32_t *pcm, int frames,
                                   void *user);
void ipc_b_mock_set_sink(ipc_b_mock_sink_fn fn, void *user);

/* Force the supervisor into IPC_B_FAULT ("B is gone") on the next poll. */
void ipc_b_mock_set_fault(int on);

/* Phase 3c knobs: pretend Core B is the PCM producer (the player then skips
 * its own decode and takes the B-side gapless hand-off path). `b_eof` is the
 * scripted "B's decoder drained" signal the player reacts to. */
void ipc_b_mock_set_b_decodes(int on);
void ipc_b_mock_set_b_eof(int on);

/* Host-only: number of ipc_b_next() calls (gapless Core-B track changes). */
int  ipc_b_next_count(void);

/* Host-only: number of times ipc_b_start() was called (B-core restarts).
 * Tests use this to assert a gapless transition does NOT restart the core. */
int ipc_b_start_count(void);

#endif /* RECHORD_IPC_B_MOCK_H */
