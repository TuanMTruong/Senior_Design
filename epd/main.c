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
#define F_CPU 16000000UL

//includes
#include<avr/io.h>
#include<util/delay.h>
#include<stdlib.h>
#include<stdio.h>

//hardware macros
#define CS_PIN		0	//PB0
#define SCK_PIN		1	//PB1
#define MOSI_PIN	2	//PB2
#define MISO_PIN	3	//PB3
#define BORDER_PIN	4	//PB4
#define PWM_PIN		5	//PB5
#define vcc		6	//PB6
#define BUSY_PIN	7	//PB7

#define RX_PIN		2	//PD2
#define TX_PIN		3	//PD3
#define RESET_PIN	4	//PD4


//protocol macros
#define REG_HEADER	0x70	//send before sending reg. index
#define DATA_HEADER	0x72	//send before sending reg. data

#define IN_DATA	0x00 //0x00000000000FFF00
#define IN2_DATA	0x00 //0x0000000001FFE000

#define PWM_DISABLE()	TCCR1B &= ~(7<<CS10)
#define PWM_ENABLE()	TCCR1B |= (2<<CS10)


#define EPD_ROW		96
#define EPD_COLUMN	76



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
	DDRA = 0x00;                //buttons are inputs
	DDRB |= (1<<CS_PIN) | (1<<SCK_PIN) | (1<<MOSI_PIN);
	DDRB &= ~(1<<MISO_PIN);
	DDRB |= (1<<PWM_PIN);       //set OC1A output for PWM
	DDRC = 0xff;                //LEds are outputs
	DDRD |= (1<<TX_PIN);        //usart tx is output
	DDRD &= ~(1<<RX_PIN);       //usart rx is input
	return;
}


/******************************************************************/
//SPI in master mode, max clk speed = 12MHz, min clk speed = 4MHz
//mode 0
/******************************************************************/
void spi_setup(void){
	SPCR = (1<<MSTR) | (1<<SPIE);   //master, MSB first, enable, 8MHz
	SPSR = (1<<SPI2X);              //double spi clock speed

	return;
}


/******************************************************************/
//100-300KHz 50% duty cycle PWM
//TIMER1 OC1A
//TODO change to true PWM mode. making our own pwm right now. 
/******************************************************************/
void pwm_setup(void){
	TCCR1A = (3<<COM1A0);   //Enable OC1A pin
	TCCR1B = (1<<WGM12);    //making our own pwm
	TCCR1B &= ~(7<<CS10);   //set up clock select
	TCCR1B |= (2<<CS10);		

	OCR1A = 10;             //set compare value
	TIMSK1 = (1<<OCIE1A);   //enable timer interrupt

	return;
}


