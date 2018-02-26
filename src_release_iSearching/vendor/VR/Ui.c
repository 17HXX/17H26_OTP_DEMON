#include "ui.h"
#include "../../proj_lib/ble_l2cap/ble_ll_ota.h"
#include "../../proj/drivers/flash.h"
#include "ui_led_pwm.h"
#include "uart_printf.h"
#include "ADC_10_17H26.h"


//static inline void send_databuf_tmp();
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

#if (1)	///suspend sleep enable
#define		SUSPEND_STATE			SUSPEND_CONN | SUSPEND_ADV
#else
#define		SUSPEND_STATE			0
#endif

#if(OTA_ENABLE)
#define TEST_OTA_1 0
#endif

#define  TEST_SUSPEND_TIME_ENABLE 0


#define LOW_ADV_INTERVAL 		50	//700	//700	///50//700//20//20  //
#define PRODUCT_ID1				0x01
#define PRODUCT_ID2				0x01
#define ADDRESS_OFFSET			19

void blt_disable_latency(){  	//disable current latency,send data faster
	//
	extern u8 blt_retry ;
	blt_retry = 1;
}

u8  tbl_mac [] = {0x20, 0x17, 0x18, 0x29, 0x01, 0x09};  //Store mac address
/// reference "https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile"
u8	tbl_adv [] =
	{	0x00, 24,		//type len   len should <= 37
		0, 0, 0, 0, 0, 0,		    //mac address,Initialization will be modified
		0x02, 0x01, 0x06, 		    //this will allow discovery by other devices 允许其他设备搜索到
		0x03, 0x02, 0xe0, 0xff,		//Incomplete List of 16-bit Service Class UUIDs  设备的服务列表的uuid
		10, 0x09, 'S','m','a','r','t',' ','T','a','g',   //Complete Local Name    设备搜索到的设备名
		0, 0, 0			            //CRC check  CRC校验码
	};
u8	tbl_rsp [] =
		{ 0x04, 14,								//type len
		  0, 0, 0, 0, 0, 0,		                //mac address,Initialization will be modified
		  0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  // Manufacturer Specific Data
		  0, 0, 0							    //  this part contains the CRC code which will be added in the physical layer
		};

///////////////////////////////////// proc_ui ///////////////////////////////////////////
extern u8 is_buzzer_working;

u8 power_mode = Mode_Power_Off;       //0->power off ; 1->power on
u8 proshutter_disconnect_state = 0 ;  //1 -> disconnect_state  0-> not

u32 selfie_adv_mode_start_tick = 0;
u16 blt_is_reconnection = 0;

u8 is_power_switch_exist = 0;
//u8 mle_15_mode = 1;
//u8 proximity_le_mode;

extern ui_type_t  led_ui_buffer_MS[];
extern ui_type_t  buzzer_ui_buffer_MS[];

extern u8 Txpower_value;
extern u8 linkLoss_value;
extern u8 FFE1_value[];

////////////////////////////////////// proc send data ///////////////////////////////////

extern u16 blt_conn_inst;
extern u8 os_check;
/********************************************* user define****************/

u8 device_status_tmp=0;
u32 tick_app_wakeup = 0;
u32 tick_connected_timer_tmp=0;

void tbl_mac_tbl_rsp() //put the defined mac address into the advertising packet,which will be acquired by ios device
{
	tbl_rsp[10] = tbl_mac[5] ;
	tbl_rsp[11] = tbl_mac[4] ;
	tbl_rsp[12] = tbl_mac[3] ;
	tbl_rsp[13] = tbl_mac[2] ;
	tbl_rsp[14] = tbl_mac[1] ;
	tbl_rsp[15] = tbl_mac[0] ;
}

static inline void gatt_init_tmp()
{
	tbl_mac_tbl_rsp() ;								//put mac in Manufacturer Specific Data
	blt_init (tbl_mac, tbl_adv, tbl_rsp);			//Initialize advertising content
	shutter_att_init ();                            //Initialize the service
	blt_suspend_mask = SUSPEND_STATE;               //this SUSPEND_STATE flag controls the entrance into bluetooth low energy <0 is not enter ble mode or enter ble mode>
}

