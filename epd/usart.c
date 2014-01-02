/******************************************************************/
// Author: Tuan
// Data: 12/20/13
// has basic usart Function 
// TODO add Interrupt support
/******************************************************************/

//includes 
#include<avr/io.h>
#include<avr/interrupt.h>
#include<usart.h>


/******************************************************************/
// set up usart for 8 bit, 1 stop bit, and no parity
// usart is set to double speed
// Takes a 32 bit number to calculate baudrate
// git baud rate in bps i.e. 9600
/******************************************************************/
void usart_setup(uint32_t baudrate){
	UCSR1A = (1<<U2X1);			//double speed
	UCSR1B = (1<<RXEN1) | (1<<TXEN1);	//enable rx & tx
	UCSR1C = (3<<UCSZ10);			//8B,1SB,NP

	UBRR1L = (F_CPU/ 4 / baudrate-1) / 2;	//Baud
	return;
}


/******************************************************************/
// Send byte polling
// wait till data register is cleared out
// put new data in to data register for transfer
/******************************************************************/
void usart_sendbyte(uint8_t data){
	while(!(UCSR1A & (1<<UDRE1))){}	//wait till register clear
	UDR1 = data;			//new data
	return;
}


/******************************************************************/
// Send array polling
// takes a pointer to an array and length of array to send out
/******************************************************************/
void usart_sendarray(char *array, uint8_t length){
	uint8_t counter = 0;	//counter

	for(counter =0; counter<length; counter++){
		//send each data of array out
		usart_sendbyte(*(array+counter));
	}
	return;
}


/******************************************************************/
// Read a byte from data register
// return byte in 8 bite unsigned int
/******************************************************************/
uint8_t usart_readbyte(void){
	while(!(UCSR1A & (1<<RXC1))){}	//wait till there is data
	return UDR1;			//return data in register
}
