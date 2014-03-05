/* Author: Garrett Clay
   Date: 3.1.2014
   Brief: Allow for SPI communication from MSP to EPD.
*/

#include "msp430.h"
#include "usart.h"

#define TX_BUFFER_SIZE 10
#define RX_BUFFER_SIZE 50

static volatile char tx_buffer[TX_BUFFER_SIZE];
static volatile char tx_buffer_front;
static volatile char tx_buffer_back;
static volatile char rx_buffer[RX_BUFFER_SIZE];
static volatile char rx_buffer_front;
static volatile char rx_buffer_back;


void usart_init(void){
	//set data direction
	P4DIR |= USART_TX;
	P4DIR &= ~(USART_RX);

	//switch pins from GPIO to USART functionality
	P4SEL |= USART_RX | USART_TX;

	UCA1CTL1 |= UCSWRST;	//enable software reset
	UCA1CTL1 |= UCSSEL_2;	//select SMCLK as source
	UCA1CTL0 =  UCMODE_3;	//set no parity, LSB, 8bit, 1SB, ASYNC

	UCA1BR0 = 140;		//set buad rate at 115200
	UCA0BR1 = 0x00;

	UCA1MCTL |= UCBRS_7 | UCBRF_0;

	UCA1CTL1 &= ~(UCSWRST);

	UCA1IE |= UCRXIE | UCTXIE; 
}

void usart_put(char data){
	char i;

	i = tx_buffer_front + 1;
	if (i >= TX_BUFFER_SIZE) i = 0;
	while (tx_buffer_back == i) ; // wait until space in buffer
	tx_buffer[i] = data;
	tx_buffer_front = i;
	UCA1IE |=  UCTXIE;
}

char usart_get(void){
	char c, i;
	char back = rx_buffer_back;
	char front = rx_buffer_front;
	while (front == back) ; // wait for character
        i = rx_buffer_back + 1;
        if (i >= RX_BUFFER_SIZE) i = 0;
        c = rx_buffer[i];
        rx_buffer_back = i;
        return c;
}


#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void){
	char i;
	char temp;
	switch(__even_in_range(UCA1IV, 4)){
		case 0: break;
		case 2: //USART Receive data vector
		temp = UCA1RXBUF;
		i = rx_buffer_front + 1;
		if (i >= RX_BUFFER_SIZE) i = 0;
		if (i != rx_buffer_back) {
			rx_buffer[i] = temp;
			rx_buffer_front = i;
		}
		
		break;
		case 4: //USART Transmit buffer empty
		temp = tx_buffer_back;
		if (tx_buffer_front == temp) {
			// buffer is empty, disable transmit interrupt
			UCA1IE &= ~(UCTXIE);
		} else {
			i = tx_buffer_back + 1;
			if (i >= TX_BUFFER_SIZE) i = 0;
			while (!(UCA0IFG&UCTXIFG));
			UCA1TXBUF = tx_buffer[i];
			tx_buffer_back = i;
		}
		break;
		default: break;
		
		
	}
}