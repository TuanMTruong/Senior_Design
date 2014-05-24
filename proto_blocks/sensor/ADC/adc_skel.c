// adc_skel.c 
// A simple voltmeter with the ADC operating in 10 bit mode
// No interrupts, coarse ADC accuracy, no quiet mode.

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL //16Mhz clock
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "lcd_functions.h"           //my 2x16 lcd routines

uint8_t i;
char lcd_str_h[16];  //holds string to send to lcd  
char lcd_str_l[16];  //holds string to send to lcd  
div_t fp_adc_result, fp_low_result; //double fp_adc_result; 

/*******************************************************/
void spi_init(void){
 /* Run this code before attempting to write to the LCD.*/
 DDRF  |= 0x08;  //port F bit 3 is enable for LCD
 PORTF &= 0xF7;  //port F bit 3 is initially low
 DDRB  |= 0x07;  //Turn on SS, MOSI, SCLK
 PORTB |= 0x01;  //port B initalization for SPI, SS_n off
 //Master mode, Clock=clk/4, Cycle half phase, Low polarity, MSB first
 SPCR=0x50;
 SPSR=0x00;
 }
/*******************************************************/

int main() {

uint16_t adc_result;  //holds the result from the ADC

//initalize the SPI port and then the LCD
spi_init();
lcd_init(); 
clear_display();

//Initalize ADC and its ports
DDRF &= ~(1<<PF7); //port F bit 7 is ADC input  
PORTF |= (1<<PF7); //poft F bit 7 pullups are off
DDRB |= (1<<PB7);  //output to reset transistor

ADMUX |= (1<<REFS0) | (1<<MUX2) | (1<<MUX1) | (1<<MUX0);    //single-ended port F bit 7, right adjusted for 10 bits
ADCSRA |= (1<<ADEN) | (1<<ADPS2) | (1<< ADPS1) | (1<<ADPS0);    //ADC enabled, don't start yet, single shot mode 
                  //division factor is 128 (125khz)

while(1){ 

  ADCSRA |= (1<<ADSC);                     //poke ADSC and start conversion
  while(bit_is_clear(ADCSRA, ADIF));  //spin while interrupt flag not set
  ADCSRA |= (1<<ADIF);              //its done, clear flag by writing a one 
  adc_result = ADC;                   //read the ADC output as 16 bits

  //div() function computes the value num/denom and returns the quotient and
  //remainder in a structure called div_t that contains two members, quot and rem. 
  
  //now determine Vin, where Vin = (adc_result/204.8)
  fp_adc_result = div(adc_result, 205);              //do division by 205 (204.8 to be exact)
//  itoa(fp_adc_result.quot, lcd_str_h, 10);           //convert non-fractional part to ascii string
  fp_low_result = div((fp_adc_result.rem*100), 205); //get the decimal fraction into non-fractional form 
//  itoa(fp_low_result.quot, lcd_str_l, 10);           //convert fractional part to ascii string

  if(fp_adc_result.quot >= 4 && fp_low_result.quot > 90){
	PORTB |= (1<<PB7);
  }
  else{
	PORTB &= ~(1<<PB7);
  }
 

  //send string to LCD
 // string2lcd(lcd_str_h);  //write upper half
 // char2lcd('.');          //write decimal point
 // string2lcd(lcd_str_l);  //write lower half

//  for(i=0;i<=10;i++){ _delay_ms(10);}  //delay 0.1 sec
 // clear_display();
 // cursor_home();
  } //while
}//main
