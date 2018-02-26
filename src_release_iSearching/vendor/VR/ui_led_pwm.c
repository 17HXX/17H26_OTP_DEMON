/*
 * ui_led_pwm.c
 *
 *  Created on: 2017-11-17
 *      Author: liu
 */

#include "../../proj/tl_common.h"
#include "../../proj_lib/blt_ll/blt_ll.h"
#include "../../proj/mcu_spec/gpio_17h26.h"
#include "ui_led_pwm.h"


ui_type_t buzzer_ui_buffer_MS[] = {  //pwm
		//off	  		on      cnt     next mode
		{{0,			0},		0,		0},//0	    //normal
		{{40,			200},   0x02,	0},//1		// power on 1S/50ms,adjust power mode
		{{180,			60},    0xff,	0},//2		// alert mode
		{{0,			60},	0x01,	0},//3		// button
		{{180,			60},	0xff,	4},//4		// button
};

ui_type_t led_ui_buffer_MS[] = {     //LED
		//off	  		on      cnt		next mode
		{{0,			0},		0,		0},//0		//power off  and connect state
		{{0,			2000},  0x01,	2},//1		//power on:0ms /2S: 1time
		{{2900,			100},	0xff,	2},//2		//adv : 3S  /50ms  :2950 + 50
		{{0,			100},	0x02,   0},//3		//button
		{{0,			100},   0xff,	4},//4		// alert mode

		{{180,			60},	0xff,   5},//5		//button
};


u8 is_buzzer_working = 0;

ui_param_t led_param ={led_ui_buffer_MS};
ui_param_t buzzer_param = {buzzer_ui_buffer_MS};

u8 	ui_suspend_en = 0;

u32 second_tick ;
u8 shutdown_cnt = TIME_CNT ;

///////////////////////////////////////////////////////////////////////////////////////
//						Function
///////////////////////////////////////////////////////////////////////////////////////

void my_led_init()
{
	gpio_set_func(GPIO_LED, AS_GPIO) ;
	gpio_set_output_en(GPIO_LED,1) ;
	gpio_set_input_en(GPIO_LED,0) ;
}

void my_buzzer_init()
{
	/*set PWM Param*/
	write_reg8(0x781,0x0f);//32M/(15+1) = 2M
	/* set buzzer pwm */
	write_reg16(0x79a,BUZZER_FREQ);       //set pwm3 max cycle 2.7K = 2M/741
	write_reg16(0x798,(BUZZER_FREQ>>1));  //set pwm3 duty_cycle = 50% = 370/1000

	gpio_set_func(GPIO_BUZZER, AS_PWM) ;
	gpio_set_output_en(GPIO_BUZZER,1) ;
	gpio_set_input_en(GPIO_BUZZER,0) ;
}

void my_gpio_wakeup_init()
{
	gpio_set_func(GPIO_KEY, AS_GPIO);
	gpio_set_output_en(GPIO_KEY,0);
	gpio_set_input_en(GPIO_KEY,1);

	gpio_setup_up_down_resistor(GPIO_KEY , PM_PIN_PULLDOWN_100K) ;
	gpio_enable_wakeup_pin(GPIO_KEY , 1 , 1) ;
	cpu_set_gpio_wakeup (GPIO_KEY , 1 , 1 ) ;
	blt_set_wakeup_source(PM_WAKEUP_PAD);
}


static inline void led_beep(u8 onOff){
	gpio_write(GPIO_LED,onOff);
}

static inline void buzzer_beep(u8 onOff){
	if(onOff){
		write_reg8(0x780,0x02);
		is_buzzer_working = 1;
	}else{
		write_reg8(0x780,0x00);
		is_buzzer_working = 0;
	}
}

u32 calc_led_ui(ui_param_t *ui_bl)
{
	if(ui_bl->cur_mode == 0) return 0;

	if(ui_bl->next_wakeup_tick - (clock_time() + 2*CLOCK_SYS_CLOCK_1MS) < BIT(30)){
		return ui_bl->next_wakeup_tick;
	}

	if(ui_bl->cur_state && ui_bl->cur_cnt && ui_bl->cur_cnt != 0xff){//!=0 !=0xff ==on
		ui_bl->cur_cnt --;
	}

	if(ui_bl->cur_cnt == 0 ){
		ui_bl->cur_mode = ui_bl->ui_type[ui_bl->cur_mode].next_mode;
		ui_bl->cur_cnt = ui_bl->ui_type[ui_bl->cur_mode].offOn_cnt;
	}

	if(ui_bl->cur_cnt && ui_bl->ui_type[ui_bl->cur_mode].offOn_Ms[0] == 0){
		ui_bl->cur_state = 1;
	}else {
		ui_bl->cur_state = ui_bl->cur_state ? 0: 1;
	}
	//ui_bl->cur_state = ui_bl->cur_state ? 0: 1;
	ui_bl->next_wakeup_tick = clock_time() + ui_bl->ui_type[ui_bl->cur_mode].offOn_Ms[ui_bl->cur_state] * CLOCK_SYS_CLOCK_1MS;

	return ui_bl->next_wakeup_tick;
}

