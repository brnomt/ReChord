
#ifndef __AVRCP_H__
#define __AVRCP_H__

/* player attributes */
#define AVRCP_ATTRIBUTE_ILEGAL      0x00
#define AVRCP_ATTRIBUTE_EQUALIZER   0x01
#define AVRCP_ATTRIBUTE_REPEAT_MODE 0x02
#define AVRCP_ATTRIBUTE_SHUFFLE     0x03
#define AVRCP_ATTRIBUTE_SCAN        0x04

/* equalizer values */
#define AVRCP_EQUALIZER_OFF         0x01
#define AVRCP_EQUALIZER_ON          0x02

/* repeat mode values */
#define AVRCP_REPEAT_MODE_OFF       0x01
#define AVRCP_REPEAT_MODE_SINGLE    0x02
#define AVRCP_REPEAT_MODE_ALL       0x03
#define AVRCP_REPEAT_MODE_GROUP     0x04

/* shuffle values */
#define AVRCP_SHUFFLE_OFF           0x01
#define AVRCP_SHUFFLE_ALL           0x02
#define AVRCP_SHUFFLE_GROUP         0x03

/* scan values */
#define AVRCP_SCAN_OFF              0x01
#define AVRCP_SCAN_ALL              0x02
#define AVRCP_SCAN_GROUP            0x03

/* media attributes */
#define AVRCP_MEDIA_ATTRIBUTE_ILLEGAL   0x00
#define AVRCP_MEDIA_ATTRIBUTE_TITLE     0x01
#define AVRCP_MEDIA_ATTRIBUTE_ARTIST    0x02
#define AVRCP_MEDIA_ATTRIBUTE_ALBUM     0x03
#define AVRCP_MEDIA_ATTRIBUTE_TRACK     0x04
#define AVRCP_MEDIA_ATTRIBUTE_N_TRACKS  0x05
#define AVRCP_MEDIA_ATTRIBUTE_GENRE     0x06
#define AVRCP_MEDIA_ATTRIBUTE_DURATION  0x07
#define AVRCP_MEDIA_ATTRIBUTE_LAST      AVRCP_MEDIA_ATTRIBUTE_DURATION

/* play status */
#define AVRCP_PLAY_STATUS_STOPPED     0x00
#define AVRCP_PLAY_STATUS_PLAYING     0x01
#define AVRCP_PLAY_STATUS_PAUSED      0x02
#define AVRCP_PLAY_STATUS_FWD_SEEK        0x03
#define AVRCP_PLAY_STATUS_REV_SEEK        0x04
#define AVRCP_PLAY_STATUS_ERROR           0xFF

/* Notification events */
#define AVRCP_EVENT_STATUS_CHANGED              0x01
#define AVRCP_EVENT_TRACK_CHANGED               0x02
#define AVRCP_EVENT_TRACK_REACHED_END           0x03
#define AVRCP_EVENT_TRACK_REACHED_START         0x04
#define AVRCP_EVENT_POSITION_CHANGED            0x05
#define AVRCP_EVENT_SETTINGS_CHANGED            0x08
#define AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED    0x0b
#define AVRCP_EVENT_VOLUME_CHANGED              0x0d
#define AVRCP_EVENT_LAST                        AVRCP_EVENT_VOLUME_CHANGED

/* AV/C VENDOR DEPENDENT commands */
#define AVRCP_VENDOR_DEP_GET_CAPABILITIES        0x10
#define AVRCP_VENDOR_DEP_GET_ELEMENTATTRIBUTES   0x20
#define AVRCP_VENDOR_DEP_GET_PLAYSTATUS          0x30
#define AVRCP_VENDOR_DEP_REG_NOTIFICATION        0x31
#define AVRCP_VENDOR_DEP_SET_ABSOLUTEVOLUME      0x50

/* CapabilityID */
#define AVRCP_CAPABILITY_COMPANYID               0x02
#define AVRCP_CAPABILITY_EVENTID_SUPPORTED       0x03

__packed
struct avrcp_header
{
    uint8 company_id[3];
    uint8 pdu_id;
    uint8 packet_type:2;
    uint8 rsvd:6;
    uint16 params_len;
    uint8 params[1];  //params[0]
};
#define AVRCP_HEADER_LENGTH 7



__packed
struct avrcp_pcb
{
    struct avrcp_pcb *next; /* For the linked list */
    struct avctp_pcb *session;

    //  struct seid_info *seps;
    struct l2cap_pcb *l2cappcb; /* The L2CAP connection */
    struct l2cap_pcb *l2cappcb_data; /* The L2CAP connection */
    //  struct l2cap_pcb *l2cappcb_sign; /* The L2CAP connection */

    void *callback_arg;
    uint16 registered_events;

};

int avctp_vendordep_rsp(struct avctp_pcb *session,uint8 code,uint8 *operands, struct avc_header *avc_hdr);
int handle_vendordep_req(struct avctp_pcb *session,uint8 code,uint8 *operands);
void avrcp_init();
void avrcp_deinit();
int ct_pause(void);
int ct_stop(void);
int ct_next(void);
int ct_previous(void);
int ct_fast_forward(void);
int ct_rewind(void);

extern struct avrcp_pcb *avrcp_pcbs;
#endif  //__AVRCP_H__