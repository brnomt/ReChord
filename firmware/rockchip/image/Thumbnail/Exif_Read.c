#include "../ImageInclude/image_main.h"
#include "../ImageInclude/image_globals.h"

#include "SysInclude.h"

#ifdef THUMB_DEC_INCLUDE

#pragma arm section code = "ImageContrlCode", rodata = "ImageContrlCode", rwdata = "ImageContrlData", zidata = "ImageContrlBss"

#include  "fsinclude.h"
#include  "File.h"
#include "Exif_Read.h"



/*  TIFF Header */
typedef struct _tiff_Header {
    unsigned short byteOrder;                       /* �G���f�B�A��        */
    unsigned short reserved;                        /* �\��ς� 0x002A     */
    unsigned int Ifd0thOffset;                      /* 0TH IFD�̃I�t�Z�b�g */
} TIFF_HEADER;

/*  APP1 Exif Segment Header */
typedef struct _App1_Header {
    unsigned short marker;                          /* �}�[�J�[���� */
    unsigned short length;                          /* �}�[�J�[�̈撷 */
    char id[6]; /* "Exif\0\0" */                    /* Exif���� */
    TIFF_HEADER tiff;                               /* Tiff Header */
} APP1_HEADER;

typedef struct _tagNode TagNode;
struct _tagNode {                                   /* �^�O��� */
    unsigned short tagId;                           /* �^�OID */
    unsigned short type;                            /* �^�O�̌^ */
    unsigned int count;                             /* �^�O�̒��� */
    unsigned int numData[64];                       /* �^�������̏ꍇ�g�p */
    unsigned char byteData[32];                     /* �^��������̏ꍇ�g�p */
    unsigned short error;                           /* �Ǎ��G���[���   0:���� 0�ȊO�F�ُ� */
    TagNode *prev;                                  /* �O�̃^�O�̃|�C���^ */
    TagNode *next;                                  /* ���̃^�O�̃|�C���^ */
};

typedef struct _ifdTable IfdTable;
struct _ifdTable {                                  /*  IFD���*/
    IFD_TYPE ifdType;                               /* IFD�̎�� */
    unsigned short tagCount;                        /* IFD���̃^�O�� */
    TagNode  tags;                                  /* IFD���̃^�O�̐擪�A�h���X */
    unsigned int nextIfdOffset;                     /* ����IFD�܂ł̃I�t�Z�b�g */
    unsigned short offset;
    unsigned short length;
    unsigned char *p;
};

#define EXIF_ID_STR     "Exif\0\0"
#define EXIF_ID_STR_LEN 5
#define APP1_TIFF_OFFSET 10
#define APP12TIFF_OFFSET 0x0C

static int App1StartOffset = -1;
static int JpegDQTOffset = -1;
static APP1_HEADER App1Header;

static unsigned char    Info_Endian_Exif;           /* �G���f�B�A�� Big:0   Little:1 */
static unsigned char    Info_Endian_Sys;            /* �G���f�B�A�� Big:0   Little:1 */

/************************************************************************/
/* �O�� IF�p�ϐ� */
static long Exif_ImgWidth;
static long Exif_ImgLength;
static long Exif_Orientation;
static char Exif_DateTime[20];
static long Exif_ColorSpace;
static long Exif_FormatLength;
static long Exif_ThumbOfs;
static long Exif_FileSize;

static  char* Exif_FileName;
static  FILE *ExifFP;



/************************************************************************/
/* �����֐�  */
static void GetEndian(void);                                                                        /* �G���f�B�A���擾 */
static int GetExifInfo(const char *FileName, int *result);                                          /* Exif���擾 */
static int  GetHeader(FILE *fp);                                                                    /* �w�b�_���擾 */
void InitErrInfo(void);                                                                             /* �Ǎ��ُ펞������ */
static void InitApp1Header(void);                                                                   /* APP1��񏉊��� */
static int  GetApp1Marker(FILE *fp, const char *exif, unsigned char exif_len, int *dqtoffset);      /* APP1 Offset�擾 */
static int  GetApp1Header(FILE *fp);                                                                /* APP1 �w�b�_���擾 */
static void *Analyze_Ifd(FILE *fp, unsigned int offset, IFD_TYPE Ifd_type, TagNode *Exiftag);       /* IFD ��� */
static int seekToRelativeOffset(FILE *fp, unsigned int ofs);                                        /* �J�[�\���ʒu�ړ� */
static void *createIfdTable(IFD_TYPE IfdType, unsigned short tagCount, unsigned int nextOfs);       /* IFD �e�[�u���쐬 */
static void freeTagNode(void *pTag);                                                                /* ��������� Tag */
static void freeIfdTable(void *pIfd);                                                               /* ��������� IFD Table */
static TagNode *getTagNodePtrFromIfd(IfdTable *ifd, unsigned short tagId);                          /* �I�t�Z�b�g��� */
static char *getTagName(int ifdType, unsigned short tagId);                                         /* �^�O��� */

