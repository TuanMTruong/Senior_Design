/******************************************************************/
/* main.c
 * authors: Tuan & Garrett
 * date: 11/21/13
 * mcu: mega128
 * brief: test code for 2.0" TFT EPD Panel SG020AS0T1.
 * Objective to construct an image on the EPD using SPI
 * and determine if EPD is easy to use and EPD should be 
 * implemented in project.
 *
 * hardware:
 * - Buttons	PA0-PA7		input to MCU                    
 * - SPI_CS	PB0		output from MCU
 * - SPI_SCK	PB1		output from MCU
 * - SPI_MOSI	PB2		output from MCU
 * - SPI_MISO	PB3		input to MCU
 * - BORDER_PIN	PB4		output from MCU
 * - PWM_OC1A	PB5		output from MCU
 * - vcc	PB6		output from MCU
 * - BUSY_PIN	PB7		input to MCU
 *
 * - LEDs	PC0-PC7
 * - USART_RX	PD2
 * - USART_TX	PD3
 * - RESET_PIN	PD4
 * - ID pin	gnd
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
/******************************************************************/

//Clock speed
#define F_CPU 16000000

//includes
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#include"usart.h"
#include"spi.h"
#include"COG.h"

//hardware macros
#define SCK_PIN		1	//PB1
#define MOSI_PIN	2	//PB2
#define MISO_PIN	3	//PB3
#define RX_PIN		2	//PD2
#define TX_PIN		3	//PD3

#define BAUD		38400	//usart speed




//global variables
enum states {CREATE_IMG, WRITE_MEM, COG_ON,COG_INIT, WRITE_EPD, CHECK_EPD, COG_OFF};
enum states state = CREATE_IMG;

uint8_t test_buffer_blk[EPD_COLUMN][EPD_ROW];
uint8_t test_buffer_wht[EPD_COLUMN][EPD_ROW];
uint8_t test_buffer_nth[EPD_COLUMN][EPD_ROW];


/******************************************************************/
// set up data direction for pins
// current settings are for AT90USB1287 
/******************************************************************/
void ddr_setup(void){
	DDRA = 0x00;                	//buttons are inputs
	DDRB |= (1<<CS_PIN) | (1<<SCK_PIN) | (1<<MOSI_PIN);
	DDRB &= ~(1<<MISO_PIN);
	DDRB &= ~(1<<BUSY_PIN);		//Busy pin is input
	DDRB |= (1<<PWM_PIN);       	//set OC1A output for PWM
	DDRB |= (1<<BORDER_PIN);	//control border
	DDRB |= (1<<VCC_PIN);
	DDRC = 0xff;                	//LEds are outputs
	DDRD |= (1<<TX_PIN);        	//usart tx is output
	DDRD &= ~(1<<RX_PIN);       	//usart rx is input
	DDRD |= (1<<RESET_PIN);
	return;
}





/******************************************************************/
//100-300KHz 50% duty cycle PWM
//TIMER1 OC1A
//TODO change to true PWM mode. making our own pwm right now. 
//TODO test pwm and verify 200KHz 
/******************************************************************/
void pwm_setup(void){
	TCCR1A = (1<<COM1A0);	//Enable OC1A pin on ever compare match
	TCCR1B = (1<<WGM12) | (1<<CS10);	//Enable CTC and no clock prescale


	OCR1A = PWM_SPEED;	//set compare value for 200KHz
	TIMSK = (1<<OCIE1A);   //enable timer interrupt

	return;
}




//TODO delete this
//delete if not using pointers to send data
//use for filling memory sequentially
/*
   void fill_pointer(uint8_t *data_ptr, uint64_t data, uint8_t num_bytes){
   uint8_t i;
   for(i=0;i<num_bytes;i++){
 *data_ptr = ((data & (0xFF<<(i*8)))>>(i*8));
 data_ptr++;
 }
 *data_ptr = NULL;
 }                
 */


/******************************************************************/
//made for testing
//creating a blank image buffer that is ready to be sent to the COG
//takes a pointer to a 2D array, data to fill that array, and scan byte
//2" display 200 x 96 pixel
/******************************************************************/
void fill_image_buff(uint8_t (*buffer)[EPD_ROW], uint8_t pixel_data, uint8_t scan_data){
	uint8_t row_counter = 0;	//counter for the rows on the image
	uint8_t column_counter= 0;	//counter for the columns on the image

	//loop through the rows
	for(row_counter = 0; row_counter< EPD_ROW; row_counter++){
		//fill in the even pixels of the image
		for(column_counter = 0; column_counter<25; column_counter++){
			buffer[row_counter][column_counter] = pixel_data;
		}
		//fill in scan byte
		for(column_counter = 25; column_counter<50; column_counter++){
			buffer[row_counter][column_counter] = scan_data;
		}
		//fill in odd pixels of the image
		for(column_counter = 50; column_counter<75; column_counter++){
			buffer[row_counter][column_counter] = pixel_data;
		}
		//blank byte
		buffer[row_counter][column_counter] = 0x00;
	}

	return;
}




ISR(TIMER1_COMPA_vect){
	TCNT1 = 0;
}

/******************************************************************/
//Adventure is upon us...
/******************************************************************/
int main(void){
	//initialize
	ddr_setup();
	spi_setup();
	pwm_setup();
	usart_setup(BAUD);

	_delay_ms(10);		//wait for everything to start up 
	usart_sendarray("SYSTEM STARTING\n", 16); 
	//disable pwm until COG power on state
	PWM_DISABLE();

	sei();			//enable global interrupt			
	state = CREATE_IMG;

	//loop, forever...
	while(1){
		switch(state){
			case CREATE_IMG:
				usart_sendarray("state = CREATE_IMG\n", 19);
				fill_image_buff(test_buffer_blk, 0xff, 0xff);
				state = WRITE_MEM;
				break;
			case WRITE_MEM:
				//break;
			case COG_ON:
				COG_startup();
				state = COG_INIT;
				break;

			case COG_INIT:
				COG_init();
				state = WRITE_EPD;
				break;
			case WRITE_EPD:
				COG_write(test_buffer_blk);
				state = COG_OFF;
				break;
			case CHECK_EPD:
				break;
			case COG_OFF:
				while(1){}
				//TODO turn off COG properly so display isn't damange
				break;
			default:
				break;

		}
	}
	return 0;
}

//you shall not pass...
