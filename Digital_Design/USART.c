/*
 * USART.c
 *
 * Created: 11-03-2021 17:05:56
 *  Author: Jan
 */ 

#include "USART.h"

//Initialize UART0 with receive interrupt
void init_uart_interrupt(unsigned int  ubrr){
	
	UCSR0A = (1<<U2X0);								//Asynkron Double speed mode
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);//RX & TX & RX interrupt enable
	UCSR0C |= (1<<UCSZ00) | (1<<UCSZ01);			//8-bit data
	UBRR0H = (unsigned char)(ubrr>>8);				//UBRR(16-bit) 19200 baud
	UBRR0L = (unsigned char)ubrr;
	
}	

//Initialize UART0
void init_uart(unsigned int  ubrr){
	
	UCSR0A = (1<<U2X0);					//Double speed half-duplex
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0);
	UCSR0C |= (1<<UCSZ00) | (1<<UCSZ01);
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	
}

//Returns 8-bit from UART receive buffer
char getCharUSART(void){
	
	while(!(UCSR0A & (1<<RXC0)));
	return UDR0;
}

//Transmits 8-bit 
void putCharUSART(char tx){
	
	while(!(UCSR0A & (1<<UDRE0))); //continues when transmit buffer is empty
	UDR0 = tx;
}

//Transmits string
//Argument: array with data
void transmitStrUSART(char * ptr){
	
	while(*ptr){
		putCharUSART(*ptr);
		ptr ++; 
	}
}

//Receive a string.
//Returns number of characters read into the buffer when '\n' is met.  
int receiveStrUSART(char * buffer){
	char input = 0; 
	int i = 0;
	while((input = getCharUSART()) != 10){
		buffer[i] = input; 
		i++;
	}
	return i; 
}


void USART_Flush( void )
{
	unsigned char dummy;
	while ( UCSR0A & (1<<RXC0) ) dummy = UDR0;
}



