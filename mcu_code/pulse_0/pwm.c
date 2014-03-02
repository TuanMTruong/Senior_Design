/* Author: Garrett Clay
   Date: 3.1.2014
   Brief: Allow for PWM communication from MSP to EPD.
*/

#include "msp430.h"
#include "pwm.h"

//timer used for PWM
void timer_init(void){
	TA0CCTL0 = CCIE;	//CCR0 interrupt enable
	TA0CCR0 = 40;		//set pwm cycle to 200KHz
	TA0CTL = TASSEL_2 + MC_1 + TACLR;	// SMCLK, upmode, clear TAR
}