static void *addTagNodeToIfd(void *pIfd,
    unsigned short tagId,
    unsigned short type,
    unsigned int count,
    unsigned int *numData,
    unsigned char *byteData,
    TagNode *Exiftag);                                                         /* �^�O���ǉ� */


static void Analyze_TagInfo(TagNode *tag, TagNode *Exiftag);                   /* �^�O���i�[ */

static int systemIsLittleEndian(void);                                         /* �V�X�e���G���f�B�A������ */
static unsigned short Lit2Big16bit(unsigned short us);                         /* Little �� Big Endian 16bit */
static unsigned int Lit2Big32bit(unsigned int ui);                             /* Little �� Big Endian 32bit */



/*************************************************/
/*  Exif�����                                 */
/*************************************************/
int Exif_Read_Proc()
{
    int     res;                                                             /* �擾���� */

    App1StartOffset = -1;
    JpegDQTOffset = -1;
    Info_Endian_Exif = 0;
    Info_Endian_Sys = 0;
    InitApp1Header();
    if ((int) FileSeek(0, SEEK_SET, (HANDLE) ExifFP) != 0)
    {                 /* �擪�܂Ŗ߂� */
        return 0;
    }
    res = GetExifInfo(Exif_FileName, &res);                                  /* Exif���擾 */

    if (res < 0)
    {
        InitErrInfo();
        return 0;
    }

    /* �o�� */
    if ((int) FileSeek(0, SEEK_SET, (HANDLE) ExifFP) != 0)
    {                 /* �擪�܂Ŗ߂� */
        return 0;
    }

    return 1;
}

/* **************************************** */
/* Exif�Ǎ������ُ펞������                 */
/* **************************************** */
void InitErrInfo(void)
{
    Exif_ImgWidth = -1;
    Exif_ImgLength = -1;
    Exif_Orientation = -1;
    memset(Exif_DateTime, -1, 20);
    Exif_ColorSpace = -1;
    Exif_FormatLength = -1;
    Exif_ThumbOfs = -1;
    Exif_FileSize = -1;
}

/******************************************************/
/*  Exif���擾                                      */
/*  �����FFileName:Exif�t�@�C����                     */
/*          result�F��͌���   �O�F���� �O�ȊO�F�ُ�  */
/******************************************************/
static int GetExifInfo(const char *FileName, int *result)
{
    int sts = 0, ifdcnt = 0;                            /* �ُ���A IFD�� */
    unsigned int ifdOffset, nextoffset = 0;             /* IFD�ԃI�t�Z�b�g */
    FILE *fp = NULL;                                    /* Exif�t�@�C�� �t�@�C���f�B�X�N���v�^ */
    void **ppIfdArray = NULL;
    void *ifdArray[32];                                 /* IFD  �e�[�u�� */
    IfdTable *ifd_0th, *ifd_exif, *ifd_gps, *ifd_io, *ifd_1st;      /* IFD ��� */
    TagNode Exiftag;
    InitErrInfo();

    ifd_0th = ifd_exif = ifd_gps = ifd_io = ifd_1st = NULL;
    memset(ifdArray, 0, sizeof(ifdArray));
    memset(&Exiftag, 0, sizeof(TagNode));

    /* Get File Size */
    Exif_FileSize = (long)(FileGetSize((HANDLE) (ExifFP)));
    /* �V�X�e���G���f�B�A���擾 */
    Info_Endian_Sys = systemIsLittleEndian();

    /* �ŏ��̃w�b�_�����ǂݍ��� */
    sts = GetHeader(ExifFP);
    if (sts < 0)
    {
        return sts;
    }

    /* 0th IFD�Ǎ� */
    ifd_0th = (IfdTable *)Analyze_Ifd(ExifFP, App1Header.tiff.Ifd0thOffset, IFD_0TH, &Exiftag);
    if (ifd_0th == NULL) {
        sts = -1;
        return sts;
    }
    nextoffset = ifd_0th->nextIfdOffset;
    ifdArray[ifdcnt++] = ifd_0th;

    if (&Exiftag && &Exiftag.error) {
        ifdOffset = Exiftag.numData[0];                                 /* �I�t�Z�b�g���擾 */
        if (ifdOffset != 0) {
            ifd_exif = (IfdTable *)Analyze_Ifd(ExifFP, ifdOffset, IFD_EXIF, &Exiftag);
            if (ifd_exif) {
                ifdArray[ifdcnt++] = ifd_exif;
            }
            else {
                /* EXIF_IFD�擾���s */
                sts = -1;
            }
        }
    }

    /* 1st IFD */
    ifdOffset = nextoffset;                 /* �I�t�Z�b�g���擾 */
    if (ifdOffset != 0) {
        ifd_1st = (IfdTable *)Analyze_Ifd(ExifFP, ifdOffset, IFD_1ST, &Exiftag);
        if (ifd_1st) {
            ifdArray[ifdcnt++] = ifd_1st;
        }
        else {
            /* 1st_IFD�擾���s */
            sts = -1;
        }
    }

    return sts;
}

