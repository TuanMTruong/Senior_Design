/* Author: Tuan Truong & Garrett Clay
   Date: 3.1.2014
   Brief: Allow for SPI communication from MSP to EPD.
*/

#include "msp430.h"
#include "spi.h"
#include "clock.h"

#define TX_BUFFER_SIZE 10

static volatile char tx_buffer[TX_BUFFER_SIZE];
static volatile char tx_buffer_front;
static volatile char tx_buffer_back;


void spi_init(void){
	//set data direction
	P4DIR |= SPI_MOSI | SPI_CLK;
	P4DIR &= ~(SPI_MISO);
	
	//switch pins from GPIO to SPI functionality
	P4SEL |= SPI_MOSI | SPI_MISO | SPI_CLK;
	
	//Enable software reset
	UCB1CTL1 |= UCSWRST;
	
	//sample on rising edge, MSB, 3 wire asynchronous SPI
	UCB1CTL0 |=  UCMSB | UCMST |UCCKPH;
	//CLK select
	UCB1CTL1 |= UCSSEL_2;
	//bit prescale (TODO: look in to this)
	UCB1BR0 = 0xff;
	UCB1BR1 = 0;
	
	//diable software reset
	UCB1CTL1 &= ~UCSWRST;
	tx_buffer_front = tx_buffer_back = 0;
	//UCB1IE |= UCTXIE;
}

void spi_on(void){
	spi_init();
	spi_put(0x00);
  	spi_put(0x00);
 	__delay_cycles(US_DELAY*10);
}

void spi_off(void){
  	spi_put(0x00);
  	spi_put(0x00);
  	__delay_cycles(US_DELAY*10);
}

void spi_put(char data){
	char i;
	i = tx_buffer_front + 1;
	if(i >= TX_BUFFER_SIZE) {i = 0;}
	while(tx_buffer_back == i);	//wait for buffer space
	tx_buffer[i] = data;
	tx_buffer_front = i;
	UCB1IE |= UCTXIE;	//enable spi tx
	//UCB1TXBUF = data;
}

void spi_put_wait(char c, int busy_pin){
	spi_put(c);
	//wait for COG ready
	while(P6IN && busy_pin){}
}

void spi_send(char cs_pin, const char *buffer, long length){
	//CS low
	P6OUT &= ~(1<<cs_pin);
  
	//send all data
	for(long i = 0; i < length; i++){
	spi_put(*buffer++);
	}
  
	//CS high
	P6OUT |= (1<<cs_pin);
}



#pragma vector=USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void){
	char i;
	char temp;
	switch(__even_in_range(UCB1IV,4)){
		case 0:
		break;
		case 2:		//data received, #dontcare ;)
		break;
		case 4:
		temp = tx_buffer_back;
		if(tx_buffer_front != temp){
			//load buffer
			i = tx_buffer_back + 1;
			if(i >= TX_BUFFER_SIZE){i = 0;}
			UCB1TXBUF = tx_buffer[i];
			tx_buffer_back = i;
		}
		else{
			UCB1IE &= ~UCTXIE;	//disable spi tx
		}
		break;
		default:
		break;
	}
}
			
		
			       	       
			       
