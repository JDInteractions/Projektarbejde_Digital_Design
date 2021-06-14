/*
 * Digital_Design.c
 *
 * Created: 04-06-2021 13:21:47
 * Author : Jan
 */ 


#include "main.h"


volatile char flag_uart = 0;
volatile char uart_tx_flag = 0;
volatile int uart_cnt_tx = 1;

unsigned int dataSizeTX = 0;
volatile int uart_cnt = 0;
char UARToutputBuffer[OUTPUT_BUF_UART] = {0};
char dataBuffer[DATA_BUF] = {0};	
char * dataPtr; 
	
unsigned char ADC_sample = 0;	//Holds latest adc sample -> read on adc interrupt
volatile char adc_flag = 0; 

char testBuffer1[500] = {0xFF};
int bufferCounter1 = 0;

	
char sampleBuffer[2][SAMPLE_BUF] = {{0},{0}};
unsigned int bufferCounter[2][1] = {{0},{0}};
		
int adc_user = 0;
int uart_user = 1;		//TODO char??

//unsigned int sampleRateTarget = 1000;
unsigned int recordLength = 500;	
int check = 0;
 
 

	
int main(void){
	sampleBuffer[uart_user][0] = 0x55;
	sampleBuffer[uart_user][1] = 0xAA;
	sampleBuffer[uart_user][2] = (recordLength+PADDING_SIZE) >> 8;
	sampleBuffer[uart_user][3] = (recordLength+PADDING_SIZE);
	sampleBuffer[uart_user][4] = 0x02;
	
	sampleBuffer[adc_user][0] = 0x55;
	sampleBuffer[adc_user][1] = 0xAA;
	sampleBuffer[adc_user][2] = (recordLength+PADDING_SIZE) >> 8;
	sampleBuffer[adc_user][3] = (recordLength+PADDING_SIZE);
	sampleBuffer[adc_user][4] = 0x02;

	sampleBuffer[uart_user][HEADER_SIZE+recordLength] = 0x00;//checksum << 8;
	sampleBuffer[uart_user][HEADER_SIZE+recordLength+1] = 0x00; //checksum;
	
	sampleBuffer[adc_user][HEADER_SIZE+recordLength] = 0x00;//checksum << 8;
	sampleBuffer[adc_user][HEADER_SIZE+recordLength+1] = 0x00; //checksum;
	setup();
	setSampleRate(9000);
	
    while (1) {
	
		if(adc_flag){
			transmitUARTPackage(&sampleBuffer[uart_user][0], SCOPE_TYPE, recordLength);
			adc_flag = 0;
			
		}
	}
}


void setup(){
		
	//UART
	init_uart_interrupt1(UBBR_D);

	//Timers
	init_timer1();
	
	//ADC
	init_adc(1);
	startADCSampling(ADC_CHANNEL);

	//Interrupt
	sei();
	
	//OLED-display
	_i2c_address = 0X78;
	I2C_Init();
	InitializeDisplay();
	print_fonts();
	clear_display();
}


// ================================================
// Buffer Handling
// ================================================

//void swapBuffer(){
	//
	//adc_user =   ~adc_user;
	//uart_user =  ~uart_user;
	//
	//bufferCounter1 = 0;//bufferCounter[adc_user][0] = 0;
//}





// ================================================
// ADC
// ================================================


//Calculate and set compare match value for ADC Auto Trigger Source based on target ADC sample rate value.
void setSampleRate(unsigned int sampleRate){
	int compareValue = (F_CPU/(2*sampleRate))/ADC_TRIG_SRC_PS-1;
	OCR1A = compareValue;
	OCR1B = compareValue;
	debug_print_int(OCR1A);
}





// ================================================
// Serial
// ================================================

void transmitUARTPackage(char * data, unsigned char type, unsigned int dataSize){
		
		//Construct package
		//UARToutputBuffer[0] = 0x55;
		//UARToutputBuffer[1] = 0xAA;
		//UARToutputBuffer[2] = (dataSize+PADDING_SIZE) >> 8;
		//UARToutputBuffer[3] = (dataSize+PADDING_SIZE);
		//UARToutputBuffer[4] = type; 
		
		//sampleBuffer[uart_user][0] = 0x55;
		//sampleBuffer[uart_user][1] = 0xAA;
		//sampleBuffer[uart_user][2] = (dataSize+PADDING_SIZE) >> 8;
		//sampleBuffer[uart_user][3] = (dataSize+PADDING_SIZE);
		//sampleBuffer[uart_user][4] = type;
		
		
		//for(int i = HEADER_SIZE; i < HEADER_SIZE+dataSize; i++){
			//UARToutputBuffer[i] = data[i-HEADER_SIZE];
		//}
		
		
		//int checksum = calcCheckSum();
				//UARToutputBuffer[505] = 0x00;
				//UARToutputBuffer[506] = 0x00;
		//UARToutputBuffer[HEADER_SIZE+dataSize] = 0x00;//checksum << 8;
		//UARToutputBuffer[HEADER_SIZE+dataSize+1] =0x00;// checksum; 
		//sampleBuffer[uart_user][HEADER_SIZE+dataSize] = 0x00;//checksum << 8;
		//sampleBuffer[uart_user][HEADER_SIZE+dataSize+1] = 0x00; //checksum;
		//Start transmission by sending first byte, then enable transmit interrupt
		//UDR1 = sampleBuffer[uart_user][0];//UARToutputBuffer[0];
		//SETBIT(UCSR1B, TXCIE1);
			
		//UDR1 = UARToutputBuffer[uart_cnt_tx++];
		for(int i = 0; i < recordLength+PADDING_SIZE; i++){
			putCharUSART(sampleBuffer[uart_user][i]);
		}
		
}




// ================================================
// Utils
// ================================================

unsigned int calcCheckSum(){
	return 0x0000; 
}

void debug_print_char(char input){
	if(DEVEL){
		char temp[100] = {0};
		sprintf(temp,"%u",input);
		sendStrXY(temp, 0,0);
	}
}

void debug_print_int(int input){
	if(DEVEL){
		char temp[100] = {0};
		sprintf(temp,"%u",input);
		sendStrXY(temp, 0,0);
	}
}	


// ================================================
// Service Routines
// ================================================

//Service routine for ADC sample ready
ISR(ADC_vect){
	sampleBuffer[adc_user][5+bufferCounter1++] = ADCH;
	
	if(bufferCounter1 >= recordLength){
		adc_flag = 1;
		
		adc_user = !adc_user;
		uart_user = !uart_user;
			
		bufferCounter1 = 0;
	}
	
	//Overflow
	//if(bufferCounter[adc_user][0] > SAMPLE_BUF){
		//bufferCounter[adc_user][0] = 0;
	//}
}

//Service routine for Timer1 Compare B
ISR (TIMER1_COMPB_vect) {
}

//Service routine for UART receive vector
ISR(USART0_RX_vect){
	//if((UARTBuffer[uart_cnt++] = UDR0) == '\n'){
		//uart_cnt = 0;
		//flag_uart = 1;
	//}
}