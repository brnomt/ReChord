
#ifndef __AVCTP_H__
#define __AVCTP_H__

#define AVCTP_CONTROL_PSM           23
#define AVCTP_BROWSING_PSM          27

#define AVC_MTU                     512
#define AVC_HEADER_LENGTH           3
#define AVCTP_HEADER_LENGTH         3

#define AV_REMOTE_TARGET_SVCLASS_ID 0x110c
#define AV_REMOTE_SVCLASS_ID        0x110e

/* ctype entries */
#define AVC_CTYPE_CONTROL           0x0
#define AVC_CTYPE_STATUS            0x1
#define AVC_CTYPE_SPECIFIC_INQUIRY  0x2     //not use
#define AVC_CTYPE_NOTIFY            0x3
#define AVC_CTYPE_GENERAL_INQUIRY   0x4     //not use
/*Response entries */
#define AVC_CTYPE_NOT_IMPLEMENTED   0x8
#define AVC_CTYPE_ACCEPTED          0x9
#define AVC_CTYPE_REJECTED          0xA
#define AVC_CTYPE_STABLE            0xC     //IMPLEMENTED / STABLE 
#define AVC_CTYPE_CHANGED           0xD
#define AVC_CTYPE_INTERIM           0xF

/* General commands opcodes  defined by 1394ta*/
#define AVC_OP_VENDORDEP            0x00        //VENDOR-DEPENDENT
#define AVC_OP_UNITINFO             0x30
#define AVC_OP_SUBUNITINFO          0x31
#define AVC_OP_PASSTHROUGH          0x7c
/* opcodes 80-9F and E0-FF are not used by 1394ta.Sneak one for the browsing channel */
#define AVRC_OP_BROWSE      0xFF    /* Browsing */
#define AVRC_OP_INVALID     0xFE    /* invalid one */

/* Company ID's*/
#define AVRC_CO_BLUETOOTH_SIG   0x00FFFFFF
#define AVRC_CO_WIDCOMM         0x00000361
#define AVRC_CO_BROADCOM        0x00001018
#define AVRC_CO_METADATA        0x00001958  /* Unique COMPANY ID for Metadata messages */


/* subunits of interest */
#define AVC_SUBUNIT_PANEL           0x09
#define AVC_SUBUNIT_MONITOR         0X00
#define AVC_SUBUNIT_AUDIO           0X01
#define AVC_SUBUNIT_PRINTER         0X02
#define AVC_SUBUNIT_DISC            0X03
#define AVC_SUBUNIT_TAPE_PLAYER     0X04


#define AVRC_UNIT_OPRND_BYTES   5
#define AVRC_SUB_OPRND_BYTES    4
#define AVRC_SUBRSP_OPRND_BYTES 3
#define AVRC_SUB_PAGE_MASK      7
#define AVRC_SUB_PAGE_SHIFT     4
#define AVRC_SUB_EXT_CODE       7
#define AVRC_PASS_OP_ID_MASK    0x7F
#define AVRC_PASS_STATE_MASK    0x80
#define AVRC_CMD_OPRND_PAD      0xFF

#define AVRC_CTYPE_MASK         0x0F
#define AVRC_SUBTYPE_MASK       0xF8
#define AVRC_SUBTYPE_SHIFT      3
#define AVRC_SUBID_MASK         0x07
#define AVRC_SUBID_IGNORE       0x07


/* subunit type */
#define AVRC_SUB_MONITOR    0x00    /* Monitor */
#define AVRC_SUB_AUDIO      0x01    /* Audio */
#define AVRC_SUB_PRINTER    0x02    /* Printer */
#define AVRC_SUB_DISC       0x03    /* Disc */
#define AVRC_SUB_TAPE       0x04    /* Tape recorder/player */
#define AVRC_SUB_TUNER      0x05    /* Tuner */
#define AVRC_SUB_CA         0x06    /* CA */
#define AVRC_SUB_CAMERA     0x07    /* Camera */
#define AVRC_SUB_PANEL      0x09    /* Panel */
#define AVRC_SUB_BB         0x0A    /* Bulletin Board */
#define AVRC_SUB_CAM_STOR   0x0B    /* Camera Storage */
#define AVRC_SUB_VENDOR     0x1C    /* Vendor unique */
#define AVRC_SUB_EXT        0x1E    /* Subunit type extended to next byte */
#define AVRC_SUB_UNIT       0x1F    /* Unit */




