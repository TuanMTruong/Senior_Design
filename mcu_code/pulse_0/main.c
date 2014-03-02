/* Author: Tuan Truong & Garrett Clay
 * Date: 3.1.2014
 * Brief: Senior design project '13-'14. MSP430F5528 is microcontroller used
 * 		for the wearable heart monitoring watch. Microcontroller uses
 * 		spi, usart, i2c, ADC, pwm, etc. to control e-ink display from 
 * 		pervasive display, pulse sensor, communication with bluetooth
 * 		etc.
 */

/* Hardware Description:
* Ext. Interface	Port #		I/O to MSP
  --------------	------		----------
 * DISP_PWM 		P1.1		O
 * BUTTONS 		P2.0-2.3	I
 * LED 			P2.4-2.7	O
 * EPD_CHIP_SEL 	P6.2		O
 * EPD_PANEL_ON 	P6.3		O
 * EPD_BUSY		P6.4		I
 * EPD_RESET 		P6.5		O
 * EPD_BORDER 		P6.6		O
 * EPD_DISCHARGE	P6.7		O
 *
 *
 *
 */

/* ERRATA
 * 1. change delay functions to use timers/interrupts to save power/cpu time
 *
 */

/*includes*/
#include <msp430.h>
#include "pwm.h"
#include "spi.h"
#include "clock.h"
#include "usart.h"

/*macros*/
#define LEDOUT 	P2OUT
#define LED0 	0x80
#define LED1 	0x10
#define LED2 	0x20
#define LED3 	0x40
#define LED_ALL	0xF0
#define true 	1
#define false 	0



volatile char temp = 0;

void ddr_init(void){
	P2DIR |= 0xFF;
	P1DIR |= 0xFF;
	P6DIR |= 0xD6; 	//P6.5-P6.7 &  output to disp.
	P6DIR &= ~BIT4;	//P6.4 disp. busy
}

void led_startup(void){
	LEDOUT &= 0x0F;
	__delay_cycles(2500000);
	LEDOUT = LED0;
	__delay_cycles(2500000);
	LEDOUT = LED1;
	__delay_cycles(2500000);
	LEDOUT = LED2;
	__delay_cycles(2500000);
	LEDOUT = LED3;
	__delay_cycles(2500000);
	LEDOUT = 0x00;
	__delay_cycles(2500000);
	LEDOUT = 0xF0;
	__delay_cycles(2500000);
	LEDOUT = 0x00;
	__delay_cycles(2500000);
	LEDOUT = 0xF0;
	__delay_cycles(2500000);
	LEDOUT = 0x00;
	
}



int main( void )
{
	// Stop watchdog timer to prevent time out reset
	WDTCTL = WDTPW + WDTHOLD;
	
	ddr_init();	//set ddr for LED 
	clock_init();	//set clock to 16MHz
	timer_init();	//set timer for PWM
	spi_init();	//set SPI
	usart_init();
	led_startup();	//run LED start up pattern
	
	P1DS &= ~BIT1;		//set pin strength
	
	pwm_on(true);
	spi_put(0x80);
	spi_put(0xA1);
	
	spi_put(0xF6);
	usart_put(0xAB);
	usart_put(0xAB);
	usart_put(0xAB);
	usart_put(0xAB);
	usart_put(0xAB);
	usart_put(0xAB);
	usart_put(0xAB);
	
	__bis_SR_register(LPM0_bits + GIE);	// Enter LPM0, enable interrupts
	__no_operation();			// For debugger
}


	       
			       
			       
			       
			       
			       