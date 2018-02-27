#include "ui.h"
#include "../../proj_lib/ble_l2cap/ble_ll_ota.h"
#include "../../proj/drivers/flash.h"
#include "../../proj/drivers/i2c.h"
#include "uart.h"

// Enable TAIL_BOOT_CODE_PRESET to embed bootcode to bin file, 
// such make bin file size to be exact 16K
#define  TAIL_BOOT_CODE_PRESET		0
#if(TAIL_BOOT_CODE_PRESET)
volatile static u8 test_boot_code = 0;
_attribute_custom_code_  volatile u8 const boot_code[] = {
0x02,0x00,0x00,0xC5,0xFF,0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0x0F,0xFC,
0xBF,0x98,0x02,0x06,0xBF,0x01,0x03,0x06,0x3F,0xF8,0x00,0x00,0xFF,0xFF,0xFF,0xFF
};
#endif

////////////////////////////////cfg address  /////////////
#if (1)
#define		SUSPEND_STATE			SUSPEND_CONN | SUSPEND_ADV
#else
#define		SUSPEND_STATE			0
#endif

#if(OTA_ENABLE)
#define TEST_OTA_1 0
#endif

#define  TEST_SUSPEND_TIME_ENABLE 0

////////////////////////////////cfg address  /////////////
/*********************  config start*********************/

__attribute__ ((aligned (128))) u8 host_rb[128] = {0};//buffer used to receive packets direct from I2C Master, Top 64 data is for master write,After 64 data is for master read

#define HIGH_ADV_INTERVAL 	10
#define LOW_ADV_INTERVAL	500//700//50///600	//700	///50
#define CONNECTED_IDLE_TIME 5

extern void deal_cmd_from_app(u8 *cmd, u8 len);

#define ADV_LED_PORT  			GPIO_GP7

#define PRODUCT_ID1				0x01
#define PRODUCT_ID2				0x01
#define ADDRESS_OFFSET			14

void blt_disable_latency(){
	//disable current latency
	extern u8 blt_retry ;
	blt_retry = 1;
}


u8  tbl_mac [] = {0x24, 0x27, 0x28, 0x29, 0x30, 0x31};
u8	tbl_adv [] =
{
	0x00, 28,
	0, 0, 0, 0, 0, 0,		//reserve 6byte for mac address
	0x02, 0x01, 0x06,
	0x0e, 0xff, 0x01, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x06, 0x64, 0x01, LOCK_ID, LENZE_VER,
	0x03, 0x02, 0xe7, 0xfe,		// incomplete list of service class UUIDs
	0, 0, 0,			//reserve 3 bytes for CRC
};

u8	tbl_rsp [30] =
{
	0x00, 16,									//	type len
	0xef, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5,		//reserve 6byte for mac address
	0x09, 0x09, 'S', 'l', 'a', 'v', 'e', 'I', 'I', 'C', //local name
	0, 0, 0								//reserve 3 bytes for CRC
};

static inline void user_define_by_mac_address()
{
	u8 temp_mac[6]={0x00};
	u32 *pmac = (u32 *) CFG_ADR_MAC_ADDR;
	if (*pmac != 0xffffffff) {
		memcpy (temp_mac, pmac, 6);
	}
	else {
		memcpy (temp_mac, tbl_mac, 6);
	}
	tbl_adv[ADDRESS_OFFSET] = temp_mac[5];
	tbl_adv[ADDRESS_OFFSET+1] = temp_mac[4];
	tbl_adv[ADDRESS_OFFSET+2] = temp_mac[3];
	tbl_adv[ADDRESS_OFFSET+3] = temp_mac[2];
	tbl_adv[ADDRESS_OFFSET+4] = temp_mac[1];
	tbl_adv[ADDRESS_OFFSET+5] = temp_mac[0];
}

///////////////////////////////////// proc_ui ///////////////////////////////////////////

////////////////////////////////////// proc send data ///////////////////////////////////

extern u16 blt_conn_inst;
extern u8 os_check;
/********************************************* user define****************/
u8 data_test[] = {0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0xAA,0xAA,0xAA,0xAA,};
u8 device_status_tmp=0;
u32 tick_app_wakeup = 0;
u32 tick_connected_timer_tmp=0;