/* **************************************** */
/* APP1�w�b�_��� ������                    */
/* **************************************** */
void InitApp1Header(void)
{
    memset(&App1Header, 0, sizeof(APP1_HEADER));
    App1Header.marker = 0xFFE1;
    App1Header.length = 0;
    //strcpy(App1Header.id, "Exif");
    strncpy(App1Header.id, "Exif",4);
    App1Header.tiff.byteOrder = 0x4D4D; /* means Big-endian */
    App1Header.tiff.reserved = 0x002A;  /* Unique */
    App1Header.tiff.Ifd0thOffset = 0x00000008;
}

/* ***************************************** */
/* �w�b�_���擾                            */
/* �����FExif�t�@�C�� �f�B�X�N���v�^         */
/* ***************************************** */
static int GetHeader(FILE *fp)
{
    int sts = 0, dqtOffset = 0;
    InitApp1Header();                                                       /* �w�b�_������ */

    sts = GetApp1Marker(fp, EXIF_ID_STR, EXIF_ID_STR_LEN, &dqtOffset);      /* APP�w�b�_���擾 */
    if (sts < 0) {
    /* �ُ팟�m */
        return sts;
    }
    JpegDQTOffset = dqtOffset;                                              /* DQT �I�t�Z�b�g�ۑ� */
    App1StartOffset = sts;                                                  /* App1 �I�t�Z�b�g�ۑ� */
    if (sts < 0) {
    /* �ُ팟�m */
        return sts;
    }
    /* Load the segment header */
    if (!GetApp1Header(fp)) {                                               /* APP�P ���擾 */
    /* �ُ팟�m */
        return 3;
    }

    return 1;

}

static int systemIsLittleEndian(void)
{
    static int i = 1;
    return (int)(*(char*)&i);
}

