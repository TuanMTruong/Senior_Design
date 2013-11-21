/* main.c
 * author: Tuan
 * date: 11/21/13
 * mcu: at90usb1287
 * brief: test code for 2.0" TFT EPD Panel SG020AS0T1.
 * Objective to construct an image on the EPD using SPI
 * and determine if EPD is easy to use and EPD should be 
 * implemented in project.
 *
 * hardware:
 * - Buttons	PA0-PA7
 * - SPI_CS	PB0
 * - SPI_SCK	PB1
 * - SPI_MOSI	PB2
 * - SPI_MISO	PB3
 * - LEDs	PC0-PC7
 * - USART_RX	PD2
 * - USART_TX	PD3
 *
 */

/* PLAN OF ATTACK:
 * In order to display something the COG (Chip On Glass) must be
 * used. Each time an image needs to be drawn the image must be 
 * loaded into 2 buffers, power on COG, initialize COG,
 * COG takes data from buffer and writes to EPD, check if writing
 * is completed, COG powers off, and repeat for next image.
 * The plan is to create a state machine and flows through the steps 
 * of drawing an image.
 */

//Clock speed
#define F_CPU 16000000UL

//includes
#include<avr/io.h>
#include<util/delay.h>


//global variables
enum states {CREATE_IMG, WRITE_MEM, COG_ON,COG_INIT, WRITE_EPD, CHECK_EPD, COG_OFF};
enum states state = CREATE_IMG;

//macros
#define CS_PIN		0
#define SCK_PIN		1
#define MOSI_PIN	2
#define MISO_PIN	3
#define RX_PIN		2
#define TX_PIN		3

//set up the data direction registers (1=output, 0=input)
void setup_ddr(){
	//buttons are inputs
	DDRA = 0x00;
	//output spi
	DDRB |= (1<<CS_PIN) | (1<<SCK_PIN) | (1<<MOSI_PIN);
	//input for spi
	DDRB &= ~(1<<MISO_PIN);
	//LEds are outputs
	DDRC = 0xff;
	//usart tx is output
	DDRD |= (1<<TX_PIN);
	//usart rx is input
	DDRD &= ~(1<<RX_PIN);

	return;
}


//SPI in master mode, max clk speed = 12MHz, min clk speed = 4MHz
//mode 0
void setup_spi(){
	//master, MSB first, enable
	//clk speed = 16MHz / 2 = 8MHz 
	SPCR = (1<<MSTR) | (1<<SPIE);
	//double spi clock speed
	SPSR = (1<<SPI2X);

	return;
}


//100-300KHz 50% duty cycle PWM
void setup_pwm(){

	return;
}


//Sending one byte over SPI
void SPI_sendbyte(uint8_t data){
	//send data
	SPDR = data;
	//wait till done
	while(!(SPSR & (1<<SPIF)));

	return;
}

//Sending SPI packet of data
void SPI_sendpacket(uint8_t *data){
	//pull CS_PIN low
	PORTB &= ~(1<<CS_PIN);

	while(*data){
		SPI_sendbyte(*data);
		data++;
	}

	//set CS_PIN high 
	PORTB |= (1<<CS_PIN);

	return;
}



//Adventure is upon us...
int main(){
	//initialize
	setup_ddr();
	setup_spi();
	setup_pwm();

	//variables
	
	//loop, forever...
	while(1){
		switch(state){
		case CREATE_IMG:
			break;
		case WRITE_MEM:
			break;
		case COG_ON:
			break;
		case COG_INIT;
			break;
		case WRITE_EPD:
			break;
		case CHECK_EPD:
			break;
		case COG_OFF:
			break;
		case defualt:
			break;

		}

	}
	
	return 0;
}

//you shall not pass...
