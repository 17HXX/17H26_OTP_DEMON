#include "ui.h"

extern void blt_disable_latency();

u32 wake_up_time_tick;
u8 wake_up_flag;

u8 data_retry_times;
u32	data_retry_tick;

u8 temp_buff[60] = {0x00};
const u8 all_zero[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00,};

static u8 data_count;
u8 run_status;	//no used
void deal_cmd_from_app(u8 *cmd ,u8 len)		////store app data in iic slave
{
	if(memcmp(temp_buff,all_zero,8)) {
		if(memcmp(temp_buff+20,all_zero,8)) {
			if(memcmp(temp_buff+40,all_zero,8)) {
			}
			else {
				memset(temp_buff+40, 0x00, 20);
				memcpy (temp_buff+40, cmd, len);
				temp_buff[40+len] = data_count++;	///last data for count
				data_retry_times = 3;
			}
		}
		else {
			memset(temp_buff+20, 0x00, 20);
			memcpy (temp_buff+20, cmd, len);
			temp_buff[20+len] = data_count++;	///last data for count
			data_retry_times = 2;
		}
	}
	else {
		memset(temp_buff, 0x00, 20);
		memcpy (temp_buff, cmd, len);
		temp_buff[len] = data_count++;	///last data for count
		data_retry_times = 1;
	}
	wake_up_flag = 1;
	wake_up_time_tick = clock_time();
	data_retry_tick = clock_time();
}

void deal_rev_data_from_app() {///when get app data,store data in iic slave ,gp18 output 1 600ms, so that the master can read it during this time
	if(data_retry_times) {
		if(wake_up_flag ||(!gpio_read(GPIO_GP18))) {
			wake_up_flag = 1;
			wake_up_time_tick = clock_time();
			gpio_set_func(GPIO_GP18, AS_GPIO);
			gpio_set_output_en(GPIO_GP18,1);
			gpio_write(GPIO_GP18, 1);		///gp18 output 1 600ms
			if(data_retry_times && clock_time_exceed(data_retry_tick ,30*1000)) {
				data_retry_tick = clock_time();
				memset(host_rb+64, 0x00, 20);
				memcpy (host_rb+64, temp_buff+(data_retry_times-1)*20, 20);//store data in iic slave
				memset(temp_buff+(data_retry_times-1)*20, 0x00, 20);
				data_retry_times--;
			}
		}
	}
}

void deal_send_data_to_app(){	///receive master iic data and send to app
	static u32 send_data_tick;
	if(clock_time_exceed(send_data_tick ,30*1000)){	///check iic data every 30ms
		send_data_tick = clock_time();
		if(host_rb[0] && (host_rb[host_rb[0]+1] == 0x55)) {	///first byte is len ,last data is 0x55. Ensure data integrity
//			if(host_rb[0] &&(host_rb[1] == 0x06)&&(host_rb[2] == 0x02)&&(host_rb[3] == 0x03)&&(host_rb[4] == 0x04)){
//				u8 back_token[] = { 0x06, 0x02, 0x03, 0xff, 0xff, 0xff, 0xff, LOCK_ID, ((LENZE_VER>>4) &0x0f), (LENZE_VER&0x0f), DEVTYPE,
//									0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
//				wake_up_flag = 1;
//				wake_up_time_tick = clock_time();
//				gpio_set_func(GPIO_GP18, AS_GPIO);
//				gpio_set_output_en(GPIO_GP18,1);
////				gpio_set_input_en(GPIO_GP18,0);
//				gpio_write(GPIO_GP18, 1);
//				extern u8 tbl_adv[];
//				memcpy (back_token+11, tbl_adv+15, 6);
//				memset(host_rb+64, 0x00, 20);
//				memcpy (host_rb+64, back_token, 17);
//				host_rb[host_rb[0]+1] = 0x00;
//				host_rb[0] = 0x00;
//			}
//			else if((host_rb[1]==0x04)&&(host_rb[2] == 0x01)){
//				extern u8 tbl_rsp [30];
//				memcpy (tbl_rsp+10, host_rb+3, host_rb[0]-2);
//				tbl_rsp[1] = host_rb[0]-2 + 8;
//				tbl_rsp[8] = host_rb[0]-2 + 1;
//
//				extern u8 tbl_mac [];
//				extern u8 tbl_adv [];
//				blt_init (tbl_mac, tbl_adv, tbl_rsp);
//
//				wake_up_flag = 1;
//				wake_up_time_tick = clock_time();
//				gpio_set_func(GPIO_GP18, AS_GPIO);
//				gpio_set_output_en(GPIO_GP18,1);
////				gpio_set_input_en(GPIO_GP18,0);
//				gpio_write(GPIO_GP18, 1);
//
//				u8 back_name[] = { 0x04, 0x01, 0x01, 0xff};
//
//				memset(host_rb+64, 0x00, 20);
//				memcpy (host_rb+64, back_name, 4);
//				host_rb[host_rb[0]+1] = 0x00;
//				host_rb[0] = 0x00;
//			}
//			else if((host_rb[1]==0x04)&&(host_rb[2] == 0x02)&&(host_rb[3] == 0x03)){
//				wake_up_flag = 1;
//				wake_up_time_tick = clock_time();
//				gpio_set_func(GPIO_GP18, AS_GPIO);
//				gpio_set_output_en(GPIO_GP18,1);
////				gpio_set_input_en(GPIO_GP18,0);
//				gpio_write(GPIO_GP18, 1);
//				u8 back_status[] = { 0x04, 0x03, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, };
//				back_status[3] = blt_state;
//				back_status[4] = data_count;
//				back_status[5] = run_status;
//				memset(host_rb+64, 0x00, 20);
//				memcpy (host_rb+64, back_status, 8);
//				host_rb[host_rb[0]+1] = 0x00;
//				host_rb[0] = 0x00;
//			}
//			else
			{
				if(blt_fifo_num()<3) {
					extern u8 Read_value[];
					memcpy (Read_value, host_rb+1, host_rb[0]);
					blt_push_notify_data(0x07, Read_value, host_rb[0]);
					blt_disable_latency();
					host_rb[host_rb[0]+1] = 0x00;
					host_rb[0] = 0x00;					///clear first data ,make sure send one time
				}
			}
		}
	}
}

