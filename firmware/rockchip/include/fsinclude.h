/* fsinclude.h — Filesystem includes (FAT, media lib).
 * Auto-created for SDK build (Aug 2026).
 */
#ifndef FSINCLUDE_H
#define FSINCLUDE_H

#include "typedef.h"

#include "fat.h"
#include "media_lib.h"
#include "SysFindFile.h"

/* Integration fix (2026-10-09): expose the VENDOR type headers before the
 * legacy duplicate blocks below. The blocks are guarded, so the vendor
 * definitions (FDT.h / dir.h / file.h) always win. This header is ours
 * ("auto-created for SDK build") and used to define its own copies, which
 * caused redefinition/semantics clashes. */
#include "FDT.h"
#include "dir.h"
#include "file.h"
#include "MemDev.h"        /* DataDiskID, MEDIAINFO_BLOCK_SIZE, disk config */
#include "AddrSaveMacro.h" /* MUSIC_TREE_ and RECORD_TREE_ sector constants */
#include "SortInfoGetMacro.h"


#endif /* FSINCLUDE_H */

/* ---- Filesystem find types (SysFindFile.h) ---- */
#ifndef FSINCLUDE_FIND
#define FSINCLUDE_FIND
#define MAX_DIR_DEPTH   8

#ifndef _FDT_DEFINED
#define _FDT_DEFINED
typedef struct _FDT {
    uint32  dwFileSize;
    uint8   bFileType;        /* 0=file, 1=dir */
    uint8   bFileAttr;
    char    szFileName[256];
    char    szPath[256];
    char    Name[256];        /* RecordControl.c uses .Name */
    uint8   Attr;
} FDT;
#endif /* _FDT_DEFINED */


/* FIND_DATA now comes from vendor FDT.h (the search-cursor layout): the
 * duplicate handle-layout copy that used to live here had zero users and
 * won include-order roulette against FDT.h, breaking dir.c. Unified
 * 2026-10-09. */

#endif

/* ---- Filesystem sample-rate constants ---- */
#ifndef FSINCLUDE_FS
#define FSINCLUDE_FS
#endif

/* MAX_FILENAME_LEN (AudioControl.h) */
#ifndef FSINCLUDE_NAME
#define FSINCLUDE_NAME
#define MAX_FILENAME_LEN   256
#define MAX_PATH_LEN       512
#endif

/* ---- Sort types (SysFindFile.c). DB-type names (MUSIC_DB, RECORD_DB,
 * VIDEO_DB, PICTURE_DB, TEXT_DB) belong to the vendor dir.h FS_TYPE enum and
 * were removed here on 2026-10-10 (macros destroyed the enum members). ---- */
#ifndef FSINCLUDE_DBTYPES
#define FSINCLUDE_DBTYPES
/* duplicate FS_TYPE (vendor dir.h owns the full enum, incl. FS_FAT_EX_VOICE) removed 2026-10-10 */



#define SORT_TYPE_SEL_FOLDER     0
#define MUSIC_TYPE_SEL_FMFILE    0
#define SORT_TYPE_SEL_SONGFILE   1
#endif

/* removed FS_FAT (vendor dir.h FS_TYPE enum member) */
#define SORT_TYPE_SEL_BROWSER  2

#define MUSIC_TYPE_SEL_RECORDFILE  2
/* removed NOT_FIND_FILE (vendor file.h owns 0x13) */
#define SORT_TYPE_SEL_NOW_PLAY     3

#define MAX_OPEN_FILES  16
#define SORT_FILENUM_DEFINE  0

/* MEDIA_MSGBOX_FILE_CMD — copied VERBATIM from filesys/file.h (the owner
 * of these IDs). We cannot include that header here: the synthetic header
 * stack (fat.h/media_lib.h/FileInfo.h) conflicts with its declarations.
 * If filesys/file.h ever changes, re-sync this enum AND firmware/ipc.h (the
 * shared contract doc for the AP build). NOTE: before Aug 2026 these IDs
 * came from the mailbox.h shim with WRONG 0x0102-style values that the
 * stock AP does not recognize (file channel silently dead).
 * (Guarded: this file's main include-guard ends above — see line 14.) */
#ifndef MEDIA_MSGBOX_FILE_CMD_DEFINED
#define MEDIA_MSGBOX_FILE_CMD_DEFINED
/* duplicate MEDIA_MSGBOX_FILE_CMD (vendor file.h owns it) removed 2026-10-10 */

#endif /* MEDIA_MSGBOX_FILE_CMD_DEFINED */