/* operands in passthrough commands */
#define AVC_VOLUME_UP               0x41
#define AVC_VOLUME_DOWN             0x42
#define AVC_MUTE                    0x43
#define AVC_PLAY                    0x44
#define AVC_STOP                    0x45
#define AVC_PAUSE                   0x46
#define AVC_RECORD                  0x47
#define AVC_REWIND                  0x48    //回倒
#define AVC_FAST_FORWARD            0x49    //快进
#define AVC_EJECT                   0x4a
#define AVC_FORWARD                 0x4b
#define AVC_BACKWARD                0x4c



/* define the key control operation */
#define KEY_VOLUMEUP                100
#define KEY_VOLUMEDOWN              101
#define KEY_PLAYCD                  102
#define KEY_STOPCD                  103
#define KEY_PAUSECD                 104
#define KEY_NEXTSONG                105
#define KEY_PREVIOUSSONG            106
#define KEY_REWIND                  107
#define KEY_FASTFORWARD             108

typedef unsigned int size_t;

typedef enum
{
    AVCTP_STATE_DISCONNECTED = 0,
    AVCTP_STATE_CONNECTING,
    AVCTP_STATE_CONNECTED,
    AVCTP_STATE_BROWSING_CONNECTING,
    AVCTP_STATE_BROWSING_CONNECTED
} avctp_state_t;

typedef enum
{
    AVCTP_CONN_NONE = 0,
    AVCTP_CONN_INITIATOR,   //主动发起者
    AVCTP_CONN_INITIATED,   //被动发起
    AVCTP_CONN_CONNECTED,   //已连上
} avctp_conn_state;

__packed
struct avctp_header
{
    uint8 ipid:1;
    uint8 cr:1;
    uint8 packet_type:2;
    uint8 transaction:4;
    uint16 pid;
};

__packed
struct avc_header
{
    uint8 code:4;
    uint8 _hdr0:4;
//  uint8 subunit_id:3;
//  uint8 subunit_type:5;
    uint8 subunit;
    uint8 opcode;
};




struct avctp_control_req
{
//  struct avctp_pending_req *p;
    uint8 code;
    uint8 subunit;
    uint8 op;
    uint8 *operands;
    uint16 operand_count;
//  avctp_rsp_cb func;
//  void *user_data;

    uint8 transaction;
};

struct avctp_browsing_req
{
    struct avctp_pending_req *p;
    uint8 *operands;
    uint16 operand_count;
//  avctp_browsing_rsp_cb func;
    void *user_data;
};



struct avctp_pending_req
{
    //struct avctp_channel *chan;
    uint8 transaction;
    uint32 timeout;
//  avctp_process_cb process;
    void *data;
    //GDestroyNotify destroy;
};

struct avctp
{
    //struct avctp_server *server;
    //struct btd_device *device;

    avctp_state_t state;

    int uinput;

    uint32 auth_id;
    unsigned int passthrough_id;
    uint32 unit_id;
    uint32 subunit_id;

    //struct avctp_channel *control;
    //struct avctp_channel *browsing;

    //uint8_t key_quirks[256];
    //struct key_pressed *key;
};


struct avctp_pdu_handler
{
    uint8 opcode;
//  avctp_control_pdu_cb cb;
    void *user_data;
    unsigned int id;
};

struct avctp_browsing_pdu_handler
{
//  avctp_browsing_pdu_cb cb;
    void *user_data;
    unsigned int id;
    //GDestroyNotify destroy;
};


__packed
struct avctp_pcb
{
    struct avctp_pcb *next; /* For the linked list */

    avctp_state_t state;

    int uinput;

    uint32 auth_id;
    unsigned int passthrough_id;
    uint32 unit_id;
    uint32 subunit_id;