/* *************************************** */
/* APP1 �I�t�Z�b�g���擾                 */
/* �����F*fp:Exif�t�@�C���f�B�X�N���v�^    */
/*          *exif:���ʃR�[�h������         */
/*           exif_len:������̒���         */
/*           *pdqtoffset:DQT�I�t�Z�b�g     */
/* *************************************** */
static int GetApp1Marker(FILE *fp, const char *exif, unsigned char exif_len, int *pdqtoffset)
{
    int pos;                                                                /* �J�[�\���ʒu */
    unsigned char buf[64];                                                  /*  */
    unsigned short len, marker;                                             /*  */

    if (FileRead((uint8*) &marker, (1 * sizeof(short)), (HANDLE) fp) < sizeof(short)) {             /* SOI �J�n�o�C�g �擾 */
        return -1;
    }

    if (Info_Endian_Sys) {
        marker = Lit2Big16bit(marker);                                      /* �G���f�B�A���C��  */
    }

    if (marker != 0xFFD8) {                                                 /* SOI Crashed */
        return -1;
    }

    if (FileRead((uint8*) &marker, (1 * sizeof(short)), (HANDLE) fp) < sizeof(short)) {             /* APP Marker �擾 */
        return -1;
    }

    if (Info_Endian_Sys) {
        marker = Lit2Big16bit(marker);                                      /* �G���f�B�A���C��  */
    }

    if (marker == 0xFFDB) {                                                 /* not exist Exif */
        return -1;
    }

    pos = FileTell((HANDLE)(fp));
    while (1){
        /* unexpected value. is not a APP[0-14] marker */
        if (!(marker >= 0xFFE0 && marker <= 0xFFEF)) {
            /* found DQT */
            if (marker == 0xFFDB && pdqtoffset != NULL) {
                *pdqtoffset = pos - sizeof(short);
            }
            break;
        }


        if (FileRead((uint8*) &len, (1 * sizeof(short)), (HANDLE) fp) < sizeof(short)) {            /* APP length �擾 */
            return -1;
        }

        if (Info_Endian_Sys) {
            len = Lit2Big16bit(len);                                        /* �G���f�B�A���C��  */
        }

        /* if is not a APP1 segment, move to next segment */
        if (marker != 0xFFE1) {
            if ((int) FileSeek(len - sizeof(short), SEEK_CUR, (HANDLE) fp) != 0) {
                return -2;
            }
        }
        else {
            /* check if it is the Exif segment */
            if (FileRead((uint8*) &buf, exif_len, (HANDLE) fp) < exif_len) {                  /* Exif ���ʃR�[�h�擾 */
                return -1;
            }
            if (memcmp(buf, exif, exif_len) == 0) {                         /* ���ʃR�[�h�F�� */
                /* return the start offset of the Exif segment */
                return pos - sizeof(short);
            }
            /* if is not a Exif segment, move to next segment */
            if (((int) FileSeek(pos, SEEK_SET, (HANDLE) fp) != 0) ||                            /* length�܂Ŗ߂� */
                    ((int) FileSeek(len, SEEK_CUR, (HANDLE) fp) != 0)) {                            /* �ǂݍ���length������������Marker�ʒu�ɂ��� */
                return -2;
            }
        }

        /* read next marker */
        if (FileRead((uint8*) &marker, (1 * sizeof(short)), (HANDLE) fp) < sizeof(short)) {
            return -1;
        }

        if (Info_Endian_Sys) {
            marker = Lit2Big16bit(marker);                                          /* �G���f�B�A���C��  */
        }
        pos = FileTell((HANDLE)(fp));
    }
    return -1; /* not found the Exif segment */

}

/* ******************************** */
/* APP1 �w�b�_���擾                 */
/* �����FExif�t�@�C���f�B�X�N���v�^ */
/* ******************************** */
static int GetApp1Header(FILE *fp)
{

    /* read the APP1 header */
    if (((int) FileSeek(App1StartOffset, SEEK_SET, (HANDLE) fp) != 0) ||
        ( FileRead((uint8*) &App1Header, (1 * sizeof(APP1_HEADER)), (HANDLE) fp) <  sizeof(APP1_HEADER))) {
        return 0;
    }

    if (((int) FileSeek(App1StartOffset + APP1_TIFF_OFFSET, SEEK_SET, (HANDLE) fp) != 0) ||         /* TIFF Header �Ǎ� */
        ( FileRead((uint8*) &App1Header.tiff, (1 * sizeof(TIFF_HEADER)), (HANDLE) fp) < sizeof(TIFF_HEADER))) {
        return 0;
    }

    if (Info_Endian_Sys) {
        App1Header.length = Lit2Big16bit(App1Header.length);
    }

    /* byte-order identifier */
    if (App1Header.tiff.byteOrder != 0x4D4D && /* big-endian */
        App1Header.tiff.byteOrder != 0x4949) { /* little-endian */
        return 0;
    }
    else if (App1Header.tiff.byteOrder == 0x4949) {
        Info_Endian_Exif = 1;   /* little-endian */
    }else{
        Info_Endian_Exif = 0;   /* big-endian */
    }

    if (Info_Endian_Exif != 1)
    {
        App1Header.tiff.reserved = Lit2Big16bit(App1Header.tiff.reserved);
    }
    /* TIFF version number (always 0x002A) */
    if (App1Header.tiff.reserved != 0x002A) {
        return 0;
    }

    /* offset of the 0TH IFD */
    if (Info_Endian_Exif != 1)
    {
        App1Header.tiff.Ifd0thOffset = Lit2Big32bit(App1Header.tiff.Ifd0thOffset);
    }
    return 1;
}

