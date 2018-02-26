/*
 * ui_led_pwm.h
 *
 *  Created on: 2017-11-17
 *      Author: liu
 */

#ifndef UI_LED_PWM_H_
#define UI_LED_PWM_H_

#define GPIO_LED          GPIO_GP7
#define GPIO_BUZZER       GPIO_GP10

#define GPIO_KEY          GPIO_GP18

#define BUZZER_FREQ       727
//		buzzer_freq = 740;		///2.7K = 2M/740
//		buzzer_freq = 488;		///4.2K = 2M/488
//		buzzer_freq = 512;		///4K = 2M/512
//		buzzer_freq = 538;		///3.8K = 2M/538
//		buzzer_freq = 256;		///8K = 2M/256

#define LED_USE_PWM       0
#define TIME_CNT          180

typedef struct{
	u16 	offOn_Ms[2];
	u8 		offOn_cnt;	// if offCnt == 0xff, indicate that this state would continues until change to other state.
	u8 		next_mode;
}ui_type_t;

typedef struct{
	ui_type_t *ui_type;
	u8 	cur_mode; //current mode
	u8 	cur_cnt;// current cnt left
	u8 	cur_state;//current state  e.g. led on/off  buzzer on/off
	u32 next_wakeup_tick;
}ui_param_t;

enum{
	Mode_Power_Off,
	Mode_Power_On,
}Power_mode_t;


extern u8 is_buzzer_working ;

extern ui_param_t led_param ;
extern ui_param_t buzzer_param ;

extern u8 	ui_suspend_en ;

extern u8	blt_suspend_mask;
extern u8 	power_mode;//0->power off ; 1->power on
extern u8 	is_power_switch_exist;

extern u32 second_tick ;
extern u8 shutdown_cnt ;

extern void blt_disable_latency() ;

void my_gpio_wakeup_init() ;
void my_buzzer_init() ;
void my_led_init() ;


u32 buzzer_led_ui () ;
void led_enter_mode(u8 mode) ;
void buzzer_enter_mode(u8 mode) ;
void proc_power_onoff(u8 cur_state,u32 poweron_start_tick) ;
void proc_powerOff_handle() ;

void proc_ui_key () ;

void clock_time_Power_Off() ;
void blt_set_ui_suspend_en( u8 suspend_en);


#endif /* UI_LED_PWM_H_ */
