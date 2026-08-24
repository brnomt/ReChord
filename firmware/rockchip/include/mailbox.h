/* mailbox.h — ReChord compatibility shim for the RKnanoD inter-core mailbox.
 *
 * IMPORTANT — command IDs: this header is included EARLY (SysInclude.h /
 * DriverInclude.h), BEFORE the SDK headers that declare the media commands
 * as ENUMS. Defining MEDIA_MSGBOX_CMD_* as macros here breaks those enum
 * declarations (the macro expands inside the enumerator list), so this shim
 * must NOT define them. Include the owning SDK header instead:
 *   - MSGBOX_SYSTEM_CMD        driver/BB/BBSystem.h          (enum)
 *   - MEDIA_MSGBOX_DECODE_CMD  audio/Include/audio_main.h    (enum)
 *   - MEDIA_MSGBOX_FILE_CMD    filesys/file.h                (enum)
 *   - MEDIA_MSGBOX_ENCODE_CMD  audio/RecordControl/RecordControl.h (enum)
 *
 * The system commands are kept here as macros (correct, sequential values —
 * the pre-2026 shim had BB_HOLD=3/ACK=4/EXIT=5, which the stock AP does not
 * recognize) because several BB files (BSP2.c, Debug2.c) include only
 * SysInclude.h and would otherwise have no definition. Nothing in the tree
 * includes BBSystem.h, so there is no enum collision for these.
 */
#ifndef MAILBOX_H
#define MAILBOX_H

#include "typedef.h"

/* ---- Mailbox channel / ID assignment ---- */
#define MAILBOX_ID_0        0
#define MAILBOX_ID_1        1
#define MAILBOX_ID_2        2
#define MAILBOX_ID_3        3

/* interrupt-enable BITMASKS (real SDK: 1<<n, NOT the channel number) */
#define MAILBOX_INT_0       ((uint32)(1 << 0))
#define MAILBOX_INT_1       ((uint32)(1 << 1))
#define MAILBOX_INT_2       ((uint32)(1 << 2))
#define MAILBOX_INT_3       ((uint32)(1 << 3))

#define MAILBOX_CHANNEL_0   0
#define MAILBOX_CHANNEL_1   1
#define MAILBOX_CHANNEL_2   2
#define MAILBOX_CHANNEL_3   3

/* ---- MSGBOX_SYSTEM_CMD (BBSystem.h enum values — sequential from 0) ----
 * ch0 system: 0=NULL, 1=START_OK, 2=BB_HOLD, 3=BB_HOLD_ACK,
 *             4=BB_HOLD_EXIT, 5=PRINT_LOG, 6=PRINT_LOG_OK */
#define MSGBOX_CMD_SYSTEM_START_OK          0x0001
#define MSGBOX_CMD_BB_HOLD                  0x0002
#define MSGBOX_CMD_BB_HOLD_ACK              0x0003
#define MSGBOX_CMD_BB_HOLD_EXIT             0x0004
#define MSGBOX_CMD_SYSTEM_PRINT_LOG         0x0005
#define MSGBOX_CMD_SYSTEM_PRINT_LOG_OK      0x0006

/* ---- API (real mailbox.c signatures: reads return uint32, others rk_err_t) ---- */
API uint32   MailBoxReadA2BCmd(uint32 id, uint32 channel);
API uint32   MailBoxReadA2BData(uint32 id, uint32 channel);
API uint32   MailBoxReadB2ACmd(uint32 id, uint32 channel);
API uint32   MailBoxReadB2AData(uint32 id, uint32 channel);
API rk_err_t MailBoxWriteA2BCmd(uint32 cmd, uint32 id, uint32 channel);
API rk_err_t MailBoxWriteA2BData(uint32 data, uint32 id, uint32 channel);
API rk_err_t MailBoxWriteB2ACmd(uint32 cmd, uint32 id, uint32 channel);
API rk_err_t MailBoxWriteB2AData(uint32 data, uint32 id, uint32 channel);
API rk_err_t MailBoxClearA2BInt(uint32 id, uint32 int_sel);
API rk_err_t MailBoxClearB2AInt(uint32 id, uint32 int_sel);
API rk_err_t MailBoxEnableA2BInt(uint32 id, uint32 int_sel);
API rk_err_t MailBoxEnableB2AInt(uint32 id, uint32 int_sel);
API rk_err_t MailBoxDisableA2BInt(uint32 id, uint32 int_sel);
API rk_err_t MailBoxDisableB2AInt(uint32 id, uint32 int_sel);
API void MailBoxInit(void);

#endif /* MAILBOX_H */
