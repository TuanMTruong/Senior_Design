/* Author: Tuan Truong & Garrett Clay
Date: 3.2.2014
Brief: Use ADC to determine heart rate from pulse sensor.
*/

#include "msp430.h"
#include "adc.h"



volatile unsigned int results[NUM_OF_RESULTS];	//needs to stay global

int get_adc(void){
	static int i = 0;
	int data_point = results[i];
	//i++;
	if(i == NUM_OF_RESULTS){i=0;}
	return data_point;
}

void adc_init(void){
	ADC12CTL0 = ADC12ON+ADC12SHT0_8+ADC12MSC; // Turn on ADC12, set sampling time
	// set multiple sample conversion
	ADC12CTL1 = ADC12SHP+ADC12CONSEQ_2;       // Use sampling timer, set mode, ACLK, divider
	ADC12IE = 0x01;                           // Enable ADC12IFG.0
	ADC12CTL0 |= ADC12ENC;                    // Enable conversions
	ADC12CTL0 |= ADC12SC;                     // Start conversion	
}


#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR (void)
{
	static unsigned char index = 0;
	
	switch(__even_in_range(ADC12IV,34))
	{
		case  0: break;                           // Vector  0:  No interrupt
		case  2: break;                           // Vector  2:  ADC overflow
		case  4: break;                           // Vector  4:  ADC timing overflow
		case  6:                                  // Vector  6:  ADC12IFG0
		results[index] = ADC12MEM0;             // Move results
		index++;                                // Increment results index, modulo; Set Breakpoint1 here
		
		if (index == NUM_OF_RESULTS){
			index = 0;
		}
		//P6OUT ^= BIT1;
		
		case  8: break;                           // Vector  8:  ADC12IFG1
		case 10: break;                           // Vector 10:  ADC12IFG2
		case 12: break;                           // Vector 12:  ADC12IFG3
		case 14: break;                           // Vector 14:  ADC12IFG4
		case 16: break;                           // Vector 16:  ADC12IFG5
		case 18: break;                           // Vector 18:  ADC12IFG6
		case 20: break;                           // Vector 20:  ADC12IFG7
		case 22: break;                           // Vector 22:  ADC12IFG8
		case 24: break;                           // Vector 24:  ADC12IFG9
		case 26: break;                           // Vector 26:  ADC12IFG10
		case 28: break;                           // Vector 28:  ADC12IFG11
		case 30: break;                           // Vector 30:  ADC12IFG12
		case 32: break;                           // Vector 32:  ADC12IFG13
		case 34: break;                           // Vector 34:  ADC12IFG14
		default: break; 
	}
}
