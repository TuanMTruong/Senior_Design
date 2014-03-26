/* Author: Tuan Truong & Garrett Clay
   Date: 3.2.2014
   Brief: Use ADC to determine heart rate from pulse sensor.
*/

#include "msp430.h"

#define NUM_OF_RESULTS	2	//CHECK: index in adc.h matches 

void adc_init(void);
int get_adc(void);