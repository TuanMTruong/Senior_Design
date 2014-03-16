/* Author: Garrett Clay
Date: 3.1.2014
Brief: Allow for PWM communication from MSP to EPD.
*/

#include "msp430.h"
#include "pwm.h"

volatile char pwm_status = 0;


void pwm_on(char state){
	if (state) {
		pwm_status = true;
	}
	else {
		pwm_status = false;
	}
}


//timer used for PWM
void pwm_init(void){
	TA0CCTL0 = CCIE;	//CCR0 interrupt enable
	TA0CCR0 = 40;		//set pwm cycle to 200KHz
	TA0CTL = TASSEL_2 + MC_1 + TACLR;	// SMCLK, upmode, clear TAR
	pwm_on(false);
}



// Timer0 A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
	if (pwm_status){
		PWMOUT ^= PWM_PIN;		// Toggle P1.1
	}
	else {
		PWMOUT &= ~(PWM_PIN);
	}
}

