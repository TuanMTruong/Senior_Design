/******************************************************************/
// Author: Tuan
// Date: 12/31/13
// has basic spi Function 
// TODO add Interrupt support
/******************************************************************/

//includes
#include<avr/io.h>
#include"spi.h"
#include"usart.h"


/******************************************************************/
//SPI in master mode, max clk speed = 12MHz, min clk speed = 4MHz
//mode 0
/******************************************************************/
void spi_setup(void){
	SPCR = (1<<MSTR) | (1<<SPE) | (1<<CPOL);   	//master, MSB first, enable, 8MHz
	SPSR = (1<<SPI2X);              		//double spi clock speed

	return;
}


/******************************************************************/
//Sending one byte over SPI
/******************************************************************/
void spi_sendbyte(uint8_t data){
	SPDR = data;                //send data
	while(!(SPSR & (1<<SPIF))){}; //wait till done
//	usart_sendbyte(data);
	return;
}

/******************************************************************/
// Sending an array over SPI
// Takes a pointer to an array and the number of bytes to send
/******************************************************************/
void spi_sendarray(uint8_t *array, uint8_t length){
	uint8_t i =0;
	for(i =0; i<length; i++){
		spi_sendbyte(*(array+i));
	}
}

