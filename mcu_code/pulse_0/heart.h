/* Author:      Garrett Clay
 * Date:        3.8.2014
 * Brief:       Calculating heart rate from pulse sensor using MSP430 F5528.
*/

#include "msp430.h"

#define FALSE 	0
#define TRUE 	1

char hr_ready(int check);
char get_hr(void);
void hr_timer_init(void);
char get_btn();
