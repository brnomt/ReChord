
/*-----------------------------------------------------------------------------------*/
/* bt_ip_dt.h
*
* This is a control application that initialises a host controller and connects to a
* network as a DT through a DUN or LAP enabled device.
*/
/*-----------------------------------------------------------------------------------*/

#ifndef __BT_IP_DT_SOURCE_H__
#define __BT_IP_DT_SOURCE_H__

/* ---------- bt_init flag ---------- */

#define    ENABLE_A2DP  (1<< 0)
#define    ENABLE_HFP   (1<< 1)
#define    ENABLE_SPP   (1<< 2)
#define    ENABLE_AUTO_CONNECT (1<< 3)
#define    ENABLE_CMD_COMPLETE (1<< 4)

/* ---------- bt_start flag ---------- */

#define    BT_START_FLAG_DEFAULT (0)
#define    SEND_HCI_RESET   (1<< 0)


void bt_init(uint32 flag);

void bt_start(uint32 flag);

void bt_restart(void);

void bt_time_serve(void);
void bt_deinit(void);
void bt_set_local_name(char * name, int namelen); //需要在bt_start 之前调用
void bt_set_pin_code(char * pincode, int len);
int  bt_disconnect(struct bd_addr *bdaddr);


void bt_discoverable_enable(void);
void bt_discoverable_disable(void);
void bt_sleep_enable(void);
void bt_sleep_disable(void);
void bt_a2dp_connect(struct bd_addr *bdaddr, void(*connect_result_hook)(int result));
void bt_scan(void (* scan_result_hook)(struct hci_inq_res *ires, uint16 devCnt, int isComplete));
void bt_a2dp_disconnect(struct bd_addr *bdaddr, void(*disconnect_result_hook)(int result));
void bt_clean_scan_result(void);
void bt_set_init_complete_hook(void(*bt_init_complete_hook)(void));

void bt_scan_cancel(void);







#endif
