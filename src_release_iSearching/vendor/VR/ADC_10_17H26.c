/*
 * ADC_10_17H26.c
 *
 *  Created on: 2017-11-20
 *      Author: liu
 */

#include "ui.h"
#include "../../proj_lib/ble_l2cap/ble_ll_ota.h"
#include "../../proj/drivers/flash.h"
#include "../../proj/tl_common.h"
#include "../../proj_lib/rf_drv.h"
#include "../../proj_lib/pm.h"
#include "../../proj_lib/blt_ll/blt_ll.h"
#include "ui_led_pwm.h"
#include "uart_printf.h"
#include "ADC_10_17H26.h"

void battery_detect()  ///deal the battery
{
#if(MODULE_ADC_ENABLE)	///adc init
	reg_adc_chn_input = ADC_CHN_VDDEC;  // adc chn
	reg_adc_ref = ADC_REF_VDDH;         //ref
	reg_adc_sample_clk_res = MASK_VAL(FLD_ADC_SAMPLE_CLOCK,ADC_CLOCK_CYCLE_3,
										FLD_ADC_RESOLUTION,ADC_RES_10_BITS,
										FLD_ADC_DATA_SIGN,ADC_DATA_SIGN_POSITIVE);
	//set clk  enable
	reg_adc_step_l = 4;
	reg_adc_mod = MASK_VAL(FLD_ADC_MOD, 192, FLD_ADC_CLK_EN, 1);
#endif

	u32 sum_bat = 0;
	u16 bat_min = adc_get();
	u16 bat_max = adc_get();
	for(u8 i=0;i<18;i++){
		u16 temp_bat = adc_get();
		if(temp_bat > bat_max){
			bat_max = temp_bat;
		}
		if(temp_bat < bat_min){
			bat_min = temp_bat;
		}
		sum_bat += temp_bat;
	}
	u16 disp_bat = (u16)((sum_bat- bat_min - bat_max)>>4);
////			0x023c	3.28v
////			0x0311	2.4v
////			0x0320	2.3v
	if(disp_bat> 0x311){	///低于某个电压关机
		analog_write (0x08, 0x0c);//G18  down 100k
		power_mode = Mode_Power_Off;
	}
}

void proc_battery_value ()  //adc做算法滤波处理，不能做太大的算法，否则功耗会起来。
{
	static u16 last_bat_vol = 0;
	static u32 last_bat_detect_time = 0;
	static u8 low_power_cnt = 0;

	if(is_buzzer_working || !clock_time_exceed(last_bat_detect_time , 3*1000*1000))
			return;
	last_bat_detect_time = clock_time();

	u32 sum_bat = 0;
	u16 bat_min = adc_get();
	u16 bat_max = adc_get();
	for(u8 i=0;i<18;i++){
		u16 temp_bat = adc_get();
		if(temp_bat > bat_max) {
			bat_max = temp_bat;
		}
		if(temp_bat < bat_min){
			bat_min = temp_bat;
		}
		sum_bat += temp_bat;
	}
	u16 batt = (u16)((sum_bat- bat_min - bat_max)>>4);
//		AAA_battery = batt;
//		blt_push_notify(16, AAA_battery, 2);
//			0x023c	3.28v
//			0x0311	2.4v
//			0x0320	2.3v
	if(batt < 0x248){
		batt = 0x0248;
	}
	else if(batt > 0x311){
		batt = 0x0311;
	}

	u16 delta_vol = last_bat_vol > batt ?(last_bat_vol - batt) :(batt - last_bat_vol) ;
	last_bat_vol = batt;
	if(delta_vol > 50){
		return;
	}else{
		u8 temp_my_batVal = (0x0311 - batt)*100 /(0x0311 - 0x0248);
		my_batVal = temp_my_batVal;

		if(my_batVal <= 10){
			low_power_cnt++;
			if(low_power_cnt > 10){
				analog_write (0x1a, 0x01);
				power_mode = Mode_Power_Off;
			}
		}
		else {
			low_power_cnt = 0;
		}
	}
}
