
#ifndef __LWBT_AVDTP_H__
#define __LWBT_AVDTP_H__
#include "SysInclude.h"

#include "l2cap.h"
#define AVDTP_PSM 25

#define MAX_SEID 0x3E
#define A2DP_HEADER_LEN 2
#define A2DP_DATA_LEN 1
#define A2DP_CAPDATA_LEN 11

#define AVDTP_DISCOVER              0x01
#define AVDTP_GET_CAPABILITIES  0x02
#define AVDTP_SET_CONFIGURATION 0x03
#define AVDTP_GET_CONFIGURATION 0x04
#define AVDTP_RECONFIGURE           0x05
#define AVDTP_OPEN                  0x06
#define AVDTP_START             0x07
#define AVDTP_CLOSE             0x08
#define AVDTP_SUSPEND               0x09
#define AVDTP_ABORT             0x0A
#define AVDTP_SECURITY_CONTROL          0x0B
#define AVDTP_GET_ALL_CAPABILITIES      0x0C
#define AVDTP_DELAY_REPORT          0x0D

#define AVDTP_PKT_TYPE_SINGLE           0x00
#define AVDTP_PKT_TYPE_START            0x01
#define AVDTP_PKT_TYPE_CONTINUE         0x02
#define AVDTP_PKT_TYPE_END          0x03

#define AVDTP_MSG_TYPE_COMMAND          0x00
#define AVDTP_MSG_TYPE_GEN_REJECT       0x01
#define AVDTP_MSG_TYPE_ACCEPT           0x02
#define AVDTP_MSG_TYPE_REJECT           0x03

#define REQ_TIMEOUT 6
#define ABORT_TIMEOUT 2
#define DISCONNECT_TIMEOUT 1
#define STREAM_TIMEOUT 20
#define START_TIMEOUT 1
/* SEP capability categories */
#define AVDTP_MEDIA_TRANSPORT           0x01
#define AVDTP_REPORTING             0x02
#define AVDTP_RECOVERY              0x03
#define AVDTP_CONTENT_PROTECTION        0x04
#define AVDTP_HEADER_COMPRESSION        0x05
#define AVDTP_MULTIPLEXING          0x06
#define AVDTP_MEDIA_CODEC           0x07
#define AVDTP_DELAY_REPORTING           0x08
#define AVDTP_ERRNO             0xff

/* SBC CODEC definitions */
#define AVDTP_AUDIO_CODEC_SBC           0x00
#define AVDTP_AUDIO_CODEC_MP3               0x01
#define AVDTP_AUDIO_CODEC_AAC               0x02

#define SBC_SAMPLING_FREQ_16000     (1 << 7)
#define SBC_SAMPLING_FREQ_32000     (1 << 6)
#define SBC_SAMPLING_FREQ_44100     (1 << 5)
#define SBC_SAMPLING_FREQ_48000     (1 << 4)

#define SBC_CHANNEL_MODE_MONO               (1 << 3)
#define SBC_CHANNEL_MODE_DUAL_CHANNEL   (1 << 2)
#define SBC_CHANNEL_MODE_STEREO         (1 << 1)
#define SBC_CHANNEL_MODE_JOINT_STEREO   (1 << 0)

#define SBC_BLOCK_LENGTH_4      (1 << 7)
#define SBC_BLOCK_LENGTH_8      (1 << 6)
#define SBC_BLOCK_LENGTH_12     (1 << 5)
#define SBC_BLOCK_LENGTH_16     (1 << 4)

#define SBC_SUBBANDS_4              (1 << 3)
#define SBC_SUBBANDS_8              (1 << 2)

#define SBC_ALLOCATION_SNR      (1 << 1)
#define SBC_ALLOCATION_LOUDNESS (1 << 0)

#define MAX_BITPOOL 0X35
#define MIN_BITPOOL 2
/* AVDTP error definitions */
#define AVDTP_BAD_HEADER_FORMAT         0x01
#define AVDTP_BAD_LENGTH                    0x11
#define AVDTP_BAD_ACP_SEID              0x12
#define AVDTP_SEP_IN_USE                    0x13
#define AVDTP_SEP_NOT_IN_USE                0x14
#define AVDTP_BAD_SERV_CATEGORY         0x17
#define AVDTP_BAD_PAYLOAD_FORMAT            0x18
#define AVDTP_NOT_SUPPORTED_COMMAND     0x19
#define AVDTP_INVALID_CAPABILITIES      0x1A
#define AVDTP_BAD_RECOVERY_TYPE         0x22
#define AVDTP_BAD_MEDIA_TRANSPORT_FORMAT    0x23
#define AVDTP_BAD_RECOVERY_FORMAT       0x25
#define AVDTP_BAD_ROHC_FORMAT               0x26
#define AVDTP_BAD_CP_FORMAT             0x27
#define AVDTP_BAD_MULTIPLEXING_FORMAT       0x28
#define AVDTP_UNSUPPORTED_CONFIGURATION     0x29
#define AVDTP_BAD_STATE                 0x31

