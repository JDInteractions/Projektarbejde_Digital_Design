/*
 * Digital_Design.c
 *
 * Created: 04-06-2021 13:21:47
 * Author : Jan & Lars
 */ 

#include <avr/io.h>






volatile char flag_uart = 0;
volatile unsigned int uart_cnt = 0;
char UARTBuffer[datasize] = {0};
unsigned int Len = 0;
char uart_type = 0;
char current_type = 0;
unsigned int checksum_val = 0;
char checksum_flag = 0;
char rec_complete = 0;
char data[datasize] = {0};
char OLED_buffer[20]={0};
unsigned int RL = 0;
unsigned int S_Rate = 0;
char SW = 0;
char BTN = 0;
unsigned int compare = 0;
char stop,reset = 0;
char telecommand[4]={0};
char spi_package[2]={0};
enum states {sync1, wait, sync2, Length, Length2, Type, ReadData, CS1, CS2};
enum tilstande {scope, set_sample, set_gen, BodePlot};
enum parameter {shape_s,amplitude_s,freq_s};
char param = shape_s;
char state = sync1;
char tilstand = scope;


//Tilstandsmaskine, som genneml�ber datapakkens bestandele. 
void evaluate_recieve(){
	switch(state){
		
		//Tjek om f�rste karakter er 0x55 og skift tilstand hvis sand. 
		case sync1:
		if(UARTBuffer[uart_cnt++] == 0x55){
			state = sync2;
		}
		break;
		
		//Tjek om anden karakter er 0xAA og skift tilstand hvis sand. Ellers skift til tilstand Sync 1 igen.
		case sync2:
		if(UARTBuffer[uart_cnt++]==0xAA){
			state = Length;
			}
		else{
			state = sync1;
		}
		break;
		
		
		//L�s l�ngden af den modtagne pakke (byte1)
		case Length:
		Len = (UARTBuffer[uart_cnt++]<<8);
		state = Length2;
		break;
		
		//L�s l�ngden af den modtagne pakke (byte2)
		case Length2:
		Len = Len + (UARTBuffer[uart_cnt++]);
		compare = Len-2;
		state = Type;			
		break;
		
		//L�s type-byten og gem den i en char. 
		case Type:
		uart_type = UARTBuffer[uart_cnt++];
		state = ReadData;
		break;
		
		//L�s data, hvis der findes databytes i pakken og gem det i data[]  IF ELSE
		case ReadData:
		if(Len>7){
			while(uart_cnt < (compare)){
				data[uart_cnt-5]=UARTBuffer[uart_cnt];
				uart_cnt++;
				break;
				}
				
		//Hvis hele datapakken er l�st og gemt skiftes tilstand. 	
			if(uart_cnt==(compare)){
				state = CS1;
				uart_cnt++;
			}
		}
		break;
		
		//L�s f�rste checksum-byte
		case CS1:
		checksum_val = (UARTBuffer[uart_cnt++]<<8);
		state = CS2;
		break;
		
		//L�s anden checksum-byte og kontroller om den nye int checksum_val == 0x000
		case CS2:
		checksum_val = checksum_val + (UARTBuffer[uart_cnt++]);
		if(checksum_val==0x0000){
			rec_complete=1;	
			uart_cnt=0;
			state = sync1;
			checksum_flag=0;
			Len=0;
			/*memset(UARTBuffer,0,datasize);*/
			}	
		else{
			checksum_flag=1;
			uart_cnt=0;
			/*memset(UARTBuffer,0,datasize);*/
			state = sync1;
			Len=0;
		}		
		break;
	}		
}
	
void toggle_stop(){
	if(stop == 0x03){
		stop = 0x02;
	}
}

