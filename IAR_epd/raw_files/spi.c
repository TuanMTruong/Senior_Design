/* Author: Garrett Clay
   Date: 2.19.2014
   Brief: Allow for SPI communication to/from MSP430 F5528.
*/

//include 
#include "msp430.h"

//macros
#define SPI_STE 0
#define SPI_MOSI 1
#define SPI_MISO 2
#define SPI_CLK 3

void dir_setup(void){
  P4DIR |= (1<<SPI_STE) | (1<<SPI_MOSI) | (1<<SPI_CLK);
  P4DIR &= ~(1<<SPI_MISO);
}

void spi_setup(void){
  P4SEL |= BIT1+BIT2+BIT3;
  //enable software reset
  UCB1CTL1 |= UCSWRST;
  
  //sample on rising edge, MSB, 3 wire asynchronous SPI
  UCB1CTL0 |= UCMSB | UCMST;
  //clk select
  UCB1CTL1 |= UCSSEL_2;
  //bit prescale TODO: look into this -Tuan
  UCB1BR0 = 0xFF;
  UCB1BR1 = 0;
  
  UCB1CTL1 &= ~UCSWRST;
  UCB1IE |= UCRXIE;
  
  return;
}

void spi_send(char data){
  UCB1TXBUF = data;
  LPM0;
  while((UCB1STAT & UCBUSY));
  return;
}

