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
 * - PWM_OC1A	PB5
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

uint8_t test_buffer_blk[5000];
uint8_t test_buffer_wht[5000];
uint8_t test_buffer_nth[5000];

//hardware macros
#define CS_PIN		0
#define SCK_PIN		1
#define MOSI_PIN	2
#define MISO_PIN	3
#define RX_PIN		2
#define TX_PIN		3
#define PWM_PIN		5

#define CS_PIN		0
#define BUSY_PIN	1
#define ID_PIN		2
#define RESET_PIN	3
#define BORDER_PIN	4

//protocol macros
#define REG_HEADER	0x70
#define DATA_HEADER	0x72

#define PWM_DISABLE()	TCCR1B &= ~(7<<CS10)
#define PWM_ENABLE()	TCCR1B |= (2<<CS10)

//set up the data direction registers (1=output, 0=input)
void setup_ddr(){
	//buttons are inputs
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
void setup_pwm(){
	//Clear out register
	TCCR1A = 0x00;
	//Enable OC1A pin
	TCCR1A |= (3<<COM1A0);
	//Set waveform mode
	//mode 4 pwm CTC, top OCR1A, update at bottom
	//TCCR1A |= (1<<WGM10);
	TCCR1B |= (1<<WGM12);

	//Set timer clock sel clk/8
	TCCR1B = 0x00;
	TCCR1B |= (2<<CS10);

	//enable timer interrupt
	TIMSK1 = (1<<ICIE1);
		
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
//delay  >= 10us between packet
void SPI_sendpacket(uint8_t header, uint8_t *data){
	//pull CS_PIN low
	PORTB &= ~(1<<CS_PIN);
    
	//send header for data
	SPI_sendbyte(header);

	//send data
	while(*data){
		SPI_sendbyte(*data);
		data++;
	}

	//set CS_PIN high
	PORTB |= (1<<CS_PIN);

	return;
}



//made for testing
//creating a blank image buffer
//2" display 200 x 96 pixel
void fill_image_buff(uint8_t *buffer){
	uint8_t pixel = 200;
	uint8_t pixel_data = 0xff;
	uint8_t scan_counter = 0;
	uint8_t line_counter= 0

	//for 2" EPD resolution 200 x 97
	//draw 97 lines with 200 pixels
	for(line_counter = 0; line_counter<97; line_counter++){
		//set the even bits of image
		for(pixel = 200; pixel <2; pixel = pixel -2){
			*buffer = pixel_data;
			buffer++;
		}
	
		//set the can bits
		for(scan_counter = 0; scan_counter>96 ; scan_counter++){
			*buffer= 0xff;
			buffer++;
		}

		//set the odd bits of image
		for(pixel = 1; pixel < 199; pixel = pixel +2){
			*buffer = pixel_data;
			buffer++;
		}

		//blank byte
		*buffer = 0x00;
		buffer++;

	}

	return;
}

//powers on COG
void startup_cog(){
	//Enable PWM
	PWM_ENABLE();
	_delay_us(5);
	//Supply voltage
	
	//toggle PWM for >= 10ms 
	_delay_us(10);

	//Set /CS = 1
	PORTF |= (1<<CS_PIN); 	
	//Set BORDER = 1
	PORTF |= (1<<BORDER_PIN);	
	//Set /RESET = 1
	PORTF |= (1<<RESET_PIN);
	//toggle PWM for >= 5ms
	_delay_us(5);
	
	//Clear /RESET=0
	PORTF &= ~(1<<RESET_PIN);
	//toggle PWM for >= 5ms
	_delay_us(5);
	
	// Set /RESET = 1
	PORTF |= (1<<RESET_PIN);
	
	//toggle PWM for >= 5ms
	_delay_us(5);
	
	//POWER is one and the next step is to initialize COG
	return; 

}

void init_cog(){
	//Check if COG is busy
	while(BUSY_PIN);

	//Channel Select for EPD 2"
	SPI_sendpacket(REG_HEADER, 0x01);
	_delay_us(10);
	SPI_sendpacket(DATA_HEADER, 0x0000000001FFE000);
	_delay_us(10);

	//set DC/DC Frequency 
	SPI_sendpacket(REG_HEADER, 0x06);
	_delay_us(10);
	SPI_sendpacket(DATA_HEADER, 0xFF);
	_delay_us(10);

	//High power mode osc setting
	SPI_sendpacket(REG_HEADER, 0x07);
	_delay_us(10);
	SPI_sendpacket(DATA_HEADER, 0x9D);
	_delay_us(10);

	//Disable ADC
	SPI_sendpacket(REG_HEADER, 0x08);
	_delay_us(10);
	SPI_sendpacket(DATA_HEADER, 0x00);
	_delay_us(10);

	//Set Vcom Level
	SPI_sendpacket(REG_HEADER, 0x09);
	_delay_us(10);
	SPI_sendpacket(DATA_HEADER, 0xD000);
	_delay_us(10);

	//Gate and source Voltage level
	SPI_sendpacket(REG_HEADER, 0x04);
	_delay_us(10);
	SPI_sendpacket(DATA_HEADER, 0x03);
	_delay_us(10);

	//toggle PWM for >= 5ms
	_delay_ms(5);

	//Driver Latch on
	//Cancel register noise
	SPI_sendpacket(REG_HEADER, 0x03);
	_delay_us(10);
	SPI_sendpacket(DATA_HEADER, 0x01);
	_delay_us(10);

	//Driver latch off
	SPI_sendpacket(REG_HEADER, 0x03);
	_delay_us(10);
	SPI_sendpacket(DATA_HEADER, 0x00);
	_delay_us(10);
	
	//Start Charge pump positive Voltage
	//VGH & VDH enable (VGh>12V & VDH >8V)
	SPI_sendpacket(REG_HEADER, 0x05);
	_delay_us(10);
	SPI_sendpacket(DATA_HEADER, 0x01);
	_delay_us(10);

	//Let pwm toggle for >= 30ms
	_delay_ms(30);

	//disable pwm
	PWM_DISABLE();

	//Start charge pump negative voltage
	//VGL < -12v & VDL < -8V
	SPI_sendpacket(REG_HEADER, 0x05);
	_delay_us(10);
	SPI_sendpacket(DATA_HEADER, 0x03);
	_delay_us(10);

	//wait for >= 30ms
	_delay_ms(30);

	//Set charge pump Vcom_Driver to ON
	SPI_sendpacket(REG_HEADER, 0x05);
	_delay_us(10);
	SPI_sendpacket(DATA_HEADER, 0x0F);

	//wait for >= 30ms
	_delay_ms(30);

	//output enable to disable
	SPI_sendpacket(REG_HEADER, 0x02);
	_delay_us(10);
	SPI_sendpacket(DATA_HEADER, 0x24);
	_delay_us(10);

	//if all went well the COG should be initialized and ready for image data
	//if not... get ready to debug ;)
	return;
	
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