/* ******************************************* */
/* IFD ���                                    */
/* �����F�������FExif�t�@�C���f�B�X�N���v�^    */
/*          offset:t�^�O���܂ł̃I�t�Z�b�g   */
/*          Ifd_type�FIFD�̎��                */
/* ******************************************* */
static void *Analyze_Ifd(FILE *fp, unsigned int offset, IFD_TYPE Ifd_type, TagNode *Exiftag)
{
    IfdTable ifd;
    unsigned char buf[64];
    unsigned short tagCount, us;
    unsigned int nextOffset = 0;
    unsigned int array[32], val, allocSize;
    int size, cnt, i;
    size_t len;
    int pos;

    uint32  filesize = 0;
    filesize = FileGetSize((HANDLE) fp);

    if ((offset < 0) || (filesize <= offset )) {
        /* offset value is illegal */
        goto ERR;
    }

    if ((seekToRelativeOffset(fp, offset) != 0) ||                        /* Offset���ړ� */
        (FileRead((uint8*) &tagCount, (1 * sizeof(short)), (HANDLE) fp) < sizeof(short))) {       /* �^�O���Ǎ� */
        return NULL;
    }

    if (Info_Endian_Exif != 1)
    {
        tagCount = Lit2Big16bit(tagCount);
    }

    pos = FileTell((HANDLE)(fp));

    if (Ifd_type == IFD_0TH) {
        /*  next IFD's offset is at the tail of the segment */
        if ((seekToRelativeOffset(fp,
            sizeof(TIFF_HEADER) + sizeof(short) + sizeof(TAG_FIELD) * tagCount) != 0) || /* ����IFD�܂ł�Offset��ǂݍ��ނ��ߍŌ�̃^�O�܂ňړ� */
            (FileRead((uint8*) &nextOffset, (1 * sizeof(int)), (HANDLE) fp) < sizeof(int))) {
            return NULL;
        }
        if (Info_Endian_Exif != 1)
        {
            nextOffset = Lit2Big32bit(nextOffset);
        }
        FileSeek(pos, SEEK_SET, (HANDLE) fp);                                       /* �ړ����������ɖ߂� */
    }

    memset(&ifd, 0, sizeof(IfdTable));
    ifd.ifdType = Ifd_type;
    ifd.tagCount = tagCount;
    ifd.nextIfdOffset = nextOffset;

    /*  parse all tags */
    for (cnt = 0; cnt < tagCount; cnt++) {
        TAG_FIELD tag;
        unsigned char data[4];

        if (pos >= filesize)
        {
            /* File Size Over */
            goto ERR;
        }

        if (((int) FileSeek(pos, SEEK_SET, (HANDLE) fp) != 0) ||
            (FileRead((uint8*) &tag, (1 * sizeof(tag)), (HANDLE) fp) < sizeof(tag))) {
            goto ERR;
        }

        memcpy(data, &tag.offset, 4); /*  keep raw data temporary */
        if (Info_Endian_Exif != 1)
        {
            tag.tag = Lit2Big16bit(tag.tag);
            tag.type = Lit2Big16bit(tag.type);
            tag.count = Lit2Big32bit(tag.count);
            tag.offset = Lit2Big32bit(tag.offset);
        }

        if (tag.tag == 0x0) {
            goto ERR;
        }

        if ((tag.count >= 32))
        {
            tag.count = 31;
        }
        pos = FileTell((HANDLE)(fp));

        if ((tag.type == TYPE_ASCII) ||     /*  ascii = the null-terminated string */
            (tag.type == TYPE_UNDEFINED)) { /*  undefined = the chunk data bytes */
            if (tag.count <= 4)  {
                /*  4 bytes or less data is placed in the 'offset' area directly */
                addTagNodeToIfd(&ifd, tag.tag, tag.type, tag.count, NULL, data, Exiftag);
            }
            else {
                /*  5 bytes or more data is placed in the value area of the IFD */
                unsigned char p[32];

                if (tag.count > sizeof(buf)) {
                    /* allocate new buffer if needed */
                    if (tag.count >= App1Header.length) { /* illegal */
                    }
                    else {
                    }
                    if (!p) {
                        /*  treat as an error */
                        addTagNodeToIfd(&ifd, tag.tag, tag.type, tag.count, NULL, NULL, Exiftag);
                        continue;
                    }
                    memset(p, 0, tag.count);
                }
                if ((seekToRelativeOffset(fp, tag.offset) != 0) ||
                    (FileRead((uint8*) p, tag.count, (HANDLE) fp) < tag.count)) {
                    if (p != &buf[0]) {
                    }
                    addTagNodeToIfd(&ifd, tag.tag, tag.type, tag.count, NULL, NULL, Exiftag);
                    continue;
                }
                addTagNodeToIfd(&ifd, tag.tag, tag.type, tag.count, NULL, p, Exiftag);
                if (p != &buf[0]) {
                }
            }
        }
        else if ((tag.type == TYPE_RATIONAL) || (tag.type == TYPE_SRATIONAL)) {
            unsigned int realCount = tag.count * 2; /*  need double the space */
            size_t len = realCount * sizeof(int);
            if (len >= App1Header.length) { /* illegal */
            }
            else {
                if (array) {
                    if ((seekToRelativeOffset(fp, tag.offset) != 0) ||
                        (FileRead((uint8*) array, len, (HANDLE) fp) < len)) {
                    }
                    else {
                        for (i = 0; i < (int)realCount; i++) {
                            array[i] = Lit2Big32bit(array[i]);
                        }
                    }
                }
            }
            addTagNodeToIfd(&ifd, tag.tag, tag.type, tag.count, array, NULL, Exiftag);
            if (array) {
            }
        }
        else if ((tag.type == TYPE_BYTE) ||
            (tag.type == TYPE_SHORT) ||
            (tag.type == TYPE_LONG) ||
            (tag.type == TYPE_SBYTE) ||
            (tag.type == TYPE_SSHORT) ||
            (tag.type == TYPE_SLONG)) {

            /*  the single value is always stored in tag.offset area directly */
            /*  # the data is Left-justified if less than 4 bytes */
            if (tag.count <= 1) {
                val = tag.offset;
                if ((tag.type == TYPE_BYTE) || (tag.type == TYPE_SBYTE)) {
                    unsigned char uc = data[0];
                    val = uc;
                }
                else if ((tag.type == TYPE_SHORT) || (tag.type == TYPE_SSHORT)) {
                    memcpy(&us, data, sizeof(short));
                    if (Info_Endian_Exif != 1)
                    {
                        us = Lit2Big16bit(us);
                    }
                    val = us;
                }
                addTagNodeToIfd(&ifd, tag.tag, tag.type, tag.count, &val, NULL, Exiftag);
            }
            /*  multiple value */
            else {
                size = sizeof(int);
                if ((tag.type == TYPE_BYTE) || (tag.type == TYPE_SBYTE)) {
                    size = sizeof(char);
                }
                else if ((tag.type == TYPE_SHORT) || (tag.type == TYPE_SSHORT)) {
                    size = sizeof(short);
                }
                /*  for the sake of simplicity, using the 4bytes area for */
                /*  each numeric data type */
                allocSize = sizeof(int) * tag.count;
                if (allocSize >= App1Header.length) { /*  illegal */
                }
                else {
                }
                if (!array) {
                    addTagNodeToIfd(&ifd, tag.tag, tag.type, tag.count, NULL, NULL, Exiftag);
                    continue;
                }
                len = size * tag.count;
                /*  if the total length of the value is less than or equal to 4bytes, */
                /*  they have been stored in the tag.offset area */
                if (len <= 4) {
                    if (size == 1) { /*  byte */
                        for (i = 0; i < (int)tag.count; i++) {
                            array[i] = (unsigned int)data[i];
                        }
                    }
                    else if (size == 2) { /*  short */
                        for (i = 0; i < 2; i++) {
                            memcpy(&us, &data[i * 2], sizeof(short));
                            if (Info_Endian_Exif != 1)
                            {
                                us = Lit2Big16bit(us);
                            }
                            array[i] = (unsigned int)us;
                        }
                    }
                }
                else {
                    if ((seekToRelativeOffset(fp, tag.offset) != 0) ||
                        (FileRead((uint8*) buf, len, (HANDLE) fp) < len)) {
                        addTagNodeToIfd(&ifd, tag.tag, tag.type, tag.count, NULL, NULL, Exiftag);
                        continue;
                    }
                    for (i = 0; i < (int)tag.count; i++) {
                        if ((i * size ) >= 32 ) {
                            break;
                        }
                        memcpy(&val, &buf[i*size], size);
                        if (size == sizeof(int)) {
                            val = Lit2Big32bit(val);
                        }
                        else if (size == sizeof(short)) {
                            if (Info_Endian_Exif != 1)
                            {
                                val = Lit2Big16bit((unsigned short)val);
                            }
                        }
                        array[i] = (unsigned int)val;
                    }
                }
                addTagNodeToIfd(&ifd, tag.tag, tag.type, tag.count, array, NULL, Exiftag);
            }
        }
    }

    return &ifd;
ERR:
    return NULL;
}