static inline void slave_iic_init()
{
	////IIC slave setting initialization
	gpio_set_func (GPIO_GP22, !AS_GPIO);		//SCL
	gpio_set_func (GPIO_GP23, !AS_GPIO);		//SDA
	gpio_set_input_en(GPIO_GP22,1);
	gpio_set_output_en(GPIO_GP23,1);
	gpio_set_input_en(GPIO_GP23,1);
	analog_write (0x09, 0x28);		//GP22 GP23 up 10K
	REG_ADDR16(0x3e) = (u16)(u32)host_rb;//buffer used to receive packets direct from I2C Master, Top 64 data is for master write,After 64 data is for master read
	REG_ADDR8(0x03) |= BIT(2);					//I2C mapping mode enable
	REG_ADDR8(0x22) = BIT(2);
////IIC slave setting

	/*set gpio18 wakeup deepsleep*/
	blt_set_wakeup_source(PM_WAKEUP_CORE);
	write_reg8 ( 0x597, 0x04);	//
	write_reg8 ( 0x594, 0x00);	// gpio_set_interrupt(GPIO_GP18, 0);	// rising edge

	analog_write (0x16, 0x02);//set enable
	analog_write (0x14, analog_read (0x14) & 0xef);// set polarity
	gpio_set_func(GPIO_GP18, AS_GPIO);
	gpio_set_output_en(GPIO_GP18,0);
	gpio_set_input_en(GPIO_GP18,1);
	analog_write (0x08, 0x0C);	//G18ÏÂÀ­100K

	my_uart_init();
	sleep_us(10000);
}

static inline void user_gpio_config_init()
{
	slave_iic_init();
}

static inline void gatt_init_tmp()
{
	u8 temp_mac[6];
	u32 *pmac = (u32 *) CFG_ADR_MAC_ADDR;
	if (*pmac != 0xffffffff) {
		memcpy (temp_mac, pmac, 6);
	}
	else {
		memcpy (temp_mac, tbl_mac, 6);
	}
	tbl_adv[15] = temp_mac[5];
	tbl_adv[16] = temp_mac[4];
	tbl_adv[17] = temp_mac[3];
	tbl_adv[18] = temp_mac[2];
	tbl_adv[19] = temp_mac[1];
	tbl_adv[20] = temp_mac[0];

	blt_init (tbl_mac, tbl_adv, tbl_rsp);			//get mac addr

	shutter_att_init ();

	blt_suspend_mask = SUSPEND_STATE;
	//////////////////// enable different event call back functions /////////
	 /*Decide whether event cb should be called by SDK,should be defined in user_init*/

	//////////////////// enable attribute table write/read call back functions /////////
	/*Use l2cap_att_read_write_cb_flag to decide whether sdk should call att_read_cb/att_write_cb
	 * when remote device process read/write operation ,should be defined in user_init*/
//	extern u8 l2cap_att_read_write_cb_flag ;
//	l2cap_att_read_write_cb_flag = 0;//ATT_READ_CB_ENABLE  | ATT_WRITE_CB_ENABLE | ATT_RSP_CB_ENABLE
}

static inline void rf_set_power_level_tmp()
{
	/*
	0x25, 0x7c, 0x67, 0x67,		// 7 dBm
	0x0a, 0x7c, 0x67, 0x67,		// 5 dBm
	0x06, 0x74, 0x43, 0x61,		// -0.6
	0x06, 0x64, 0xc2, 0x61,		// -4.3
	0x06, 0x64, 0xc1, 0x61,		// -9.5
	0x05, 0x7c, 0x67, 0x67,		// -13.6
	0x03, 0x7c, 0x67, 0x67,		// -18.8
	0x02, 0x7c, 0x67, 0x67,		// -23.3
	0x01, 0x7c, 0x67, 0x67,		// -27.5
	0x00, 0x7c, 0x67, 0x67,		// -30
	0x00, 0x64, 0x43, 0x61,		// -37
	0x00, 0x64, 0xcb, 0x61,		// -max	power down PA & PAD
*/	
	analog_write (0xa2, 0x25);
	analog_write (0x04, 0x7c);
	analog_write (0xa7, 0x67);
	analog_write (0x8d, 0x67);
}

static inline void adv_status_poll()
{
	blt_set_adv_interval(LOW_ADV_INTERVAL*1000);
}