/******************************************************************/
//Sending one byte over SPI
/******************************************************************/
void spi_sendbyte(uint8_t data){
	SPDR = data;                //send data
	while(!(SPSR & (1<<SPIF))); //wait till done
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




/******************************************************************/
//Check to see if COG is busy
//if 1 = BUSY else ready to receive new data
/******************************************************************/
uint8_t COG_busy(){
	return (PORTB & (1<<BUSY_PIN));
}


/******************************************************************/
//Sending byte of Data to COG
//Takes the index of which register to edit and data
//Procedure:
//1. set CS low
//2. send REG_HEADER
//3. send reg_index
//4. toggle CS high then low
//5. send DATA_HEADER
//6. send data
//7. set CS high
/******************************************************************/
void COG_sendbyte(uint8_t reg_index, uint8_t data){
	//1. pull CS_PIN low
	PORTB &= ~(1<<CS_PIN);

	//2. send header for data
	spi_sendbyte(REG_HEADER);

	//3. send reg_index
	spi_sendbyte(reg_index);

	//4. toggle CS
	PORTB |= (1<<CS_PIN);
	_delay_us(10);
	PORTB &= ~(1<<CS_PIN);

	//5. send DATA_HEADER
	spi_sendbyte(DATA_HEADER);

	//6. send data
	spi_sendbyte(data);

	//7. set CS_PIN high
	PORTB |= (1<<CS_PIN);

	return;
}

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
//Sending an array of data to a register on COG
//This function is used for when a COG register needs more than an 
//byte.
//Give register index and the array to fill register and the length
//of array.
//Procedure:
//1. set CS low
//2. send REG_HEADER
//3. send reg_index
//4. toggle CS high then low
//5. send DATA_HEADER
//6. send array
//7. set CS high
/******************************************************************/
void COG_sendArray(uint8_t reg_index, uint8_t *data, uint8_t num_bytes){
	//1. pull CS_PIN low
	PORTB &= ~(1<<CS_PIN);

	//2. send header for data
	spi_sendbyte(REG_HEADER);

	//3. send reg_index
	spi_sendbyte(reg_index);

	//4. toggle CS
	PORTB |= (1<<CS_PIN);
	_delay_us(10);
	PORTB &= ~(1<<CS_PIN);

	//5. send DATA_HEADER
	spi_sendbyte(DATA_HEADER);

	//6. send array
	spi_sendarray(data, num_bytes);

	//7. set CS_PIN high
	PORTB |= (1<<CS_PIN);

	return;
} 


/******************************************************************/
//made for testing
//creating a blank image buffer that is ready to be sent to the COG
//takes a pointer to a 2D array, data to fill that array, and scan byte
//2" display 200 x 96 pixel
/******************************************************************/
void fill_image_buff(uint8_t *buffer[EPD_ROW], uint8_t pixel_data, uint8_t scan_data){
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




/******************************************************************/
//powers on COG
//0. start with vcc/vdd, /reset, /cs, border, SI, SCLK = 0
//1. start pwm toggling
//2. vcc/vdd voltage > 2.7v 
//3. /cs = 1
//4. border = 1
//5. toggle reset 
/******************************************************************/
void COG_startup(){
	//1. Enable PWM
	PWM_ENABLE();
	_delay_ms(5);

	//2. Supply voltage
	PORTB |= (1<vcc);
	_delay_ms(10);

	//3. Set /cs = 1
	PORTB |= (1<<CS_PIN); 	

	//4. Set BORDER = 1
	PORTB |= (1<<BORDER_PIN);	

	//5. toggle reset
	PORTD |= (1<<RESET_PIN);
	_delay_ms(5);
	PORTD &= ~(1<<RESET_PIN);
	_delay_ms(5);
	PORTD |= (1<<RESET_PIN);
	_delay_ms(5);

	//POWER is one and the next step is to initialize COG
	return; 
}


void COG_init(){
	//COG_sendbyte(uint8_t reg_index, uint8_t data)
	//COG_sendbyte(uint8_t reg_index, uint64_t data, uint8_t num_bytes)
	//pass data as an address. ie &data
	uint8_t data;

	//Check if COG is busy
	while(BUSY_PIN);

	//Channel Select for EPD 2"
	//COG_sendarray(0x01, 2IN_DATA, 8);

	//set DC/DC Frequency 
	COG_sendbyte(0x06, 0xFF);

	//High power mode osc setting
	COG_sendbyte(0x07, 0x9D);

	//Disable ADC
	COG_sendbyte(0x08, 0x00);

	//Set Vcom Level
	//COG_sendbyte(0x09, 0xD000, 2);

	//Gate and source Voltage level
	//COG_sendbyte(0x04, 2IN_DATA, 8);
	_delay_ms(5);

	//Driver Latch on
	//Cancel register noise
	COG_sendbyte(0x03, 0x01);

	//Driver latch off
	COG_sendbyte(0x03, 0x00);

	//Start Charge pump positive Voltage
	//VGH & VDH enable (VGh>12V & VDH >8V)
	COG_sendbyte(0x05, 0x01);
	_delay_ms(30);

	//disable pwm
	PWM_DISABLE();

	//Start charge pump negative voltage
	//VGL < -12v & VDL < -8V
	COG_sendbyte(0x05, 0x03);
	_delay_ms(30);

	//Set charge pump Vcom_Driver to ON
	COG_sendbyte(0x05, 0x03);
	_delay_ms(30);

	//output enable to disable
	COG_sendbyte(0x02, 0x24);

	//if all went well the COG should be initialized and ready for image 
	//data. if not... get ready to debug ;)
	return;
}

//after COG is initialized began writing data from buffer to COG to be drawn
void write_cog(uint8_t *buffer){
	//keeps track of which line is being written to
	uint8_t line_counter = 0;

	//start charge pump
	//SPI_sendpacket(REG_HEADER, 0x04);
	_delay_us(10);
	//SPI_sendpacket(DATA_HEADER, 0x03);
	_delay_us(10);

	//send out each line, total of 97 lines
	for(line_counter = 0; line_counter<97; line_counter++){


	}
}


//Adventure is upon us...
int main(){
	//initialize
	ddr_setup();
	spi_setup();
	pwm_setup();

	//disable pwm until COG power on state
	PWM_DISABLE();
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
			case COG_INIT:
				break;
			case WRITE_EPD:
				break;
			case CHECK_EPD:
				break;
			case COG_OFF:
				break;
			default:
				break;

		}
	}	
	return 0;
}

//you shall not pass...
