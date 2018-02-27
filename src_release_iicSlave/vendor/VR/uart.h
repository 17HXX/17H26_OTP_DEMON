#include "../../proj/tl_common.h"

#if UART_EN

#define my_uart_init()	simulation_uart_init()
#define puts			puts
#define putbyte			putbyte
void simulation_uart_init();

#else

#define my_uart_init()
#define puts
#define putbyte

#endif
