/*
 * ipc_shared.h — shared A<->B audio control block + PCM ring (BRNOMT-68).
 *
 * Both cores use this layout. Memory follows the vendor B-core scatter file
 * (community/sdks/.../Scatter/B_Core.sct): Core B runs code from HIGHRAM0 and
 * keeps its data in 0x01020000..0x01060000 (up to HIFIACC). So:
 *
 *   0x01000000..0x01020000   B code (HIGHRAM0, 128 KiB)
 *   0x01020000..0x01021000   control block (shared)
 *   0x01021000..0x01029000   A->B file ring (shared)
 *   0x01030000..0x01040000   PCM ring (phase 3c)
 *   0x01040000..0x01060000   B .data/.bss (decode pool) + stack
 *
 * Phase 3c: Core B decodes and is the PCM-ring PRODUCER (writes IPC_SH_WR);
 * the AP keeps the DAC/I2S bring-up and drains the ring in its TX ISR
 * (consumer, writes IPC_SH_RD and IPC_SH_FRAMES). Fallback: if B does not come
 * up, the AP decodes and produces the ring itself. Single producer / single
 * consumer; Cortex-M3 has no data cache and every publication is followed by a
 * DMB, so no cache maintenance is needed.
 */
#ifndef RECHORD_IPC_SHARED_H
#define RECHORD_IPC_SHARED_H

#include <stdint.h>

#define IPC_SHARED_BASE   0x01020000u
#define IPC_RING_BASE     0x01030000u
#define IPC_RING_BYTES    (64u * 1024u)
#define IPC_RING_MASK     (IPC_RING_BYTES - 1u)

_Static_assert(IPC_RING_BASE >= IPC_SHARED_BASE + 0x1000u,
               "PCM ring must sit above the control block");
_Static_assert(IPC_RING_BASE + IPC_RING_BYTES <= 0x01040000u,
               "PCM ring must stay below Core B's .data/.bss region");
_Static_assert((IPC_RING_BYTES & IPC_RING_MASK) == 0u, "ring must be pow2");

/* Control word indexes (all uint32_t). */
#define IPC_SH_MAGIC      0u   /* B writes 'BRDY' once                     */
#define IPC_SH_STATE      1u   /* AP writes: IPC_SH_IDLE/RUN/PAUSE         */
#define IPC_SH_RD         2u   /* consumer writes (byte rd index)          */
#define IPC_SH_WR         3u   /* producer writes (byte wr index)          */
#define IPC_SH_FRAMES     4u   /* consumer increments (frames played)      */
#define IPC_SH_UNDERRUNS  5u   /* consumer increments (ring dry)           */
#define IPC_SH_RATE       6u   /* AP writes (Hz, informational)            */
#define IPC_SH_CHANNELS   7u   /* AP writes (1/2)                          */
#define IPC_SH_BITS       8u   /* AP writes (16/24/32)                     */

/* ---- file service (phase 3c): A owns the FS and streams a file to B ---- */
/* All logical offsets are relative to IPC_F_BASE; B's file position =
 * IPC_F_BASE + IPC_F_RD. */
#define IPC_F_OPEN     9u    /* A: 1 while a file is open/served          */
#define IPC_F_SIZE     10u   /* A: file size (0xFFFFFFFF = unknown)       */
#define IPC_F_BASE     11u   /* A: logical offset of ring[0]              */
#define IPC_F_RD       12u   /* B: ring read cursor (relative)            */
#define IPC_F_WR       13u   /* A: ring write cursor (relative)           */
#define IPC_F_EOF      14u   /* A: 1 = rest of the file is in the ring    */
#define IPC_F_SEEK_TO  15u   /* B: requested absolute file offset         */
#define IPC_F_SEEK_SEQ 16u   /* B: increment to request a seek            */
#define IPC_F_SEEK_ACK 17u   /* A: copies SEQ once the seek is applied    */

/* ---- command channel (phase 3c): AP -> B decode control ---- */
#define IPC_C_CMD      18u   /* AP writes IPC_C_* command                 */
#define IPC_C_ARG      19u   /* AP: command argument (ext code / seek ms) */
#define IPC_C_ACK      20u   /* B: last completed command                 */
#define IPC_C_BSTATE   21u   /* B: 0 idle, 1 open, 2 eof, 3 error         */
#define IPC_C_BEOF     22u   /* B: 1 = decoder drained                    */
#define IPC_C_BERR     23u   /* B: negative codec error                   */
#define IPC_C_RG       24u   /* AP: ReplayGain Q16 (32768 = neutral 1.0x) */
#define IPC_C_GAP_DELAY 25u  /* AP: leading trim samples (encoder delay)  */
#define IPC_C_GAP_PAD  26u   /* AP: trailing trim samples (encoder pad)   */
#define IPC_C_GAP_TOTAL 27u  /* AP: exact total samples (0 = unknown)     */

#define IPC_SH_COUNT   28u

#define IPC_SH_MAGIC_VAL  0x42524459u   /* 'BRDY' */

#define IPC_SH_IDLE       0u
#define IPC_SH_RUN        1u
#define IPC_SH_PAUSE      2u

/* B commands (IPC_C_CMD). */
#define IPC_C_NONE        0u
#define IPC_C_OPEN        1u   /* ARG = codec id (0 wav, 1 mp3, 2 flac)   */
#define IPC_C_SEEK        2u   /* ARG = milliseconds                      */
#define IPC_C_CLOSE       3u

/* B states (IPC_C_BSTATE). */
#define IPC_BS_IDLE       0u
#define IPC_BS_OPEN       1u
#define IPC_BS_EOF        2u
#define IPC_BS_ERROR      3u

/* ---- file ring ---- */
#define IPC_FILE_BASE     0x01021000u
#define IPC_FILE_BYTES    (32u * 1024u)
#define IPC_FILE_MASK     (IPC_FILE_BYTES - 1u)

_Static_assert(IPC_FILE_BASE >= IPC_SHARED_BASE + 0x1000u,
               "file ring must sit above the 4 KiB control block");
_Static_assert(IPC_FILE_BASE + IPC_FILE_BYTES <= IPC_RING_BASE,
               "file ring must stay below the PCM ring");

#endif /* RECHORD_IPC_SHARED_H */
