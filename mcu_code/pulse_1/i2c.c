/* Authors: Tuan Truong & Garrett Clay
Date: 3.6.2014
Brief: I2C library

*/

#include "msp430.h"
#include "i2c.h"
#include "clock.h"
 
#define RX_BUFF_SIZE 10

unsigned char tx_cnt;
unsigned char *p_tx_data;

unsigned char *p_rx_data;	
volatile unsigned char rx_buff[RX_BUFF_SIZE];
 
char rx_front = 0;
char rx_back = 0;

void i2c_init(void){
	P3SEL |= SDA_PIN | SCL_PIN;	//set output pins
	UCB0CTL1 |= UCSWRST;		//software reset
	UCB0CTL0 = UCMST | UCSYNC | UCMODE_3;	//master, synchronous, i2c mode
	UCB0CTL1 |= UCSSEL_2;		//SMCLK
	UCB0BR0 = 128;		//fSCL = SMCLK/128 = 125khz
	UCB0BR1 = 0;		
	UCB0I2CSA = 0xAA;	//slave address 0xAA write, 0xAB read
	UCB0CTL1 &= ~(UCSWRST);
	UCB0IE |= UCRXIE + UCTXIE;	//recieve, transmit interrupt enable
}

void i2c_put(char *tx_buf, char num_byte){	//pointer to TX data
	
	p_tx_data = (unsigned char *) tx_buf;	//pass pointer
	tx_cnt = num_byte;	//number of bytes to transmit
	
	UCB0CTL1 |= UCTR + UCTXSTT;	//i2c TX, start condition
	while(UCB0CTL1 & UCTXSTP);	//ensure stop condition was sent
}

//call in while loop until 0 is received
char i2c_get(void){
	delay_us(10);
	char temp;
	if(rx_back != rx_front){
		p_rx_data = (unsigned char*) rx_buff;
		temp = rx_buff[rx_back];
		rx_back++;
		if(rx_back > RX_BUFF_SIZE){
			rx_back = 0;
		}
		return temp;
	}
	return 0;
}

#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void){
	switch(__even_in_range(UCB0IV, 12)){
		case 0: break;
			//no pending interrupt
		case 2: break;
			//Arbitration lost; UCALIFG
		case 4: break;
			//NACK
		case 6: break;
			//Start condition received
		case 8: break;
			//stop condition received
		case 10:	//data received 
		*p_rx_data++ = UCB0RXBUF;		//get RX'd byte into buffer
		rx_front++;
		if(rx_front > RX_BUFF_SIZE){
			rx_front = 0;
		}
		break; 
		
		case 12:	//transmit buffer empty 
		if(tx_cnt){
			UCB0TXBUF = *p_tx_data++;	//load TX buffer
			tx_cnt--;			//dec TX byte counter
		}
		else{
			UCB0CTL1 |= UCTXSTP;	//i2c stop condition
			UCB0IFG &= ~UCTXIFG;	//clear USCI_B0 TX int flag
		}
		break;
		
		default: break;	
	}
}