/* SEP types definitions */
#define AVDTP_SEP_TYPE_SOURCE           0x00
#define AVDTP_SEP_TYPE_SINK         0x01

/* Media types definitions */
#define AVDTP_MEDIA_TYPE_AUDIO          0x00
#define AVDTP_MEDIA_TYPE_VIDEO          0x01
#define AVDTP_MEDIA_TYPE_MULTIMEDIA     0x02

#define AVDTP_SEPID_INUSE               0x01
#define AVDTP_SEPID_NOT_INUSE           0x00
#define AVDTP_SEP_ID        0x01
typedef enum
{
    AVDTP_STATE_IDLE,
    AVDTP_STATE_CONFIGURED,
    AVDTP_STATE_OPEN,
    AVDTP_STATE_SUSPEND,
    AVDTP_STATE_STREAMING,
    AVDTP_STATE_CLOSING,
    AVDTP_STATE_ABORTING,
    AVDTP_STATE_WAIT_CLOSE,
    AVDTP_STATE_WAIT_ABORT,
    AVDTP_STATE_WAIT_START_COMPLETE,
    AVDTP_STATE_WAIT_SUSPEND_COMPLETE,
    AVDTP_STATE_GOTO_START_QUEUE,
    AVDTP_STATE_GOTO_SUSPEND_QUEUE,
} avdtp_state_t;
__packed struct get_cap_cmd_resp
{
    uint8 ServiceCategory1;
    uint8 LOSC1;
    uint8 MediaType;
    uint8 MediaCodecType;
    uint8 SF_CM;
    uint8 BL_BS_AM;
    uint8 MinimumBitpoolValue;
    uint8 MaximumBitpoolValue ;
};
__packed struct discover_cmd_resp
{
    uint8 RFAINUSESEID;
    uint8 RFATSEPMEDAI;
};
__packed struct set_configuration_cmd
{
    uint8 MPT;
    uint8 SIRFA;
    uint8 Acp_SEID;
    uint8 Int_SEID;
    uint8 ServiceCategory;
    uint8 LOSC;
    uint8 ServiceCategory1;
    uint8 LOSC1;
    uint8 MediaType;
    uint8 MediaCodecType;
    uint8 SF_CM;
    uint8 BL_BS_AM;
    uint8 MinimumBitpoolValue;
    uint8 MaximumBitpoolValue ;
};
__packed struct Suspend_cmd
{
    uint8 MPT;
    uint8 SIRFA;
    uint8 Acp_SEID;
};
__packed struct start_cmd
{
    uint8 MPT;
    uint8 SIRFA;
    uint8 Acp_SEID;
};
__packed struct open_cmd
{
    uint8 MPT;
    uint8 SIRFA;
    uint8 Acp_SEID;
};
__packed struct get_cap_cmd
{
    uint8 MPT;
    uint8 SIRFA;
    uint8 RFAINUSESEID;
};
__packed struct discover_cmd
{
    uint8 MPT;
    uint8 SIRFA;
};

__packed struct Abort_resp
{
    uint8 MPT;
    uint8 SIRFA;
};
__packed struct Suspend_resp
{
    uint8 MPT;
    uint8 SIRFA;
};
__packed struct close_resp
{
    uint8 MPT;
    uint8 SIRFA;
};
__packed struct start_resp
{
    uint8 MPT;
    uint8 SIRFA;
};
__packed struct open_resp
{
    uint8 MPT;
    uint8 SIRFA;
};
__packed struct set_configuration_resp
{
    uint8 MPT;
    uint8 SIRFA;
};
__packed struct discover_resp
{
    uint8 MPT;
    uint8 SIRFA;
    uint8 RFAINUSESEID;
    uint8 RFATSEPMEDAI;
};
__packed struct get_cap_resp
{
    uint8 MPT;
    uint8 SIRFA;
    uint8 ServiceCategory;
    uint8 LOSC;
    uint8 ServiceCategory1;
    uint8 LOSC1;
    uint8 MediaType;
    uint8 MediaCodecType;
    uint8 SF_CM;
    uint8 BL_BS_AM;
    uint8 MinimumBitpoolValue;
    uint8 MaximumBitpoolValue ;
#ifdef A2DP_ACC
    uint8 AAC_ServiceCategory1;
    uint8 AAC_LOSC1;
    uint8 AAC_MediaType;
    uint8 AAC_MediaCodecType;
    uint8 AAC_SF_CM;
    uint8 AAC_BL_BS_AM;
    uint8 AAC_MinimumBitpoolValue;
    uint8 AAC_MaximumBitpoolValue ;
    uint8 AAC_MaximumBitpoolValue1 ;
    uint8 AAC_MaximumBitpoolValue2 ;
#endif
    uint8 ServiceCategory2;
    uint8 LOSC2;
    uint8 CP_TYPE_LSB;
    uint8 CP_TYPE_MSB;
};

