/*
 * ipc_shared.h — shared A<->B audio control block + PCM ring (BRNOMT-68).
 *
 * Both cores use this layout. The PCM ring lives inside HIGHRAM0, above the
 * tiny Core-B image (linked at 0x01000000, < 64 KiB); the control block is
 * the documented shared region at 0x01020000. Single producer (AP) / single
 * consumer (B). The Cortex-M3 has no data cache, and each side publishes its
 * index with a DMB, so no cache maintenance is needed.
 *
 * Ring: 64 KiB of interleaved int32 stereo frames (8 bytes), left-justified
 * (the internal pipeline format) — exactly one 8192-sample FLAC block at
 * 96 kHz (85.3 ms), so the AP's atomic per-block decode has the whole block
 * of slack. The AP writes [wr] then DMB then updates IPC_SH_WR; B reads WR,
 * copies, then updates IPC_SH_RD.
 */
#ifndef RECHORD_IPC_SHARED_H
#define RECHORD_IPC_SHARED_H

#include <stdint.h>

#define IPC_SHARED_BASE   0x01020000u
#define IPC_RING_BASE     0x01010000u
#define IPC_RING_BYTES    (64u * 1024u)
#define IPC_RING_MASK     (IPC_RING_BYTES - 1u)

_Static_assert(IPC_RING_BASE >= 0x01000000u, "ring must stay in HIGHRAM0");
_Static_assert(IPC_RING_BASE + IPC_RING_BYTES <= IPC_SHARED_BASE,
               "ring must not overlap the control block");
_Static_assert((IPC_RING_BYTES & IPC_RING_MASK) == 0u, "ring must be pow2");

/* Control word indexes (all uint32_t). */
#define IPC_SH_MAGIC      0u   /* B writes 'BRDY' once                     */
#define IPC_SH_STATE      1u   /* AP writes: IPC_SH_IDLE/RUN/PAUSE         */
#define IPC_SH_RD         2u   /* B writes (byte rd index, free-running)   */
#define IPC_SH_WR         3u   /* AP writes (byte wr index, free-running)  */
#define IPC_SH_FRAMES     4u   /* B increments (frames played)             */
#define IPC_SH_UNDERRUNS  5u   /* B increments (ring dry)                  */
#define IPC_SH_RATE       6u   /* AP writes (Hz, informational)            */
#define IPC_SH_CHANNELS   7u   /* AP writes (1/2)                          */
#define IPC_SH_BITS       8u   /* AP writes (16/24/32)                     */
#define IPC_SH_COUNT      9u

#define IPC_SH_MAGIC_VAL  0x42524459u   /* 'BRDY' */

#define IPC_SH_IDLE       0u
#define IPC_SH_RUN        1u
#define IPC_SH_PAUSE      2u

/* ---- file service (phase 3): A owns the FS and streams a file to B ---- */
/* Layout of the B data region (0x01020000..0x01060000, per the vendor
 * B_Core.sct): control block 0x01020000..0x01021000, file ring
 * 0x01021000..0x01029000, B .data/.bss/stack above 0x01031000. */
#define IPC_FILE_BASE     0x01021000u
#define IPC_FILE_BYTES    (32u * 1024u)
#define IPC_FILE_MASK     (IPC_FILE_BYTES - 1u)

/* File control words (continue after the PCM ones). All logical offsets are
 * relative to IPC_F_BASE; B's file position = IPC_F_BASE + IPC_F_RD. */
#define IPC_F_OPEN     9u    /* A: 1 while a file is open/served          */
#define IPC_F_SIZE     10u   /* A: file size (0xFFFFFFFF = unknown)       */
#define IPC_F_BASE     11u   /* A: logical offset of ring[0]              */
#define IPC_F_RD       12u   /* B: ring read cursor (relative)            */
#define IPC_F_WR       13u   /* A: ring write cursor (relative)           */
#define IPC_F_EOF      14u   /* A: 1 = rest of the file is in the ring    */
#define IPC_F_SEEK_TO  15u   /* B: requested absolute file offset         */
#define IPC_F_SEEK_SEQ 16u   /* B: increment to request a seek            */
#define IPC_F_SEEK_ACK 17u   /* A: copies SEQ once the seek is applied    */
#define IPC_F_COUNT    18u

_Static_assert(IPC_FILE_BASE >= IPC_SHARED_BASE + 0x1000u,
               "file ring must sit above the 4 KiB control block");
_Static_assert(IPC_FILE_BASE + IPC_FILE_BYTES <= 0x01031000u,
               "file ring must stay below B's .data/.bss region");

#endif /* RECHORD_IPC_SHARED_H */
