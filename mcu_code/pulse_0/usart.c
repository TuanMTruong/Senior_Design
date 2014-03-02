/* Author: Garrett Clay
   Date: 3.1.2014
   Brief: Allow for SPI communication from MSP to EPD.
*/

#include"msp430.h"
#include"usart.h"





void usart_init(void){
	//set data direction
	P4DIR |= USART_TX;
	P4DIR &= ~(USART_RX);

	//switch pins from GPIO to USART functionality
	P4SEL |= USART_RX | USART_TX;
	
	UCA1CTL1 |= UCSWRST;	//enable software reset
	UCA1CTL1 |= UCSSEL_2;	//select SMCLK as source
	UCA1CTL0 = 0;		//set no parity, LSB, 8bit, 1SB, ASYNC

	UCA1BR0 = 139;		//set buad rate at 115200
	UCA0BR1 = 0;
	
	UCA1MCTL |= UCBRS_1 | UCBRF_0;

	UCA1CTL1 &= ~(UCSWRST);

	UCA1IE |= UCRXIE; 

}


#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A0_ISR(void){

	switch(__even_in_range(UCA1IV, 4)){
	case 0: break;
	case 2: //USART Receive data vector

	case 4: //USART Transmit buffer empty


	default: break;


	}



}
