/* Author: Garrett Clay
   Date: 3.1.2014
   Brief: Allow for SPI communication from MSP to EPD.
*/

#include "msp430.h"
#include "spi.h"

void spi_init(void){
	//set data direction
	P4DIR |= SPI_MOSI | SPI_CLK;
	P4DIR &= ~(SPI_MISO);
	
	//switch pins from GPIO to SPI functionality
	P4SEL |= BIT1+BIT2+BIT3;
	
	//Enable software reset
	UCB1CTL1 |= UCSWRST;
	
	//sample on rising edge, MSB, 3 wire asynchronous SPI
	UCB1CTL0 |=  UCMSB | UCMST;
	//CLK select
	UCB1CTL1 |= UCSSEL_2;
	//bit prescale (TODO: look in to this)
	UCB1BR0 = 0xff;
	UCB1BR1 = 0;
	
	//diable software reset
	UCB1CTL1 &= ~UCSWRST;
	UCB1IE |= UCRXIE;
}
