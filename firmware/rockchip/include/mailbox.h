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
 * The channel/ID/INT assignment and the MSGBOX_CMD_SYSTEM_* values are the
 * shared wire contract — they now live in firmware/ipc.h (single source for
 * both the AP and BB builds), which this header includes.
 */
#ifndef MAILBOX_H
#define MAILBOX_H

#include "typedef.h"
#include "ipc.h"

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
