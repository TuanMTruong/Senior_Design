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

//Clock speed
#define F_CPU 16000000UL

//includes
#include<avr/io.h>
#include<util/delay.h>
#include<stdlib.h>
#include<stdio.h>


//global variables
enum states {CREATE_IMG, WRITE_MEM, COG_ON,COG_INIT, WRITE_EPD, CHECK_EPD, COG_OFF};
enum states state = CREATE_IMG;

uint8_t test_buffer_blk[5000];
uint8_t test_buffer_wht[5000];
uint8_t test_buffer_nth[5000];


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

//#define CS_PIN		0	//delete line, repeated
//#define ID_PIN		2	//TODO ask Tuan what this is for?

//protocol macros
#define REG_HEADER	0x70	//send before sending reg. index
#define DATA_HEADER	0x72	//send before sending reg. data

#define 1_4IN_DATA	0x00000000000FFF00
#define 2IN_DATA	0x0000000001FFE000

#define PWM_DISABLE()	TCCR1B &= ~(7<<CS10)
#define PWM_ENABLE()	TCCR1B |= (2<<CS10)

//set up the data direction registers (1=output, 0=input)
void setup_ddr(){
	//buttons are inputs
	//1 = output, 0 = input
	DDRA = 0x00;
	//output spi
	DDRB |= (1<<CS_PIN) | (1<<SCK_PIN) | (1<<MOSI_PIN);
	//input for spi
	DDRB &= ~(1<<MISO_PIN);
	//set OC1A output for PWM
	DDRB |= (1<<PWM_PIN);
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
//TIMER1 OC1A
//TODO change to true PWM mode. making our own pwm right now. 
void setup_pwm(){
	//Clear out register
	TCCR1A = 0x00;
	//Enable OC1A pin
	TCCR1A |= (3<<COM1A0);
	//Set waveform mode
	//mode 4 pwm CTC, top OCR1A, update at bottom
	//TCCR1A |= (1<<WGM10);
	TCCR1B |= (1<<WGM12);		//making our own pwm

	//Set timer clock sel clk/8
	//16mhz / 8 = 2mhz
	//2mhz / (OCR1A = 10) = 200khz
	TCCR1B &= ~(7<<CS10);    	
	TCCR1B |= (2<<CS10);		
                                        
	//set compare value
	OCR1A = 10;

	//enable timer interrupt
	TIMSK1 = (1<<OCIE1A);    
	
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

//Sending SPI byte of data
//1. set CS low
//2. send REG_HEADER
//3. send reg_index
//4. toggle CS high then low
//5. send DATA_HEADER
//6. send data
//7. set CS high
void SPI_byte(uint8_t reg_index, uint8_t data){
	//1. pull CS_PIN low
	PORTB &= ~(1<<CS_PIN);
    
	//2. send header for data
	SPI_sendbyte(REG_HEADER);

	//3. send reg_index
	SPI_sendbyte(reg_index);

	//4. toggle CS
	PORTB |= (1<<CS_PIN);
	_delay_us(10);
	PORTB &= ~(1<<CS_PIN);

	//5. send DATA_HEADER
	SPI_sendbyte(DATA_HEADER);

	//6. send data
	SPI_sendbyte(data);

	//7. set CS_PIN high
	PORTB |= (1<<CS_PIN);
	_delay_us(10);		//wait 10us between packets

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

//Sending SPI packet of data
//1. set CS low
//2. send REG_HEADER
//3. send reg_index
//4. toggle CS high then low
//5. send DATA_HEADER
//6. send data
//7. set CS high
void SPI_bytes(uint8_t reg_index, uint64_t data, uint8_t num_bytes){
	//1. pull CS_PIN low
	PORTB &= ~(1<<CS_PIN);
    
	//2. send header for data
	SPI_sendbyte(REG_HEADER);

	//3. send reg_index
	SPI_sendbyte(reg_index);

	//4. toggle CS
	PORTB |= (1<<CS_PIN);
	_delay_us(10);
	PORTB &= ~(1<<CS_PIN);

	//5. send DATA_HEADER
	SPI_sendbyte(DATA_HEADER);

	//6. send data
	//TODO review with Tuan.
	//check for busy pin between data bytes?
	for(i=0;i<num_bytes;i++)
		SPI_sendbyte((data & (0xFF<<(i*8)))>>(i*8));
	}

	//7. set CS_PIN high
	PORTB |= (1<<CS_PIN);
	_delay_us(10);		//wait 10us between packets

	return;
} 


//made for testing
//creating a blank image buffer
//2" display 200 x 96 pixel
void fill_image_buff(uint8_t *buffer){
	uint8_t data_counter = 0;
	uint8_t pixel_data = 0xFF; //TODO are pixels 1&0 or byte value?
	uint8_t scan_counter = 0;
	uint8_t line_counter= 0;

	//for 2" EPD resolution 200 x 96
	//draw 97 lines with 200 pixels
	for(line_counter = 0; line_counter<96; line_counter++){
		//set the even bits of image
		for(data_counter = 0; data_counter<25; data_counter++){
			*buffer = pixel_data;
			buffer++;
		}
	
		//set the scan bits
		for(scan_counter = 0; scan_counter<24 ; scan_counter++){
			*buffer= 0xFF;
			buffer++;
		}

		//set the odd bits of image
		for(data_counter = 25; data_counter<50; data_counter++){
			*buffer = pixel_data;
			buffer++;
		}
		
		//TODO why didn't we fill to 200? stopped at 99

		//blank byte
		*buffer = 0x00;
		buffer++;
	}

	return;
}
               
//powers on COG
//0. start with vcc/vdd, /reset, /cs, border, SI, SCLK = 0
//1. start pwm toggling
//2. vcc/vdd voltage > 2.7v 
//3. /cs = 1
//4. border = 1
//5. toggle reset 
void startup_cog(){
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


void init_cog(){
	//SPI_byte(uint8_t reg_index, uint8_t data)
	//SPI_bytes(uint8_t reg_index, uint64_t data, uint8_t num_bytes)
	//pass data as an address. ie &data
	uint8_t data;

	//Check if COG is busy
	while(BUSY_PIN);

	//Channel Select for EPD 2"
	SPI_bytes(0x01, 2IN_DATA, 8);

	//set DC/DC Frequency 
	SPI_byte(0x06, 0xFF);

	//High power mode osc setting
	SPI_byte(0x07, 0x9D);

	//Disable ADC
	SPI_byte(0x08, 0x00);

	//Set Vcom Level
	SPI_bytes(0x09, 0xD000, 2);

	//Gate and source Voltage level
	SPI_bytes(0x04, 2IN_DATA, 8);
	_delay_ms(5);

	//Driver Latch on
	//Cancel register noise
	SPI_byte(0x03, 0x01);

	//Driver latch off
	SPI_byte(0x03, 0x00);
	
	//Start Charge pump positive Voltage
	//VGH & VDH enable (VGh>12V & VDH >8V)
	SPI_byte(0x05, 0x01);
	_delay_ms(30);

	//disable pwm
	PWM_DISABLE();

	//Start charge pump negative voltage
	//VGL < -12v & VDL < -8V
	SPI_byte(0x05, 0x03);
	_delay_ms(30);

	//Set charge pump Vcom_Driver to ON
	SPI_byte(0x05, 0x03);
	_delay_ms(30);

	//output enable to disable
	SPI_byte(0x02, 0x24);

	//if all went well the COG should be initialized and ready for image 
	//data. if not... get ready to debug ;)
	return;
}

//after COG is initialized began writing data from buffer to COG to be drawn
void write_cog(uint8_t *buffer){
	//keeps track of which line is being written to
	uint8_t line_counter = 0;

	//start charge pump
	SPI_sendpacket(REG_HEADER, 0x04);
	_delay_us(10);
	SPI_sendpacket(DATA_HEADER, 0x03);
	_delay_us(10);

	//send out each line, total of 97 lines
	for(line_counter = 0; line_counter<97; line_counter++){


	}
}


//Adventure is upon us...
int main(){
	//initialize
	setup_ddr();
	setup_spi();
	setup_pwm();
   	
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
