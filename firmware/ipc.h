/*
 * ipc.h — canonical inter-core mailbox contract (ReChord).
 *
 * The Echo Mini runs two firmware halves (AP/UI on SYS_M3, BB/audio on
 * CAL_M3) that talk over the hardware mailbox at 0x40110000. This header is
 * the SINGLE source of truth for the wire protocol so the AP and BB builds
 * can never drift out of sync. It is deliberately freestanding: no SDK
 * typedefs, plain integer literals, so both bare-metal code (bb_stub.c) and
 * the SDK tree can include it.
 *
 * Command-value ownership:
 *   - MAILBOX channel/ID/INT assignment   -> here.
 *   - MSGBOX_CMD_SYSTEM_* (channel 0)      -> here (sequential, matches
 *                                             driver/BB/BBSystem.h enum).
 *   - MEDIA_MSGBOX_CMD_DECODE_* (channel 1) -> SDK enum
 *                                             audio/Include/audio_main.h.
 *   - MEDIA_MSGBOX_CMD_FILE_*   (channel 2) -> SDK enum filesys/file.h
 *                                             (also mirrored in include/fsinclude.h).
 *   - MEDIA_MSGBOX_CMD_ENCODE_* (channel 1) -> SDK enum
 *                                             audio/RecordControl/RecordControl.h.
 * The media command IDs are SDK ENUMS (not macros) and are intentionally NOT
 * redefined here — duplicating them as macros would collide with the enum
 * declarations. They are listed below as documentation only.
 */
#ifndef RECHORD_IPC_H
#define RECHORD_IPC_H

/* Hardware mailbox base (SDK hw_memap.h; verified against stock binary). */
#define IPC_MAILBOX_BASE      0x40110000u

/* Channel / ID assignment. */
#define MAILBOX_ID_0          0
#define MAILBOX_ID_1          1
#define MAILBOX_ID_2          2
#define MAILBOX_ID_3          3

#define MAILBOX_CHANNEL_0     0
#define MAILBOX_CHANNEL_1     1
#define MAILBOX_CHANNEL_2     2
#define MAILBOX_CHANNEL_3     3

/* Interrupt-enable BITMASKS (1<<n, NOT the channel number). */
#define MAILBOX_INT_0         (1u << 0)
#define MAILBOX_INT_1         (1u << 1)
#define MAILBOX_INT_2         (1u << 2)
#define MAILBOX_INT_3         (1u << 3)

/* Channel roles (RKnanoD convention). */
#define MB_CH_SYSTEM          0u   /* MAILBOX0 / IRQ 9  / vector slot 25 */
#define MB_CH_DECODE          1u   /* MAILBOX1 / IRQ 10 / vector slot 26 */
#define MB_CH_FILE            2u   /* MAILBOX2 / IRQ 11 / vector slot 27 */
#define MB_CH_DEBUG           3u   /* MAILBOX3 / IRQ 12 / vector slot 28 */

/*
 * Channel 0 — system control (driver/BB/BBSystem.h MSGBOX_SYSTEM_CMD,
 * sequential from 0):
 *   0 NULL, 1 SYSTEM_START_OK, 2 BB_HOLD, 3 BB_HOLD_ACK,
 *   4 BB_HOLD_EXIT, 5 PRINT_LOG, 6 PRINT_LOG_OK
 */
#define MSGBOX_CMD_SYSTEM_START_OK       1u
#define MSGBOX_CMD_BB_HOLD               2u
#define MSGBOX_CMD_BB_HOLD_ACK           3u
#define MSGBOX_CMD_BB_HOLD_EXIT          4u
#define MSGBOX_CMD_SYSTEM_PRINT_LOG      5u
#define MSGBOX_CMD_SYSTEM_PRINT_LOG_OK   6u

/*
 * Channel 1 — decode (audio/Include/audio_main.h MEDIA_MSGBOX_DECODE_CMD,
 * sequential from 0). DOCUMENTATION ONLY — defined as enum by the SDK:
 *   0 DECODE_NULL, 1 DEC_OPEN, 2 DEC_OPEN_ERR, 3 DEC_OPEN_CMPL,
 *   4 DECODE, 5 DECODE_CMPL, 6 DECODE_ERR, 7 DECODE_GETBUFFER,
 *   8 DECODE_GETBUFFER_CMPL, 9 DECODE_GETTIME, 10 DECODE_GETTIME_CMPL,
 *   11 DECODE_SEEK, 12 DECODE_SEEK_CMPL, 13 DECODE_CLOSE, 14 DECODE_CLOSE_CMPL
 *
 * Channel 2 — file (filesys/file.h MEDIA_MSGBOX_FILE_CMD, FILE_NULL=100):
 *   101 FILE_OPEN_CMPL, 102 FILE_OPEN_HANDSHK, 103 FILE_CREATE_CMPL,
 *   104 FILE_CREATE_HANDSHK, 105 FILE_SEEK, 106 FILE_SEEK_CMPL,
 *   107 FILE_READ, 108 FILE_READ_CMPL, 109 FILE_WRITE, 110 FILE_WRITE_CMPL,
 *   111 FILE_TELL, 112 FILE_TELL_CMPL, 113 FILE_GET_LENGTH,
 *   114 FILE_GET_LENGTH_CMPL, 115 FILE_CLOSE, 116 FILE_CLOSE_CMPL,
 *   117 FILE_CLOSE_HANDSHK
 *
 * Channel 1 — encode (audio/RecordControl/RecordControl.h,
 * ENCODE_NULL=200): 201 ENC_OPEN, 202 ENC_OPEN_CMPL, 203 ENCODE, ...
 */

#endif /* RECHORD_IPC_H */
