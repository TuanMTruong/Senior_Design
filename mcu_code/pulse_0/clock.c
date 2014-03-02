/* Authors: Tuan Truong & Garrett Clay
   Date: 3.1.2014
   Brief: Sets master clock (MCLK) for MSP430. 

   CPU freq. = 16mhz
*/
#include "msp430.h"

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