//Funktion, som skelner mellem tastetryk i generator-fanen.
//BTN-byte og SW-byte gemmes i hver sin variabel. 
void handle_generator(){
	BTN = data[0];
	SW = data[1];

//Tjek v�rdien af BTN	
	switch(BTN)
	{

//ENTER: konstru�r en SPI-datapakke med det tilsvarende dataindhold.
//Ligeledes opdateres telecommand-pakken.
		case 0x00: 
			if (param == shape_s){
				telecommand[1] = SW;
				spi_package[0]=4;
				spi_package[1]=SW;	
			}
			
			if (param == freq_s){
				telecommand[3] = SW;	
				spi_package[0]=7;
				spi_package[1]=SW;
				
			}
			if (param == amplitude_s){
				telecommand[2] = SW;
				spi_package[0]=5;
				spi_package[1]=SW;
				
				
			}
		break;

//SELECT: Tilstandsloop, som gemmer v�rdien af den nuv�rende valgte parameter (amplitude, frekvens eller shape). 
//Opdater telecommandpakken med den tilsvarende v�rdi. 
		case 0x01:
			switch(param){
				case shape_s:
				
				telecommand[0]=1;
				//transmit Generator DATA UART	
				param = amplitude_s;
				break;
				
				case amplitude_s:
				telecommand[0]=2;
				//transmit_generator
				param = freq_s;
				break;
				
				case freq_s:
				telecommand[0] = 0;
				//transmit_generator
				param = shape_s;
				break;
			}
			break;

//Run/Stop: Toggle stop-char mellem de to start/stop v�rdier. Opdat�r SPI-pakken med tilh�rende v�rdi.		
		case 0x02: //run/stop
			//toggle bit-0;
			if(stop == 0x03){
				stop = 0x02;
			}
			spi_package[0] = stop;
			spi_package[1] = 0;
			//send stop-byte on SPI
			break;

//RESET: Toggle reset-byte og opdater dette i spi-package. 		
		case 0x03:
			if(reset == 0x01){
				reset = 0;
				spi_package[0] = reset;
				spi_package[1] = 0;
				//send reset_byte + 0data SPI;
			}
			else{
				reset = 0x01;	
				spi_package[0] = reset;
				spi_package[1] = 0;
				//send reset_byte + 0data SPI;
			}
			break;
			
	}
}

//Funktion som returnerer tilstande p� baggrund af den l�ste Type modtaget i telemetry. 
enum tilstande handle_type(char input){
	if(uart_type==0x01){
		return set_gen;
	}
	if(uart_type==0x02){
		return set_sample;
	}
	if(uart_type==0x03){
		return BodePlot;
	}
	else return scope;
}


void debug_print(char input, int value){
	char temp[100]={0};
	sprintf(temp,"%u",input);
	sendStrXY(temp,value,13);
}

int main(void)
{
	init_uart_interrupt(UBBR_D);
	_i2c_address = 0x78;
	I2C_Init();
	InitializeDisplay();
	print_fonts();
	_delay_ms(200);
	clear_display();
	sei();
	
    while (1){

//Main tilstandsmaskine
//Reagerer p� uart-receive-flag. Scope er begyndelsestilstanden og herfra kaldes funktionen Handle_type.
//Dermed skiftes der tilstand baseret p� den modtagne uart-type. 

	switch(tilstand){
		
		//Grundtilstand. Tjek for uart-flag. skift tilstand baseret p� uart-type. 
		case scope:
		if(flag_uart==1){
			flag_uart=0;
			tilstand = handle_type(uart_type);
		}
		break;
		
		//"Send" er modtaget. Opdat�r S_rate og RL.
		case set_sample:
		S_Rate = ((unsigned int)data[0]<<8)|(unsigned int)data[1];
//  		putCharUSART(S_Rate>>8);
//  		putCharUSART(S_Rate);
		RL = ((unsigned int)data[2]<<8)|(unsigned int)data[3];
//  		putCharUSART(RL>>8);
//  		putCharUSART(RL);
		tilstand = scope;		
		break;
		
		//Knaptryk fra "Generator" modtaget. Funktionen handle_generator behandler tastetryk. 
		case set_gen:
		handle_generator();
		tilstand = scope;
		break;
		
		//"Start" er modtaget. Spi-pakken-opdateres og der loopes med increments af 1Hz.
		case BodePlot:
		spi_package[0]=7;
		for(int i = 1; i<=255;i++){//adjust frequency 1 hz pr step
			spi_package[1]=i;
			//send SPI package
			//vent - record sample (delay)
			//bode_data[i-1]=ADC-sample
		}
		//Transmit UART datapackage
		tilstand = scope;
		
		
		break;
	}
		
		
		
		for(int i=0;i<10;i++){
			OLED_buffer[i]=data[i]+0x30;
		}		
		sendStrXY(OLED_buffer,4,5);
		debug_print(uart_type,5);
		debug_print(rec_complete,6);
		debug_print(checksum_flag,7);
		sendStrXY("Data:",4,0);
 		sendStrXY("Type:",5,0);
 		sendStrXY("Rec_comp:",6,0);
 		sendStrXY("Checksum_f:",7,0);

		
		
	
}
}




//Service routine for UART receive vector
ISR(USART0_RX_vect){
	UARTBuffer[uart_cnt] = UDR0;
	flag_uart = 1; 
	evaluate_recieve();
 }