/* ************************************* */
/* �J�[�\���ʒu�ړ�                      */
/* �����F*fp:Exif�t�@�C���f�B�X�N���v�^  */
/*          ofs�F�ړ���                  */
/* ************************************* */
static int seekToRelativeOffset(FILE *fp, unsigned int ofs)
{
    static int start = APP12TIFF_OFFSET;
    return (int) FileSeek((App1StartOffset + start - 2) + ofs, SEEK_SET, (HANDLE) fp);
}

/* *************************************** */
/* IFD �e�[�u���쐬                        */
/* �����FIfdType�FIFD�̎��                */
/*          tagCount�F�^�O��               */
/*          nextOfs:��IFD�܂ł̃I�t�Z�b�g  */
/* *************************************** */
static void *createIfdTable(IFD_TYPE IfdType, unsigned short tagCount, unsigned int nextOfs)
{
    IfdTable ifd;

    memset(&ifd, 0, sizeof(IfdTable));
    ifd.ifdType = IfdType;
    ifd.tagCount = tagCount;
    ifd.nextIfdOffset = nextOfs;
    return 0;
}

/* ******************************** */
/* ��������� IFD Table             */
/* �����F����Ifd�FIFD���           */
/* ******************************** */
static void freeIfdTable(void *pIfd)
{
    IfdTable *ifd = (IfdTable*)pIfd;
    TagNode *tag;
    if (!ifd) {
        return;
    }
    tag = &ifd->tags;
    if (ifd->p) {
    }

    if (tag) {
        while (tag->next) {
            tag = tag->next;
        }
        while (tag) {
            TagNode *tagWk = tag->prev;
            freeTagNode(tag);
            tag = tagWk;
        }
    }
    return;
}