u32 buzzer_led_ui ()
{
	u32 next_led_wakeup_tick = calc_led_ui(&led_param);
	led_beep(led_param.cur_state);

	u32 next_buzzer_wakeup_tick = calc_led_ui(&buzzer_param);
	buzzer_beep(buzzer_param.cur_state);

		// return the min of(next_buzzer_wakeup_tick,next_led_wakeup_tick except 0) or return 0;
	#if LED_USE_PWM
		if(buzzer_param.cur_state || led_param.cur_state){
	#else
		if(buzzer_param.cur_state){
	#endif
			blt_suspend_mask = 0;
		}
		else {
			blt_suspend_mask = ui_suspend_en;
		}

		if((next_led_wakeup_tick - next_buzzer_wakeup_tick < BIT(30)  && next_buzzer_wakeup_tick) || next_led_wakeup_tick == 0){
			return next_buzzer_wakeup_tick;
		}

	return next_led_wakeup_tick;
}

void ui_enter_mode(ui_param_t *ui_param,u8 mode){
	if(ui_param->cur_mode == 1 || ui_param->cur_mode == mode){
		return;
	}
	ui_param->cur_cnt = ui_param->ui_type[mode].offOn_cnt;
	ui_param->cur_mode = mode;
	ui_param->cur_state = 0;
	ui_param->next_wakeup_tick = clock_time();
}
void led_enter_mode(u8 mode){
	ui_enter_mode(&led_param, mode);
	return;
}

void buzzer_enter_mode(u8 mode){
	ui_enter_mode(&buzzer_param, mode);
	return;
}

///////////////////////////// power mode handle ///////////////////////////
void proc_power_onoff(u8 cur_state,u32 poweron_start_tick)
{
	if(is_power_switch_exist == 0){
		power_mode = Mode_Power_On;
		return;
	}

	if(!cur_state){//power off state
		while(!clock_time_exceed(poweron_start_tick,2000*1000)){
			if(gpio_read(GPIO_KEY)){
				blt_sleep_wakeup(0,PM_WAKEUP_TIMER,clock_time() + 100*CLOCK_SYS_CLOCK_1MS);
			}else{
				blt_sleep_wakeup(1,PM_WAKEUP_PAD,0);
			}
		}
		power_mode = Mode_Power_On ;
	}
	else {
		if(clock_time_exceed(poweron_start_tick,2000*1000)){
			power_mode = Mode_Power_Off;
		}
	}
}
void proc_powerOff_handle()
{
	buzzer_beep(1) ;
	for(u8 i=1; i<4; ++i)
	{
		led_beep (i&0x01);
		sleep_us (100*1000);
	}
	sleep_us(100*1000);

	blt_set_wakeup_source(PM_WAKEUP_PAD);
	while(gpio_read(GPIO_KEY))
		blt_sleep_wakeup(0,PM_WAKEUP_TIMER,clock_time() + CLOCK_SYS_CLOCK_1S);
	led_beep (0);
	buzzer_beep(0) ;
	blt_sleep_wakeup(1,PM_WAKEUP_PAD,0);
}

void clock_time_Power_Off()   //auto powerdown
{
	if(clock_time_exceed(second_tick, 1000*1000)){
		second_tick = clock_time();
		if(shutdown_cnt){
			shutdown_cnt--;
			if(shutdown_cnt < 2){
				power_mode = Mode_Power_Off;
				analog_write (0x1A, 0x01);//set enable
			}
		}
	}
}


void proc_ui_key ()   //button deal
{
	static u8 last_button_pressed   = 0;
	static u32 last_button_press_time = 0;
	static u32 last_button_release_time = 0;
	static u32 power_key_start_time;
	static u32 button_check_time = 0;

	static u8 key_mode_num_led_off ;

	u32 clock_tick = clock_time();
	//button1 --> gp18
	if(!button_check_time || (clock_tick - button_check_time > 5 * CLOCK_SYS_CLOCK_1MS ))//((clock_tick - last_button_release_time -150*CLOCK_SYS_CLOCK_1MS + 1) < BIT(31)) || (clock_tick - last_button_release_time > 0x7fffffff))
	{
		button_check_time = clock_tick;

		u8 gpio18_level = !!gpio_read ( GPIO_KEY );//sw1 sw2
		if( gpio18_level ){
			if(power_key_start_time == 0)
			{
				power_key_start_time = clock_tick;
			}
			proc_power_onoff(1, power_key_start_time);

			gpio_set_interrupt_pol(GPIO_KEY,1);
		}
		else {
			power_key_start_time = 0;
			gpio_set_interrupt_pol(GPIO_KEY,0);
		}

		u8 button_pressed = gpio18_level ;

		if(button_pressed){
			if(!last_button_pressed && (!button_check_time || clock_time_exceed(last_button_release_time, 80*1000))){
				buzzer_enter_mode(3);
				led_enter_mode(3);

				last_button_pressed = 1;
				key_mode_num_led_off = button_pressed ;
				last_button_press_time = clock_tick;

				blt_disable_latency() ;
				blt_push_notify(10, 0x01, 1);  //send data to app

//				proximity_le_mode = 0;
//				selfie_adv_mode_start_tick = clock_tick;
				shutdown_cnt = TIME_CNT ;
			}
		}
		else {
			if(last_button_pressed && clock_time_exceed(last_button_press_time, 80*1000)){
				last_button_pressed = 0;
				last_button_release_time = clock_tick;
			}
		}
	}
}

void blt_set_ui_suspend_en( u8 suspend_en){   //suspend control
	ui_suspend_en = suspend_en;
}
