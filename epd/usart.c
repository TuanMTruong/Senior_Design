/******************************************************************/
// Author: Tuan
// Data: 12/20/13
/******************************************************************/

//includes 
#include<avr/io.h>
#include<avr/interrupt.h>
#include<usart.h>


void usart_setup(uint32_t baudrate){
	UCSR1A = (1<<U2X1);
	UCSR1B = (1<<RXEN1) | (1<<TXEN1);
	UCSR1C = (3<<UCSZ10);

	UBRR1L = (F_CPU/ 4 / baudrate-1) / 2;
}

void usart_sendbyte(uint8_t data){
	while(!(UCSR1A & (1<<UDRE1))){}
	UDR1 = data;
}

void usart_sendarray(uint8_t *array, uint8_t length){
	uint8_t counter = 0;
	
	for(counter =0; counter<length; counter++){
		usart_sendbyte(*(array+counter));
	}
}

uint8_t usart_readbyte(void){
	while(!(UCSR1A & (1<<RXC1))){}
	return UDR1;
}
