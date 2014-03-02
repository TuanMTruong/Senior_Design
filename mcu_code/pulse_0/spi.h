/* Author: Garrett Clay
   Date: 3.1.2014
   Brief: Allow for SPI communication from MSP to EPD.
*/

#include "msp430.h"

#define SPI_STE 	0x01
#define SPI_MOSI	0x02
#define SPI_MISO	0x04
#define SPI_CLK		0x08

void spi_init(void);