const u8 txPower_table[]= {0x07, 0x04, 0x00, 0xfc, 0xff-9, 0xff-13, 0xff-18, 0xff-23, 0xff-30, 0xff-37, 0xff-40};

static inline void rf_set_power_level_tmp()   //setting rf power level
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
	analog_write (0xa2, 0x06);
	analog_write (0x04, 0x74);
	analog_write (0xa7, 0x43);
	analog_write (0x8d, 0x61);
}

// GPIO Initial function
void my_init_gpio()   //用户数据初始化
{
	my_led_init() ;        //LED  Initial Configuration   LED初始化设置
	my_buzzer_init() ;     //PWM  Initial Configuration   pwm初始化设置
	my_gpio_wakeup_init() ;//Wakeup Initial Configuration 唤醒初始化设置

	my_uart_init() ;       //UART_TX Initial Configuration  uart_TX
	battery_detect() ;     // check battery value

	gpio_set_func(GPIO_GP17, AS_GPIO);
	gpio_set_output_en(GPIO_GP17,0);
	gpio_set_input_en(GPIO_GP17,1);
	//set up pulldown 100k resistor for gpio 17
	gpio_setup_up_down_resistor(GPIO_GP17 , PM_PIN_PULLDOWN_100K) ; //引脚下拉100K
    //<when is_power_switch_exist =1>  chip will wake up after pressing the  power switch  ;
	//<when is_power_switch_exist =0>  chip will wake up automatically;
	is_power_switch_exist = 1 ;         //为"1"按键开机，“0”自动开机
	if(gpio_read(GPIO_GP17)){
		is_power_switch_exist = 0;
	}

	proc_power_onoff(0,clock_time()) ;  //Long press to power on

	led_enter_mode(1) ;                 //LED into mode 1

	buzzer_enter_mode(1) ;              //buzzer into mode1
	led_ui_buffer_MS[1].next_mode = 2;
	buzzer_ui_buffer_MS[1].next_mode = 0;

	gpio_set_func(GPIO_GP17, AS_GPIO);
	gpio_set_output_en(GPIO_GP17,0);
	gpio_set_input_en(GPIO_GP17,0);
	gpio_setup_up_down_resistor(GPIO_GP17 , PM_PIN_UP_DOWN_FLOAT) ;

	blt_set_ui_suspend_en(SUSPEND_STATE) ;//setting  suspend enable
	// printf function ,used in development procedure
	uart_printf(0,"my_init_off:\n");      //
}

void user_init()
{
#if(TAIL_BOOT_CODE_PRESET)
	if(test_boot_code){
		int i;
		att_response_cb(boot_code + i);
	}
#endif
	
#if(DEBUG_FROM_FLASH)
	set_tp_flash();
	set_freq_offset_flash(0x56);
	set_mac_flash(tbl_mac);
//	static u32 chip_id;
//	chip_id = read_chip_ID_flash();
#else
	set_tp_OTP();
	set_freq_offset_OTP(0x56);
	set_mac_OTP(tbl_mac);
//	static u32 chip_id;
//	chip_id = read_chip_ID_OTP();
#endif
	
	rf_set_power_level_tmp();
	gatt_init_tmp();
	my_init_gpio() ;
}


//ATT_OP_READ_REQ:
//rf_packet_att_read_t

