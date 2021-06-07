/*
 * Digital_Design.c
 *
 * Created: 04-06-2021 13:21:47
 * Author : Jan
 */ 


#include "main.h"




volatile char flag_uart = 0;
volatile int uart_cnt = 0;
char UARTBuffer[100] = {0};
	

int main(void)
{
    while (1) 
    {
    }
}





//Service routine for UART receive vector
ISR(USART0_RX_vect){
	if((UARTBuffer[uart_cnt++] = UDR0) == '\n'){
		uart_cnt = 0;
		flag_uart = 1;
	}
}