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


//includes
#include<avrio.h> 
#include<util/delay.h>

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


void setup_spi(){

}



int main(){

	return 0;
}

//you shall not pass...
