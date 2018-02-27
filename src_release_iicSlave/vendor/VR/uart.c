#include "../../proj/tl_common.h"

#if UART_EN

#define simulation_BaudRate 22

volatile u8 uart_bit;
volatile int delay_cnt =0;
void simulation_uart_init()
{
	gpio_set_func(GPIO_GP17, AS_GPIO);
	gpio_set_output_en(GPIO_GP17,1);
	gpio_set_input_en(GPIO_GP17,0);
	gpio_write(GPIO_GP17, 1);
//	simulation_BaudRate = 53;////波特率57600   17.361us
//	simulation_BaudRate = 22;////波特率115200   8.56
}

_attribute_ram_code_ void delay_uart_Tx(void)////delay 函数需要放在 ram里面，否则调用的时候延迟比较重
{
	delay_cnt = 0;
	while((delay_cnt++)<simulation_BaudRate);
}

_attribute_ram_code_ void putbyte(char a)
{
	for(uart_bit=0;uart_bit<10;uart_bit++){
		if(uart_bit == 0){
			gpio_write(GPIO_GP17, 0);
		}
		else if(uart_bit == 9){
			gpio_write(GPIO_GP17, 1);
		}
		else {
			if(a & (1<<(uart_bit-1))){
				gpio_write(GPIO_GP17, 1);
			}
			else {
				gpio_write(GPIO_GP17, 0);
			}
		}
		delay_uart_Tx();
	}
}

char getbyte()
{
	return 0;
}



void putschar(char a)
{
    if(a == '\n')
    {
        putbyte(0x0d);
        putbyte(0x0a);
    }
    else
    {
        putbyte(a);
    }
}

void put_u4hex(u8 dat)
{
    dat = 0xf & dat;

    if(dat > 9)
    {
        putbyte(dat - 10 + 'A');
    }
    else
    {
        putbyte(dat + '0');
    }
}
void put_u32hex(u32 dat)
{
    putbyte('0');
    putbyte('x');
    put_u4hex(dat >> 28);
    put_u4hex(dat >> 24);

    put_u4hex(dat >> 20);
    put_u4hex(dat >> 16);

    put_u4hex(dat >> 12);
    put_u4hex(dat >> 8);

    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putschar('\n');
}

void put_u32hex0(u32 dat)
{

    put_u4hex(dat >> 28);
    put_u4hex(dat >> 24);

    put_u4hex(dat >> 20);
    put_u4hex(dat >> 16);

    put_u4hex(dat >> 12);
    put_u4hex(dat >> 8);

    put_u4hex(dat >> 4);
    put_u4hex(dat);
}

void put_u64hex(u64 dat)
{
    putbyte('0');
    putbyte('x');
    put_u32hex0(dat>>32);
    put_u32hex0(dat);
    putschar('\n');
}
void put_u16hex(u16 dat)
{
    putbyte('0');
    putbyte('x');


    put_u4hex(dat >> 12);
    put_u4hex(dat >> 8);

    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putschar(' ');
}

void put_u8hex(u8 dat)
{
    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putschar(' ');
}

void printf_buf(u8 *buf, u32 len)
{

    u32 i ;

    for(i = 0 ; i < len ; i++)
    {
        if((i % 16) == 0)
        {
            putbyte('\n') ;
        }

        put_u8hex(buf[i]) ;
    }

    putbyte('\n') ;

}

int puts(const char *out)
{
    while(*out != '\0')
    {
    	putschar(*out);
        out++;
    }

    return 0;
}

void put_buf(u8 *buf, u32 len )
{
    u32 i ;
    for(i = 0 ; i < len ; i++)
    {
        if((i%16)==0)
        {
        	putschar('\n') ;
        }
        put_u8hex(buf[i]) ;
    }
    putschar('\n') ;
}

#endif
