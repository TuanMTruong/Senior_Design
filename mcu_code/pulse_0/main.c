/* Author: Tuan Truong
 * test code for pulse msp430f5528 project
 */

/* Hardware Description:
 * 
 * DISP_PWM = P1.1
 * BUTTONS = P2.0-2.3
 * LED = P2.4-2.7
 * CHIP_SEL = P6.2
 * PANEL_ON = P6.3
 * BUSY	= P6.4
 * RESET = P6.5
 * BORDER = P6.6
 * DISCHARGE P6.7
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

/*macros*/

#define LEDOUT 	P2OUT
#define LED0 	0x80
#define LED1 	0x10
#define LED2 	0x20
#define LED3 	0x40
#define LED_ALL	0xF0
#define PWMOUT 	P1OUT
#define PWM_PIN 0x02
#define true 	1
#define false 	0
#define S_DELAY	16000000
#define MS_DELAY 16000
#define US_DELAY 16


#define USART_TX	0x10
#define USART_RX	0x20


volatile char pwm_on = 0;
volatile char temp = 0;

void ddr_init(void){
	P2DIR |= 0xFF;
	P1DIR |= 0xFF;
	P6DIR |= 0xD6; 	//P6.5-P6.7 &  output to disp.
	P6DIR &= ~BIT4;	//p6.4 disp. busy
}

void clock_init(void){
	UCSCTL3 = SELREF_2;		//set DCO FLL reference = REFO
	UCSCTL4 |= SELA_2;		//set ACLK = REFO
	UCSCTL0 = 0x0000;		//set lowest possible DC)x, MODx
	
	do
	{
		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
		// Clear XT2,XT1,DCO fault flags
		SFRIFG1 &= ~OFIFG;	// Clear fault flags
	}while (SFRIFG1&OFIFG);		// Test oscillator fault flag
	
	__bis_SR_register(SCG0);	//disable FLL control loop
	UCSCTL1 = DCORSEL_5;		//select DCO range 16mhz operation
	UCSCTL2 |= 487;			//set DCO multiplier for 16mhz
	//(N+1) * FLLRef = Fdco
	//(487 + 1) * 32768 = 16mhz
	__bic_SR_register(SCG0);	//enable the FLL control loop
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

/*
void wait_s(int time){
	__delay_cycles(s_delay*time);
}

void wait_us(int time){
	__delay_cycles(us_delay*time);
}

void wait_ms(int time){
	__delay_cycles(ms_delay*time);
}
*/

int main( void )
{
	// Stop watchdog timer to prevent time out reset
	WDTCTL = WDTPW + WDTHOLD;
	
	ddr_init();
	clock_init();
	timer_init();
	led_startup();
	P1DS &= ~BIT1;		//set pin strength
	pwm_on = true;
	
	
	__bis_SR_register(LPM0_bits + GIE);	// Enter LPM0, enable interrupts
	__no_operation();			// For debugger
}


// Timer0 A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
	if (pwm_on){
		PWMOUT ^= PWM_PIN;		// Toggle P1.1
	}
	else {
		PWMOUT &= ~(PWM_PIN);
	}
}
