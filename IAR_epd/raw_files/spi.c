/* Author: Garrett Clay
   Date: 2.19.2014
   Brief: Allow for SPI communication to/from MSP430 F5528.
*/

//include 
#include "msp430.h"
#include "spi.h"
#include "epd.h"

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

void SPI_on(){
  spi_setup();
  SPI_put(0x00);
  SPI_put(0x00);
  __delay_cycles(us_delay*10);
}

void SPI_off(){
  SPI_put(0x00);
  SPI_put(0x00);
  __delay_cycles(us_delay*10);
  UCB1IE &= ~UCRXIE;
}  

void SPI_put(char data){
  UCB1TXBUF = data;
  LPM0;
  while((UCB1STAT & UCBUSY));
  return;
}

void SPI_put_wait(char c, int busy_pin){
  SPI_put(c);
  //wait for COG ready
  while(P6IN && busy_pin){}
}

void SPI_send(char cs_pin, const char *buffer, long length){
  //CS low
  P6OUT &= ~(1<<cs_pin);
  
  //send all data
  for(long i = 0; i < length; i++){
    SPI_put(*buffer++);
  }
  
  //CS high
  P6OUT |= (1<<cs_pin);
}

void PWM_start(int pin){
  //TODO: set pwm at 50% duty cycle
  TA0CCR0 = 12000;      //PWM period, 12000 ACLK ticks or 1/second
  TA0CCR1 = 6000;       //PWM duty cycle, time cycle on vs off, on 50%
  TA0CCTL1 = OUTMOD_7;  //TA0CCR1 reset/set -- high voltage below TA0CCR1 count
                        //and low voltage when past
  TA0CTL = TASSEL_1 + MC_1;     //Timer A control set to submain clock TASSEL_1
                                //ACLK and count up to TA0CCR0 mode MC_1
}

void PWM_stop(int pin){
  //TODO: turn off pwm, not sure setting TA0CCR1 to zero work
  TA0CCR1 = 0;          //on 0% of time
}