__packed struct Remote_SBC_CODEC_info
{
    uint8 Int_SEID;
    uint8 MediaType;
    uint8 MediaCodecType;
    uint8 Sampling_Frequency;
    uint8 Channel_Mode;
    uint8 Block_Length;
    uint8 Subbands;
    uint8 Allocation_Method ;
    uint8 MinimumBitpoolValue;
    uint8 MaximumBitpoolValue ;
};
__packed struct Local_SBC_CODEC_info
{
    uint8 Sampling_Frequency;
    uint8 Channel_Mode;
    uint8 Block_Length;
    uint8 Subbands;
    uint8 Allocation_Method ;
    uint8 MinimumBitpoolValue;
    uint8 MaximumBitpoolValue ;
    bool delay_reporting;
};
__packed
struct avdtp_local_sep
{
    uint8 InUse;
    uint8 SEID ;
    uint8 TSEP;
    uint8 Media_Type;
    bool delay_reporting;
};
__packed struct avdtp_single_header
{
    uint8 message_type:2;
    uint8 packet_type:2;
    uint8 transaction:4;
    uint8 signal_id:6;
    uint8 rfa0:2;
};
__packed struct avdtp_Command_Data
{
    uint8 rfa0:2;
    uint8 Acp_SEID:6;
};
__packed struct avdtp_Command_Cap_Data
{
    uint8 Int_SEID;
    uint8 ServiceCategory;
    uint8 LOSC;
    uint8 ServiceCategory1;
    uint8 LOSC1;
    uint8 MediaType;
    uint8 MediaCodecType;
    uint8 SF_CM;
    uint8 BL_BS_AM;
    uint8 MinimumBitpoolValue;
    uint8 MaximumBitpoolValue ;

};
__packed struct a2dp_pcb
{
    struct a2dp_pcb *next; /* For the linked list */
    struct avdtp_local_sep local_sep;
    struct Local_SBC_CODEC_info Localcodec;
    struct Remote_SBC_CODEC_info Remotecodec;
    struct discover_cmd_resp RemoteSEID[8];
    struct l2cap_pcb *l2cappcb; /* The L2CAP connection */
//  struct l2cap_pcb *l2cappcb_sign; /* The L2CAP connection */
    uint8  RemoteSEIDCnt;
    uint16 tid; /* Transaction ID */

    void *callback_arg;
};



/* Functions for interfacing with SDP: */
void avdtp_init(void);
void avdtp_deinit(void);


void avdtp_reset_all(void);

void avdtp_sendMessage(struct l2cap_pcb *l2_pcb,uint16 state,void (* avctp_startTimer)(struct l2cap_pcb *l2_pcb ));
void avctp_startTimer(struct l2cap_pcb *pcb);
/* Lower layer API */
//void avdtp_lp_disconnected(struct l2cap_pcb *l2cappcb);
err_t avdtp_recv(void *arg, struct l2cap_pcb *pcb, struct pbuf *p, err_t err);
err_t avdtp_Cmd_recv(void *arg, struct l2cap_pcb *pcb, struct pbuf *p, err_t err);

err_t avdtp_data_recv(void *arg, struct l2cap_pcb *pcb, struct pbuf *s, err_t err);
void avdtp_media_recv(void(*media_data_recv)(char * buf, int len));
int avdtp_connect(struct bd_addr *bdaddr, void(*connect_result)(int result));

uint16 get_avdtp_state(void);
uint8 avdtp_get_codec_type(void);
uint8 avdtp_get_sbc_bitpool(void);

void avdtp_start(void);
void avdtp_suspend(void);
void avdtp_tmr(void);




__packed struct avdtp_notify
{
    void(*streaming_notify)(void);
    void(*suspend_notify)(void);
    void(*close_notify)(void);
    void(*connected_notify)(void);
    void(*abort_notify)(void);
    void(*connected_result)(int result);
    void(*disconnected_result)(int result);
};


void avdtp_streaming_notify(void(*streaming_notify)(void));
void avdtp_suspend_notify(void(*suspend_notify)(void));
void avdtp_close_notify(void(*close_notify)(void));
void avdtp_abort_notify(void(*abort_notify)(void));
void avdtp_connected_notify(void(*connected_notify)(void));



/* Type is constructed by ORing a type and size bitmask.
   Size is ignored for String, URL and sequence types.
   For String, URL types, the given value must be a char*,
   from which the size is calculated.
   For a sequence type the size is calculated directly from the
   list of elements added into the sequence.
   For integer types greater than 32 bit, and for 128 bit UUID
   types, the value is given as a byte array.
*/



#endif /* __LWBT_SDP_H__ */
