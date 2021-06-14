/*
 * main.h
 *
 * Created: 07-06-2021 13:28:22
 *  Author: Jan
 */ 

#ifndef DEVEL
#define DEVEL 1
#endif

#define F_CPU 16000000UL

#ifndef MAIN_H_
#define MAIN_H_

// ================================================
// Includes
// ================================================
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>

#include <util/delay.h>
#include <string.h>
#include "USART.h"
#include "ADC.h"
#include "Timer.h"


#include "I2C.h"
#include "ssd1306.h"

// ================================================
// Defines/macros
// ================================================
#define SETBIT(ADDR, BIT)(ADDR |= (1<<BIT))
#define CLRBIT(ADDR, BIT)(ADDR &= ~(1<<BIT))
#define CHKBIT(ADDR, BIT)(ADDR & (1<<BIT))

#define HEADER_SIZE	5
#define FOOTER_SIZE 2
#define PADDING_SIZE	HEADER_SIZE+FOOTER_SIZE
#define OUTPUT_BUF_UART	1008
#define DATA_BUF	1008  
 
//Telemetry types
#define BTN_TYPE	0x01
#define SEND_TYPE	0x02
#define START_TYPE	0x03

//Telecommand types
#define GENERATOR_TYPE	0x01
#define SCOPE_TYPE	0x02
#define BODE_TYPE	0x03



#define ADC_CHANNEL	0
#define ADC_TRIG_SRC_PS	64

#define SAMPLE_BUF	1100 


// ================================================
// Functions
// ================================================
void setup();
void transmitUARTPackage(char * data, unsigned char type, unsigned int dataSize);
unsigned int calcCheckSum();
void setSampleRate(unsigned int sampleRate);
//void swapBuffer();
void readBuffer();
void debug_print_char(char input);
void debug_print_int(int input);

#endif /* MAIN_H_ */