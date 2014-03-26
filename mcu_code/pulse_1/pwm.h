/* Author: Garrett Clay
Date: 3.1.2014
Brief: Allow for PWM communication from MSP to EPD.
*/

#include "msp430.h"

#define PWMOUT 	P1OUT
#define PWM_PIN 0x02
#define true 	1
#define false 	0


void pwm_init(void);
void pwm_on(char state);
