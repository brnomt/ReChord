/* FileInfo.h — file info structure (AudioControl.c / HoldonPlay.c). */
#ifndef FILEINFO_H
#define FILEINFO_H
#include "typedef.h"

typedef struct {
    uint32 Offset;
    uint32 FileSize;
    uint32 CurPos;
    uint32 TotalTime;    /* ms */
    uint32 CurTime;      /* ms */
    uint8  IsCue;
    uint8  PlayState;
    uint8  ErrCode;
    uint8  FileType;
    uint32 Clus;
    uint32 FstClus;
} FILE_INFO;

extern FILE_INFO FileInfo[];
#endif /* FILEINFO_H */

/* HANDLE — generic handle type (AudioControl.c casts with it) */
#ifndef HANDLE
typedef int8 HANDLE;
#endif
