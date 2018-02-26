/*
 * uart_printf.h
 *
 *  Created on: 2017-11-16
 *      Author: liu
 */
#ifndef UART_PRINTF_H_
#define UART_PRINTF_H_

#define GPIO_UART            GPIO_GP31
#define GPIO_UART_TICK       0

u8 simulation_BaudRate ;

/*************************************************************
 * 初始化
 * simulation_BaudRate： 数据为控制波特率
 ************************************************************/
void my_uart_init(void) ;
void uart_tx_byte(u8 temp_data) ;
void uart_tx_array(u8 *uart_tx_array,u8 length) ;

u8 uart_printf_char(u8 *buff) ;
u8 uart_printf_int(u8 *buff , u8 len) ;
u8 uart_printf_u16(u8 len ,u16 *num) ;
u8 uart_printf_u32(u8 len ,u32 *num) ;


/*************************************************************
 * type:数据类型   当为“0”的时候输入为char  当为“len”的时候输入为int
 * type:数据      直接给数据就OK
 * 返回值：打印的数据长度
 ************************************************************/
u8 uart_printf(u8 type , char *buff)  ;


#endif /* UART_PRINTF_H_ */
