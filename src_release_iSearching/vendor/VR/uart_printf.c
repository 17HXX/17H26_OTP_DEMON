/*
 * uart_printf.c
 *
 *  Created on: 2017-11-16
 *      Author: liu
 */
#include "ui.h"
#include "../../proj_lib/ble_l2cap/ble_ll_ota.h"

#include "../../proj/drivers/flash.h"

#include "../../proj/tl_common.h"
#include "../../proj_lib/rf_drv.h"
#include "../../proj_lib/pm.h"
#include "../../proj_lib/blt_ll/blt_ll.h"

#include "uart_printf.h"

void my_uart_init()  //uart tx init
{
#if(GPIO_UART_TICK)
	gpio_set_func(GPIO_UART,AS_GPIO);
	gpio_set_output_en(GPIO_UART,1);
	gpio_set_input_en(GPIO_UART,0);
	gpio_write(GPIO_UART, 1);

	simulation_BaudRate = 54;  //// 57600    17.361us
//	simulation_BaudRate = 24;  //// 115200   8.56
//	simulation_BaudRate = 89 ; //// 4800

	sleep_us(1000);
#endif
}
//
_attribute_ram_code_ _attribute_no_inline_ void delay_uart_Tx(void) //延时时间要放到arm里面
{
	volatile int i =0;
	while((i++)<simulation_BaudRate);
}

void uart_tx_byte(u8 temp_data){  //写一个byte
	for(u8 bit=0;bit<10;bit++){
		if(bit == 0){
			gpio_write(GPIO_UART, 0);
		}
		else if(bit == 9){
			gpio_write(GPIO_UART, 1);
		}
		else {
			if(temp_data & (1<<(bit-1))){
				gpio_write(GPIO_UART, 1);
			}
			else {
				gpio_write(GPIO_UART, 0);
			}
		}
		delay_uart_Tx();
	}
}

void uart_tx_array(u8 *uart_tx_array,u8 length)  //写n个
{
#if(GPIO_UART_TICK)
	for(u8 i=0;i<length;i++){
		uart_tx_byte(uart_tx_array[i]);
	}
#endif
}

u8 uart_printf(u8 type , char *buff)
{
#if(GPIO_UART_TICK)

	if(type == 0 ){
		return uart_printf_char(buff) ;
	}else {
		return uart_printf_int(buff , type);
	}
#endif
}

u8 uart_printf_char(u8 *buff)
{
#if(GPIO_UART_TICK)
	u8 ret = 0;
	while(buff[ret])
	{
		uart_tx_byte(buff[ret]);
		ret++;
	}
	return ret;
#endif
}

u8 uart_printf_int(u8 *buff , u8 len)
{
#if(GPIO_UART_TICK)
	u8 ret = 0;

	uart_tx_byte(0x30);
	uart_tx_byte(0x58);

	for(ret=0 ; ret<len ; ret++){

		u8 buff_temp_h = (buff[ret]&0xf0)>>4 ;
		if(buff_temp_h<=9){
			buff_temp_h = buff_temp_h+0x30 ;
		}else {
			buff_temp_h = (buff_temp_h-0x0a)+0x41 ;
		}
		uart_tx_byte(buff_temp_h);

		u8 buff_temp_l = (buff[ret]&0x0f);
		if(buff_temp_l<=9){
			buff_temp_l = buff_temp_l+0x30 ;
		}else {
			buff_temp_l = (buff_temp_l-0x0a)+0x41 ;
		}
		uart_tx_byte(buff_temp_l);

		uart_tx_byte(0x20);
	}

	uart_tx_byte(0x0a);
	return ret+1;
#endif
}

u8 uart_printf_u32(u8 len ,u32 *num)
{
#if(GPIO_UART_TICK)
	u8 uart_temp[4] ;
	u8 ret = 0 ;

	uart_tx_byte(0x30);
	uart_tx_byte(0x58);

	for(u8 i=0 ;i<len ;i++){
		uart_temp[0] = (num[i]>>24)&0xff ;
		uart_temp[1] = (num[i]>>16)&0xff ;
		uart_temp[2] = (num[i]>>8)&0xff ;
		uart_temp[3] = (num[i]>>0)&0xff ;

		for(ret=0 ; ret<4 ; ret++){

			u8 buff_temp_h = (uart_temp[ret]&0xf0)>>4 ;
			if(buff_temp_h<=9){
				buff_temp_h = buff_temp_h+0x30 ;
			}else {
				buff_temp_h = (buff_temp_h-0x0a)+0x41 ;
			}
			uart_tx_byte(buff_temp_h);

			u8 buff_temp_l = (uart_temp[ret]&0x0f);
			if(buff_temp_l<=9){
				buff_temp_l = buff_temp_l+0x30 ;
			}else {
				buff_temp_l = (buff_temp_l-0x0a)+0x41 ;
			}
			uart_tx_byte(buff_temp_l);
		}
		uart_tx_byte(0x20);
	}

	uart_tx_byte(0x0a);
	return ret+1 ;
#endif
}

u8 uart_printf_u16(u8 len ,u16 *num)
{
#if(GPIO_UART_TICK)
	u8 uart_temp[2] ;
	u8 ret = 0;

	uart_tx_byte(0x30);
	uart_tx_byte(0x58);

	for(u8 i=0 ;i<len ;i++){
		uart_temp[0] = (num[i]>>8)&0xff ;
		uart_temp[1] = (num[i]>>0)&0xff ;

		for(ret=0 ; ret<2 ; ret++){

			u8 buff_temp_h = (uart_temp[ret]&0xf0)>>4 ;
			if(buff_temp_h<=9){
				buff_temp_h = buff_temp_h+0x30 ;
			}else {
				buff_temp_h = (buff_temp_h-0x0a)+0x41 ;
			}
			uart_tx_byte(buff_temp_h);

			u8 buff_temp_l = (uart_temp[ret]&0x0f);
			if(buff_temp_l<=9){
				buff_temp_l = buff_temp_l+0x30 ;
			}else {
				buff_temp_l = (buff_temp_l-0x0a)+0x41 ;
			}
			uart_tx_byte(buff_temp_l);
		}
		uart_tx_byte(0x20);
	}
	uart_tx_byte(0x0a);
	return ret+1 ;
#endif
}