/* ******************************** */
/* ��������� Tag���               */
/* �����F*pTag�F�^�O���            */
/* ******************************** */
static void freeTagNode(void *pTag)
{
    TagNode *tag = (TagNode*)pTag;
    if (!tag) {
        return;
    }
    if (tag->numData) {
    }
    if (tag->byteData) {
    }
}

/*  search the specified tag's node from the IFD table */
/* ************************************************** */
/* �^�O���擾                                       */
/* �����F*ifd:�w�肵��IFD���                         */
/*      tagId:�擾�����^�OID                          */
/* ************************************************** */
static TagNode *getTagNodePtrFromIfd(IfdTable *ifd, unsigned short tagId)
{
    TagNode *tag;
    tag = &ifd->tags;
    while (tag) {
        if (tag->tagId == tagId) {
            return tag;
        }
        tag = tag->next;
    }
    return NULL;
}

/* ***************************************************** */
/* �^�O�l�[���擾����                                    */
/* �����FifdType�FIFD�̎��                              */
/*          tagId�F�^�OID�i���o�[                        */
/* ***************************************************** */
static char *getTagName(int ifdType, unsigned short tagId)
{
    return 0;
}

/*  add the TagNode enrtry to the IFD table */
/* ************************************************ */
/* �^�O���擾                                     */
/* �����F����Ifd�F�i�[IFD�̃|�C���^                 */
/*          tagId�F�^�OID                           */
/*          type�F�^�O�̌^                          */
/*          count�F�^�O�̗�                         */
/*          numdata�F����                           */
/*          bytedata�F������                        */
/* ************************************************ */
static void *addTagNodeToIfd(void *pIfd,
    unsigned short tagId,
    unsigned short type,
    unsigned int count,
    unsigned int *numData,
    unsigned char *byteData,
    TagNode *Exiftag)
{
    int i;
    IfdTable *ifd = (IfdTable*)pIfd;
    TagNode tag;
    if (!ifd) {
        return NULL;
    }

    memset(&tag, 0, sizeof(TagNode));
    tag.tagId = tagId;
    tag.type = type;
    tag.count = count;

    if (count > 0) {
        if (numData != NULL) {
            int num = count;
            if ((type == TYPE_RATIONAL) ||                            /* LONG2�Ԃ�Ȃ̂� */
                (type == TYPE_SRATIONAL)) {
                num *= 2;
                if (num >= 32) {
                    num = 31;
                }
            }
            for (i = 0; i < num; i++) {
                tag.numData[i] = numData[i];
            }
        }
        else if (byteData != NULL) {
            memcpy(tag.byteData, byteData, count);
        }
        else {
            tag.error = 1;
        }
        Analyze_TagInfo(&tag, Exiftag);
    }
    else {
        tag.error = 1;
    }

    return 0;
}

