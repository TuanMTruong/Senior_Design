/* Authors: Tuan Truong & Garrett Clay
   Date: 3.1.2014
   Brief: Sets master clock (MCLK) for MSP430.
*/
//CPU freq. = 16mhz

#define S_DELAY	16000000
#define MS_DELAY 16000
#define US_DELAY 16

void clock_init(void);