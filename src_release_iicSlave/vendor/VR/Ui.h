
#ifndef 	_UI_H_
#define	_UI_H_


#include "../../proj/tl_common.h"
#include "../../proj_lib/rf_drv.h"
#include "../../proj_lib/pm.h"
#include "../../proj_lib/blt_ll/blt_ll.h"
#include "../../proj/drivers/usbkeycode.h"
#include "../../proj/mcu_spec/adc_17H26.h"

///////////////////////////////////////////////////////////////////////////////////////
//						External Variable
///////////////////////////////////////////////////////////////////////////////////////
//extern ui_type_t led_ui_buffer_MS[4];
extern u8		 blt_suspend_mask;
extern u8 		 blt_terminate_pending;
extern u32 		 blt_adv_interval;
extern u32		 blt_conn_interval_next;
extern rf_packet_version_ind_t	pkt_version_ind;
extern u16		 blt_conn_latency_next;;
extern u8 blt_test_dongle_conn  ;   // master type in current connection: 1/ test dongle     0/other test dongle
extern int				blt_wakeup_src;
extern u8 		blt_smp_paring_req_recvd;
extern u8		blt_retry ;





extern  u8 reportKeyIn[];
extern  u8 reportConsumerControlIn[];
extern   u8 reportMouseIn[];
extern  u8 reportJoyStickIn[];

extern  u8 flag_has_new_event_tmp;
//extern  u8  connected_idle_time_count_tmp;

///////////////////////////////////////////////////////////////////////////////////////
//						External fuction
///////////////////////////////////////////////////////////////////////////////////////

extern void shutter_att_init ();
extern int blt_sleep_wakeup (int deepsleep, int wakeup_src, u32 wakeup_tick);
extern void blt_update_connPara_request (u16 min_interval, u16 max_interval, u16 latency, u16 timeout);
extern void hid_setting_flag(u16 en);

/************************************user define **************/

#define BIT0                            0x1
#define BIT1                            0x2
#define BIT2                            0x4
#define BIT3                            0x8
#define BIT4                            0x10
#define BIT5                            0x20
#define BIT6                            0x40
#define BIT7                            0x80
#define BIT8                            0x100
#define BIT9                            0x200
#define BIT10                           0x400
#define BIT11                           0x800
#define BIT12                           0x1000
#define BIT13                           0x2000
#define BIT14                           0x4000
#define BIT15                           0x8000
#define BIT16                           0x10000
#define BIT17                           0x20000
#define BIT18                           0x40000
#define BIT19                           0x80000
#define BIT20                           0x100000
#define BIT21                           0x200000
#define BIT22                           0x400000
#define BIT23                           0x800000
#define BIT24                           0x1000000
#define BIT25                           0x2000000
#define BIT26                           0x4000000
#define BIT27                           0x8000000
#define BIT28                           0x10000000
#define BIT29                           0x20000000
#define BIT30                           0x40000000
#define BIT31                           0x80000000


extern u8	 blt_suspend_mask;
extern u32 tick_app_wakeup;
#define MAX_WAKEUP_INTERVAL 		20	///20ms

#define LENZE_VER 	0x11
#define LOCK_ID		0x10
#define DEVTYPE		0x06

void iic_wakeup_enable();

extern void deal_iic_data_fun(u8 *temp_write_buf);
extern void data_to_gps(u8 *cmd);
extern u8 host_rb[];
extern u32 wake_up_time_tick;
extern u8 wake_up_flag;

typedef enum
{
	POWER_ON_DEVICE_STATUS,
	DEEP_SLEEP_DEVICE_STATUS,
	HIGH_ADV_DEVICE_STATUS,
	LOW_ADV_DEVICE_STATUS,
	CONNECTED_DEVICE_STATUS,
	AFTER_CONNECTED_DEVICE_STATUS,
	KEY_PRESS_DEVICE_STATUS,
}DEVICE_STATUS;

#define VR_MODE_REG 0X19
#define POWER_TYPE_REG 0X1A
#define POWER_SWTICH_REG  0X1B

#endif