/* ************************************************ */
/* �^�O���i�[                                     */
/* �����F��tag�Ftag���̃|�C���^                   */
/* ************************************************ */
static void Analyze_TagInfo(TagNode *tag, TagNode *Exiftag)
{
    switch (tag->tagId) {
    case    TAG_JPEGInterchangeFormat:
        Exif_ThumbOfs = (long)(*tag->numData + App1StartOffset + APP12TIFF_OFFSET - 2);
        break;
    case    TAG_ExifIFDPointer:
        memcpy(Exiftag->byteData, tag->byteData, sizeof(tag->byteData));
        Exiftag->count = tag->count;
        Exiftag->error = tag->error;
        memcpy(Exiftag->numData, tag->numData, sizeof(tag->numData));
        Exiftag->tagId = tag->tagId;
        Exiftag->type = tag->type;
        break;
    case    TAG_PixelXDimension:
        Exif_ImgWidth = (long)*tag->numData;
        break;
    case    TAG_PixelYDimension:
        Exif_ImgLength = (long)*tag->numData;
        break;
    case    TAG_Orientation:
        Exif_Orientation = (long)*tag->numData;
        break;
    case    TAG_DateTimeOriginal:
        memcpy(Exif_DateTime, tag->byteData, tag->count);
        break;
    case    TAG_ColorSpace:
        Exif_ColorSpace = (long)*tag->numData;
        break;
    case    TAG_JPEGInterchangeFormatLength:
        Exif_FormatLength = (long)*tag->numData;
        break;
    default:
        break;
    }
}

static unsigned short Lit2Big16bit(unsigned short us)              /* Little �� Big Endian 16bit */
{
    return (us << 8) | ((us >> 8) & 0x00FF);
}


static unsigned int Lit2Big32bit(unsigned int ui)                  /* Little �� Big Endian 32bit */
{
    return
        ((ui << 24) & 0xFF000000) | ((ui << 8) & 0x00FF0000) |
        ((ui >> 8) & 0x0000FF00) | ((ui >> 24) & 0x000000FF);
}


/* ---------------------------------------------------------------------------------------------- */
/*  �O��I/F�֐�                                                                                   */
/* ---------------------------------------------------------------------------------------------- */
void Exif_Proc_SetFileName(char *buf[])
{
    Exif_FileName = buf[0];
}

void Exif_Proc_SetFilePointer(FILE *fp)
{
    ExifFP = fp;
}

int Exif_Proc_SeekFilePointer(FILE *fp, long ofs)
{
    if ((int) FileSeek((int)ofs, SEEK_SET, (HANDLE) fp) != 0) {
                return -1;
    }
}

long Exif_Proc_GetImgWidth(void)
{
    return Exif_ImgWidth;
}

long Exif_Proc_GetImgLength(void)
{
    return Exif_ImgLength;
}

long Exif_Proc_GetOrientation(void)
{
    return Exif_Orientation;
}

long Exif_Proc_GetColorSpace(void)
{
    return Exif_ColorSpace;
}

long Exif_Proc_GetFormatLength(void)
{
    return Exif_FormatLength;
}

unsigned char *Exif_Proc_GetDateTime(void)
{
    return &Exif_DateTime[0];
}

long Exif_Proc_GetThumbnailOffset(void)
{
    return Exif_ThumbOfs;
}


long Exif_Proc_GetFileSize(void)
{
    return Exif_FileSize;
}

#endif /* JPG_DEC_INCLUDE */

