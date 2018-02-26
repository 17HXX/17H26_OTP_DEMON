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
 * ��ʼ��
 * simulation_BaudRate�� ����Ϊ���Ʋ�����
 ************************************************************/
void my_uart_init(void) ;
void uart_tx_byte(u8 temp_data) ;
void uart_tx_array(u8 *uart_tx_array,u8 length) ;

u8 uart_printf_char(u8 *buff) ;
u8 uart_printf_int(u8 *buff , u8 len) ;
u8 uart_printf_u16(u8 len ,u16 *num) ;
u8 uart_printf_u32(u8 len ,u32 *num) ;


/*************************************************************
 * type:��������   ��Ϊ��0����ʱ������Ϊchar  ��Ϊ��len����ʱ������Ϊint
 * type:����      ֱ�Ӹ����ݾ�OK
 * ����ֵ����ӡ�����ݳ���
 ************************************************************/
u8 uart_printf(u8 type , char *buff)  ;


#endif /* UART_PRINTF_H_ */