void OTA_init_tmp()
{
#if(OTA_ENABLE)	
	u8 buf[4] = {0};
	flash_read_page(OTA_FLASH_ADDR_START, 4, buf);
	u32 tmp = buf[0] | (buf[1]<<8) |(buf[2]<<16) | (buf[3]<<24);
	if(tmp != ONES_32){
		flash_erase_block(OTA_FLASH_ADDR_START);
		//ota_reboot_tmp();
		sleep_us(1*1000*1000);	// because flash_erase_block may exit premature, becuase of flash_wait_done
	}
#endif
}
extern u8 run_status;
void user_init()
{
	run_status = 0x01;
#if(TAIL_BOOT_CODE_PRESET)
	if(test_boot_code){
		int i;
		att_response_cb(boot_code + i);
	}
#endif
	
#if(DEBUG_FROM_FLASH)
	OTA_init_tmp();
	set_tp_flash();
	set_freq_offset_flash(0x56);
	set_mac_flash(tbl_mac);
	run_status = 0x02;
//	static u32 chip_id;
//	chip_id = read_chip_ID_flash();
#else
	set_tp_OTP();
	set_freq_offset_OTP(0x56);
	set_mac_OTP(tbl_mac);
//	static u32 chip_id;
//	chip_id = read_chip_ID_OTP();
	gpio_set_func(GPIO_SWS, AS_GPIO);
	gpio_set_output_en(GPIO_SWS,0);
	gpio_set_input_en(GPIO_SWS,0);
#endif
	
	gpio_set_func(GPIO_GP18, AS_GPIO);
	analog_write(0x02, 0x01);		// reset PLL,  must !!!!,  or sometimes will dead
	sleep_us(10);
	analog_write(0x02, 0x05);		//restore PLL,  must !!!!,  or sometimes will dead
	rf_set_power_level_tmp();
	user_gpio_config_init();
	gatt_init_tmp();
	adv_status_poll();
	run_status = 0x03;
}


//ATT_OP_READ_REQ:
//rf_packet_att_read_t

int att_read_cb(void*p ){
//	rf_packet_att_read_t *req= (rf_packet_att_read_t*)p;
	return ATT_NO_HANDLED;
}


/*Master write my_Attributes,  sdk firstly call att_write_cb. So user shall decide here 
 * to allow or disallow the write operation and return different value 
 * User should not modify the name of function and shall not delete it even if user don't
 * need this function
 *
 * Return: ATT_NO_HANDLED means user has not processed the write operation,
 *              SDK should also write the value automatically
 *             ATT_HANDLED means user has processed the write operation,
 *             SDK should not re-write the value*/
int att_write_cb(void*p)	//callback when app send data
{
	rf_packet_att_write_t *src = (rf_packet_att_write_t*)p;
	u8 *value;
	value=&src->value;
#if(OTA_ENABLE)
	 if(src->handle == OTA_CMD_OUT_DP_H){
		u8 result=otaWrite(src);
		
		return ATT_HANDLED;
	}
#endif
	if(3 == src->handle) {
		deal_cmd_from_app(&src->value,(src->rf_len - 7));	///deal the app data
		return ATT_HANDLED;
	}
	return ATT_NO_HANDLED;
}
/*
 other opcode
 * */
void att_response_cb( u8 *p){
//	rf_packet_l2cap_req_t * req = (rf_packet_l2cap_req_t *)p;
	return;
}


 /*Following functions are used to define different events, details please refers to blt_ll.h, user
  * can use flag ll_event_cb_flag to decide whether call this event*/
#if (LOW_COST_EVENT_CB_MODE)
  
  /*Decide whether event cb should be called by SDK,should be defined in user_init*/
  //ll_event_cb_flag = BLT_EV_FLAG_CONNECT  | BLT_EV_FLAG_TERMINATE | BLT_EV_FLAG_BOND_START;

/*task_connection_established
 * This event is returned once module receives a connection request packet
 * and establishs a connection successfully
 * ex: start send connection parameter update request after a time from 
 * the connection event; notify application connection establishment state*/
 
void task_connection_established(rf_packet_connect_t* p){	//callback when connected
	//adv_start_tick = last_update_paramter_time = clock_time();// in bond state better
	//adv_time_cnt = 0;
	tick_connected_timer_tmp=clock_time();
	device_status_tmp=CONNECTED_DEVICE_STATUS;
}

/*task_connection_terminated
 * This event is returned once connection is terminated
 * ex:notify application connection terminated; reset connection para*/
//volatile u8 aa_cnt = 0;
void task_connection_terminated(rf_packet_connect_t* p){	//callback when disconnected
	//app_wakeup_tick = adv_start_tick = clock_time();
	//adv_time_cnt = 0;
	device_status_tmp=POWER_ON_DEVICE_STATUS;
}

/*task_bond_finished
 * This event is returned once encryption process is finished
 * ex: HID key can be used once connection is encrypted*/

void task_bond_finished(rf_packet_connect_t* p){
}
#endif


/**********************ui start ************************************/
//"KEY_BIT_VR" must be consistent with "ordinary_key_matrix"
void vr_autoSetMode(){
//#if (AUTO_CKECK_OS_TYPE_FLAG)
//	if(3 == os_check){
//		vr_mode_tmp = IOS_ICADE_MODE;
//	}else if(2 == os_check){
//		vr_mode_tmp = ANDROID_GAME_MODE;
//	}
//#endif
}
//u32 start_ota_tick;
//u8 start_ota_cnt;
static inline void connection_status_poll()	//Update the connection parameters
{
	if((device_status_tmp==CONNECTED_DEVICE_STATUS)&&( blt_fifo_num()<3)&&clock_time_exceed(tick_connected_timer_tmp,2*1000*1000))
	{
		device_status_tmp=AFTER_CONNECTED_DEVICE_STATUS;
		blt_update_connPara_request(16,32,0,400);
//		blt_update_connPara_request(60,80,0,400);
	}
}

