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
 * "mock audio sink", so tests can verify streamed content end-to-end. */
typedef void (*ipc_b_mock_sink_fn)(const int16_t *pcm, int frames,
                                   void *user);
void ipc_b_mock_set_sink(ipc_b_mock_sink_fn fn, void *user);

/* Force the supervisor into IPC_B_FAULT ("B is gone") on the next poll. */
void ipc_b_mock_set_fault(int on);

#endif /* RECHORD_IPC_B_MOCK_H */