    struct l2cap_pcb *l2cappcb; /* The L2CAP connection */
    struct l2cap_pcb *l2cappcb_data; /* The L2CAP connection */
    //  struct l2cap_pcb *l2cappcb_sign; /* The L2CAP connection */

    uint8 tid; /* Transaction ID */

    void *callback_arg;

};


__packed struct avctp_notify
{
    void(*close_notify)(void);
    void(*connected_notify)(void);
    void(*audio_play)(void);
    void(*audio_stop)(void);
    void(*audio_pause)(void);
    void(*audio_next)(void);
    void(*audio_previous)(void);
    void(*audio_volumeup)(void);
    void(*audio_volumedown)(void);
    void(*audio_volume_change)(uint8 volume);
    void(*audio_mute)(void);
    void(*audio_ffd)(void);
    void(*audio_ffw)(void);
};



/* Define two macros, AVCTP_REG and AVCTP_REMOVE that registers a AVCTP PCB
   with a PCB list or removes a PCB from a list, respectively. */
//
//#define AVCTP_REG(pcbs, npcb) do { \
//                            npcb->next = *pcbs; \
//                            *pcbs = npcb; \
//                            } while(0)
//#define AVCTP_REMOVE(pcbs, npcb) do { \
//                            if(*pcbs == npcb) { \
//                               *pcbs = (*pcbs)->next; \
//                            } else for(avctp_tmp_pcb = *pcbs; avctp_tmp_pcb != NULL; avctp_tmp_pcb = avctp_tmp_pcb->next) { \
//                               if(avctp_tmp_pcb->next != NULL && avctp_tmp_pcb->next == npcb) { \
//                                  avctp_tmp_pcb->next = npcb->next; \
//                                  break; \
//                               } \
//                            } \
//                            npcb->next = NULL; \
//                            } while(0)



/* Company ID is 24-bit integer */
#define AVRC_CO_ID_TO_BE_STREAM(p, u32) {*(p)++ = (UINT8)((u32) >> 16); *(p)++ = (UINT8)((u32) >> 8); *(p)++ = (UINT8)(u32); }
#define AVRC_BE_STREAM_TO_CO_ID(u32, p) {u32 = (((UINT32)(*((p) + 2))) + (((UINT32)(*((p) + 1))) << 8) + (((UINT32)(*(p))) << 16)); (p) += 3;}


struct avctp_pcb* avctp_connect();
void avctp_InitiateConn(void);
void avctp_StopTimer(void);

void avctp_init(void);
void avctp_deinit(void);

void avctp_reset_all();
err_t avctp_recv(void *arg, struct l2cap_pcb *pcb, struct pbuf *s, err_t err);
err_t avctp_l2cap_connected_ind(void *arg, struct l2cap_pcb *pcb, err_t err);
void avctp_lp_disconnected(void *arg, struct l2cap_pcb *l2cappcb, err_t err);

err_t
avctp_connect_req(struct avctp_pcb *pcb, struct bd_addr *bdaddr, uint16 psm, \
                  uint8 role_switch, err_t (* l2ca_connect_cfm)(void *arg, struct l2cap_pcb *lpcb,\
                          uint16 result, uint16 status));
avctp_connected(void *arg, struct l2cap_pcb *l2cappcb, uint16 result, uint16 status);
void avctp_connected_notify(void(*connected_notify)(void));
void avctp_close_notify(void(*close_notify)(void));

void avctp_audio_play_notify(void(*audio_play_notify)(void));
void avctp_audio_pause_notify(void(*audio_pause_notify)(void));

void avctp_audio_next_notify(void(*audio_next_notify)(void));
void avctp_audio_previous_notify(void(*audio_previous_notify)(void));
void avctp_audio_stop_notify(void(*audio_stop_notify)(void));
void avctp_audio_volumeup_notify(void(*audio_volumeup_notify)(void));
void avctp_audio_volumedown_notify(void(*audio_volumedown_notify)(void));
void avctp_audio_volume_change_notify(void(*audio_volume_change_notify)(uint8 volume));
void avctp_audio_mute_notify(void(*audio_mute_notify)(void));
extern struct avrcp_pcb *avrcp_pcbs;

#endif //__AVCTP_H__