static inline void ble_status_poll()
{
	static u8 is_connected = 0;
	static u8 ble_status_data[] = { 0x03, 0x01, 0x01, 0xff};//send connected status to master iic
	if (blt_state == BLT_LINK_STATE_ADV)
	{
		blt_send_adv (BLT_ENABLE_ADV_ALL);	//setting advertising channel
		blt_adv_interval = ((rand()% 10) + LOW_ADV_INTERVAL-5)*CLOCK_SYS_CLOCK_1MS;//setting advertising interval
		if(is_connected == 1){
			is_connected = 0;
			ble_status_data[2] = 0x00;
			deal_cmd_from_app(ble_status_data,(sizeof(ble_status_data)));
		}
	}
	else {
		if(is_connected == 0) {
			ble_status_data[2] = 0x01;
			deal_cmd_from_app(ble_status_data,(sizeof(ble_status_data)));
			is_connected = 1;
		}
		connection_status_poll();//setting connect interval
	}
}
void test_power()
{
	sleep_us(500000);
	sleep_us(100000);
	blt_sleep_wakeup(0,PM_WAKEUP_PAD,0);
}

#define 	TEST	1		///test for iic

void main_loop()
{
	run_status = 0x04;

#if TEST
	static u32 cnt;
	static u32 second_tick;
	run_status = 0x05;
	if(clock_time_exceed(second_tick ,500*1000)) {
		second_tick = clock_time();

		run_status = 0x06;
#if 1
//			extern u8 Read_value[20];
//			i2c_sim_burst_read(0x5d, 0x00,Read_value, 20);
//			blt_push_notify_data(0x07, Read_value, 20);
//			memset(host_rb+64, 0x00, 20);

//			if(gpio_read(GPIO_GP18)) {
//
//				gpio_write(GPIO_GP31,1);
//				cnt++;
//				u8 iic_test_buf[20] = {18, 0x04, 0x01, 'm', 'y', 'm', 'y', 'm', 'y', 'm', 'y','m', 'y', 'm', 'y','m', 'y', 'm', 'y',0x55};
//				i2c_sim_burst_write(0x5C, 0x00,iic_test_buf, 20);
//			//	while(1);
//				gpio_write(GPIO_GP31,0);
//			}
//			memcpy(host_rb+64, iic_test_buf, 20);
#else
		extern u8 Read_value[20];
		i2c_sim_burst_read(0x5d, 0x00,Read_value, 20);
		blt_push_notify_data(0x07, Read_value, 20);
#endif
		if(gpio_read(GPIO_GP18))
		{
			extern u8 Read_value[20];
			i2c_sim_burst_read(0x5d, 0x00,Read_value, 20);
			blt_push_notify_data(0x07, Read_value, 20);

			run_status = 0x07;
//			u8 iic_buf[20] = {0x06,0x01,0x02,0x03,0x04,0x05,0x06,0x55};
//			for(u8 i=1;i<6;i++) {
//				iic_buf[i] = cnt++;
//			}
//			i2c_sim_burst_write(0x5C, 0x00,iic_buf, 20);///master send data ,received by 'void deal_send_data_to_app()'
		}
	}
#endif

/// do not sleep when slave iic working
	if(wake_up_flag && clock_time_exceed(wake_up_time_tick ,600*1000)) {	//wakeup timeout
		wake_up_flag = 0;
		gpio_set_output_en(GPIO_GP18,0);
		gpio_write(GPIO_GP18, 0);
	}
	else {
		if(wake_up_flag == 0) {
			if(gpio_read(GPIO_GP18)) {
				blt_suspend_mask = 0;
			}
			else {
				blt_suspend_mask = SUSPEND_STATE;//SUSPEND_CONN | SUSPEND_ADV;
			}
		}
		else {
			blt_suspend_mask = 0;
		}
	}
/// do not sleep when slave iic working

	extern void deal_rev_data_from_app();
	extern void deal_send_data_to_app();

	deal_rev_data_from_app();/// deal data received from app

	deal_send_data_to_app();/// check iic data ,send data to app

	ble_status_poll();

	u32 next_wakeup_tick = 0;
	blt_brx_sleep (next_wakeup_tick);

	if(blt_state!=BLT_LINK_STATE_ADV) {
		blt_brx ();
	}
}