int att_read_cb(void*p ){
	rf_packet_att_read_t *req= (rf_packet_att_read_t*)p;

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

int att_write_cb(void*p)	//Callback when received app data
{
	rf_packet_att_write_t *src = (rf_packet_att_write_t*)p;
	u8 *value;
	value=&src->value;
	u16 att_handle = src->handle | (src->handle1  << 8);

	u16 ret_handle = ATT_NO_HANDLED;
	// this correspond to  att_handle=7,ie: immediateAlertLevel  service control method
	if(7 == att_handle){        //handle in my_Attributes[] to receive immediateAlert value
		u8 write_value = value[0];
		if(write_value == 1 || write_value == 2){
			buzzer_enter_mode(4);
			led_enter_mode(5);
			buzzer_ui_buffer_MS[4].offOn_Ms[0] = 180;
			led_ui_buffer_MS[5].offOn_Ms[0] = 180;
		}
		else if(write_value == 3){
			buzzer_enter_mode(4);
			led_enter_mode(5);
			buzzer_ui_buffer_MS[4].offOn_Ms[0] = 1900;
			led_ui_buffer_MS[5].offOn_Ms[0] = 1900;
		}
		else {
			buzzer_enter_mode(0);
			led_enter_mode(0);
		}
	}
	ret_handle = att_handle;
	return ret_handle;
}
/*
 other opcode
 * */
void att_response_cb( u8 *p){
	rf_packet_l2cap_req_t * req = (rf_packet_l2cap_req_t *)p;
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
 
void task_connection_established(rf_packet_connect_t* p){///Callback when connected
	//adv_start_tick = last_update_paramter_time = clock_time();// in bond state better
	//adv_time_cnt = 0;
	tick_connected_timer_tmp=clock_time();
	device_status_tmp=CONNECTED_DEVICE_STATUS;

	led_enter_mode(0);
	led_ui_buffer_MS[1].next_mode = 0;
	buzzer_ui_buffer_MS[1].next_mode = 0;
	buzzer_enter_mode(0);
}

/*task_connection_terminated
 * This event is returned once connection is terminated
 * ex:notify application connection terminated; reset connection para*/
//volatile u8 aa_cnt = 0;
void task_connection_terminated(rf_packet_connect_t* p){	///Callback when disconnected
	//app_wakeup_tick = adv_start_tick = clock_time();
	//adv_time_cnt = 0;
	device_status_tmp=POWER_ON_DEVICE_STATUS;

	buzzer_enter_mode(2);
	led_enter_mode(2);
	shutdown_cnt = TIME_CNT ;

	selfie_adv_mode_start_tick =  clock_time ();
	blt_is_reconnection = 0;
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
}
// this part contains some self defined function : how to acquire battery_value,and send specific key when press the button.
static inline void proc_ui()
{
	proc_battery_value() ;	///battery value check
	proc_ui_key () ;		///button deal
}

// This part relates to 1.blt_brx_sleep() which control the entrance to the BLE mode,
// 2.blt_send_adv() ": advertising channel and  3.blt_brx() : blue tooth transmitting and receiving function
static inline void public_loop()
{
	tick_app_wakeup = buzzer_led_ui() ;		///ui wake up time
	blt_brx_sleep (tick_app_wakeup);		/// in suspend , wakeup by time( time should >clock_time()) or gpio.
	if(blt_state!=BLT_LINK_STATE_ADV){
		blt_brx ();							// Send and receive data with app when connected
	}else {
		blt_send_adv (BLT_ENABLE_ADV_ALL);		///setting  advertising channel   when disconnected
	}
}

void main_loop()
{
	if (blt_state == BLT_LINK_STATE_ADV)
	{
		//the  shutdown function, contain the Timer setting
		clock_time_Power_Off() ;  //no connect auto powerdown by time
		//this set the advertising interval to 2000ms  which affect power consumption
		blt_adv_interval = ((rand()%5) +2000)*CLOCK_SYS_CLOCK_1MS;  //2000ms
//		blt_adv_interval = ((rand()%5) +100)*CLOCK_SYS_CLOCK_1MS;  //100ms
//		blt_adv_interval = ((rand()%5) +1000)*CLOCK_SYS_CLOCK_1MS;  //1000ms
	} else {
		if((device_status_tmp==CONNECTED_DEVICE_STATUS)&& clock_time_exceed(tick_connected_timer_tmp,2*1000*1000))
		{
			//update the connection parameters
			device_status_tmp=AFTER_CONNECTED_DEVICE_STATUS;
			blt_update_connPara_request(200,220,4,500);//reference "http://blog.csdn.net/qq576494799/article/details/51305352"
		}
	}

	if(power_mode == Mode_Power_On){
		// This function will only be running after device is switched on
		proc_ui ();
	}else{
		proc_powerOff_handle();  //shutdown.
	}

/******************************public area*******************/
	// This part relates to the Library functions which contains in liblt_17H26_0110.a
	//1.blt_brx_sleep() which control the entrance to the BLE mode,
	// 2.blt_send_adv() ": advertising channel and
	//3.blt_brx() : blue tooth transmitting and receiving function
	public_loop();
}
