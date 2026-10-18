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

/* ---- second file service (pre-open ring, BRNOMT-73) ---- */
/* Same fields as IPC_F_*, for the ring the AP PRE-READS into while the current
 * track still plays. At the gapless boundary the AP promotes this ring and
 * tells B to read it (IPC_C_RING), so B's codec reopen finds the next file
 * already in RAM instead of waiting on the SD. */
#define IPC_F2_OPEN    29u
#define IPC_F2_SIZE    30u
#define IPC_F2_BASE    31u
#define IPC_F2_RD      32u
#define IPC_F2_WR      33u
#define IPC_F2_EOF     34u
#define IPC_F2_SEEK_TO 35u
#define IPC_F2_SEEK_SEQ 36u
#define IPC_F2_SEEK_ACK 37u

/* ---- command channel (phase 3c): AP -> B decode control ---- */
/* Sequencing: the AP increments IPC_C_SEQ on every command and B copies it
 * into IPC_C_ACK once done. Comparing SEQUENCE numbers (not the command word)
 * lets B see a repeated command — two SEEKs to the same ms, two gapless NEXTs
 * — which an "cmd == ack" test would silently drop. */
#define IPC_C_CMD      18u   /* AP writes IPC_C_* command                 */
#define IPC_C_ARG      19u   /* AP: command argument (ext code / seek ms) */
#define IPC_C_ACK      20u   /* B: sequence number of the last done cmd   */
#define IPC_C_BSTATE   21u   /* B: 0 idle, 1 open, 2 eof, 3 error         */
#define IPC_C_BEOF     22u   /* B: 1 = decoder drained                    */
#define IPC_C_BERR     23u   /* B: negative codec error                   */
#define IPC_C_RG       24u   /* AP: ReplayGain Q16 (32768 = neutral 1.0x) */
#define IPC_C_GAP_DELAY 25u  /* AP: leading trim samples (encoder delay)  */
#define IPC_C_GAP_PAD  26u   /* AP: trailing trim samples (encoder pad)   */
#define IPC_C_GAP_TOTAL 27u  /* AP: exact total samples (0 = unknown)     */
#define IPC_C_SEQ      28u   /* AP: command sequence (increments per cmd) */
#define IPC_C_RING     38u   /* AP: file ring B must read: 0 = IPC_F_*,
                              * 1 = IPC_F2_* (set before OPEN/NEXT)       */

/* Stream geometry the AP discovered while prescanning the file (BRNOMT-73):
 * published with each OPEN/NEXT so Core B can size its decode pool without
 * repeating the header walk (see codec_t.prescan_info / flac.c). Valid only
 * while IPC_C_PRE_VALID == 1. */
#define IPC_C_PRE_VALID    39u   /* AP: 1 = geometry below is for this open */
#define IPC_C_PRE_MAXBLOCK 40u   /* AP: FLAC max block size (samples)       */
#define IPC_C_PRE_CHANNELS 41u   /* AP: stream channels (1..2)              */
#define IPC_C_PRE_SEEKTABLE 42u  /* AP: SEEKTABLE bytes in the stream       */

#define IPC_SH_COUNT   43u

#define IPC_SH_MAGIC_VAL  0x42524459u   /* 'BRDY' */

#define IPC_SH_IDLE       0u
#define IPC_SH_RUN        1u
#define IPC_SH_PAUSE      2u

/* B commands (IPC_C_CMD). */
#define IPC_C_NONE        0u
#define IPC_C_OPEN        1u   /* ARG = codec id (0 wav, 1 mp3, 2 flac)   */
#define IPC_C_SEEK        2u   /* ARG = milliseconds                      */
#define IPC_C_CLOSE       3u
#define IPC_C_NEXT        4u   /* gapless track change (BRNOMT-73): same  */
                               /* ARG/GAP/RG as OPEN, but on the LIVE ring */

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

/* ---- pre-open ring (BRNOMT-73) ---- */
/* Fits the free gap between the file ring and the PCM ring
 * (0x01029000..0x01030000), so the memory map below Core B is unchanged. */
#define IPC_FILE2_BASE     0x01029000u
#define IPC_FILE2_BYTES    (16u * 1024u)
#define IPC_FILE2_MASK     (IPC_FILE2_BYTES - 1u)

_Static_assert(IPC_FILE2_BASE >= IPC_FILE_BASE + IPC_FILE_BYTES,
               "pre-open ring must follow the current file ring");
_Static_assert(IPC_FILE2_BASE + IPC_FILE2_BYTES <= IPC_RING_BASE,
               "pre-open ring must stay below the PCM ring");
_Static_assert((IPC_FILE2_BYTES & IPC_FILE2_MASK) == 0u,
               "pre-open ring must be pow2");

#endif /* RECHORD_IPC_SHARED_H */
