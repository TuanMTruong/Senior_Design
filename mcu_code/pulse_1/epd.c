/* Author: Tuan Truong & Garrett Clay
Date: 3.3.2014
Brief: Pervasive display e-ink display chip on glass (COG) driver. Intended
for use with 1.44" display. 
*/

#include "msp430.h"
#include "epd.h"
#include "spi.h"
#include "pwm.h"
//#include "Images.h"  // this is just an empty file
#include "clock.h"

#include "numbers.xbm"
//macros        TODO: move to epd.h
#define false 0
#define true 1
#define EPD_PORT_OUT	P6OUT
#define EPD_PORT_IN	P6IN
#define CHIP_SELECT_PIN 2
#define PANEL_ON_PIN    3
#define BUSY_PIN        4
#define RESET_PIN       5
#define BORDER_PIN      6
#define DISCHARGE_PIN   7
//#define PWM_PIN         1

#define ULONG_MAX       4294967295UL

//#define ARRAY(type, ...) ((type[]){__VA_ARGS__})
//#define CU8(...) (ARRAY(const char, __VA_ARGS__))]

//display parameters
int factored_stage_time = 480;   //no temperature data
char lines_per_display = 96;
char dots_per_line = 128;
char bytes_per_line = 16;
char bytes_per_scan = 24;
char filler = false;

static char channel_select[] = {0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x00};
long channel_select_length = 9;
static char gate_source[] = {0x72, 0x03};
long gate_source_length = 2;
static char v_com_lvl[] = {0x72, 0xD0, 0x00};

unsigned char buf_write[1536];
//unsigned char buf_draw[1536];

char temp_buff[2];

char *fill_temp(char data1, char data2){
	temp_buff[0] = data1;
	temp_buff[1] = data2;
	return temp_buff;
}
//volatile int numbers[2][2];
void epd_begin(void){          
	//power up sequence
	EPD_PORT_OUT &= ~(1<<RESET_PIN);
	EPD_PORT_OUT &= ~(1<<PANEL_ON_PIN);
	EPD_PORT_OUT &= ~(1<<DISCHARGE_PIN);
	EPD_PORT_OUT &= ~(1<<BORDER_PIN);
	EPD_PORT_OUT &= ~(1<<CHIP_SELECT_PIN);
	
	spi_on();
	
	pwm_on(true);
	
	//delay_ms(5);
	__delay_cycles(MS_DELAY*5);
	
	EPD_PORT_OUT |= (1<<PANEL_ON_PIN);
	
	///delay_ms(10);
	__delay_cycles(MS_DELAY*10);
	
	EPD_PORT_OUT |= (1<<RESET_PIN);
	EPD_PORT_OUT |= (1<<BORDER_PIN);
	EPD_PORT_OUT |= (1<<CHIP_SELECT_PIN);
	
	//delay_ms(5);
	__delay_cycles(MS_DELAY*5);
	
	EPD_PORT_OUT &= ~(1<<RESET_PIN);
	//delay_ms(5);
	__delay_cycles(MS_DELAY*5);
	
	EPD_PORT_OUT |= (1<<RESET_PIN);
	//delay_ms(5);
	__delay_cycles(MS_DELAY*5);
	
	//wait for COG to be ready
	while(EPD_PORT_IN & (1<<BUSY_PIN));
	
	//channel select
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x01), 2);
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, channel_select, channel_select_length);
	
	//DC/DC frequency
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x06), 2);
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0xFF), 2);
	
	//high power mode osc
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x07), 2);
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x9D), 2);
	
	//disable ADC
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x08), 2);
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x00), 2);
	
	//Vcom level
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x09), 2);
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, v_com_lvl, 3);
	
	//gate and source voltage levels
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x04), 2);
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, gate_source, gate_source_length);
	
	delay_ms(10);
	//__delay_cycles(MS_DELAY*10);
	
	//drive latch on
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x03), 2);
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x01), 2);
	
	//drive latch off
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x03), 2);
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x00), 2);
	
	//delay_ms(5);
	__delay_cycles(MS_DELAY*5);
	
	//charge pump positive voltage on
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x05), 2);
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x01), 2);
	
	//final delay before PWM off
	delay_ms(30);
	//__delay_cycles(MS_DELAY*30);
	pwm_on(false);
	
	//charge pump negative voltage on
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x05), 2);
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x03), 2);
	
	//delay_ms(30);
	__delay_cycles(MS_DELAY*30);
	
	//vcom driver on
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x05), 2);
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x0F), 2);
	
	//delay_ms(30);
	__delay_cycles(MS_DELAY*30);
	
	
	//output enable to disable
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x02), 2);
	//delay_us(10);
	__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x24), 2);
	
	spi_off();
}

void epd_end(void){
	//dummy frame
	frame_fixed(0x55, EPD_normal);
	
	//dummy line and border
	line(0x7FFFU, 0, 0x55, false, EPD_normal); //CHECK: 0xaa should be 0x55
	delay_ms(250);
	//__delay_cycles(MS_DELAY*250);
	
	spi_on();
	
	//latch reset turn on
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x03), 2);
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x01), 2);
	
	//output enable off
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x02), 2);
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x05), 2);
	
	//vcom power off
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x05), 2);
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x0e), 2);
	
	//power off negative charge pump
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x05), 2);
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x02), 2);
	
	//discharge
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x04), 2);
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x0C), 2);
	
	delay_ms(120);
	//__delay_cycles(MS_DELAY*120);
	
	//all charge pumps off
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x05), 2);
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x00), 2);
	
	//turn off osc
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x07), 2);
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x0D), 2);
	
	//discharge internal - 1
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x04), 2);
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x50), 2);
	
	delay_ms(40);
	//__delay_cycles(MS_DELAY*40);
	
	//discharge internal - 2
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x04), 2);
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0xA0), 2);
	
	delay_ms(40);
	//__delay_cycles(MS_DELAY*40);
	
	//discharge internal - 3
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x04), 2);
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x00), 2);
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	
	//turn off power and all signals
	EPD_PORT_OUT &= ~(1<<RESET_PIN);
	EPD_PORT_OUT &= ~(1<<PANEL_ON_PIN);
	EPD_PORT_OUT &= ~(1<<BORDER_PIN);
	
	//ensure SPI MOSI and CLOCK are low before CS low
	spi_off();
	EPD_PORT_OUT &= ~(1<<CHIP_SELECT_PIN);
	
	//discharge pulse
	EPD_PORT_OUT |= (1<<DISCHARGE_PIN);
	delay_ms(150);
	//__delay_cycles(MS_DELAY*150);
	EPD_PORT_OUT &= ~(1<<DISCHARGE_PIN);
}

//one frame is made up of the number of lines * rows. 
//1.44" has 96 lines * 128 dots. ie 96 * 32 bytes (2 bits per dot)
void frame_fixed(unsigned char fixed_value, EPD_stage stage){
	for(char line_number = 0; line_number < lines_per_display; line_number++){
		line(line_number, 0, fixed_value, false, stage);
	}
}

void frame_data(unsigned char *image, EPD_stage stage){
	for(char line_number = 0; line_number < lines_per_display; line_number++){
		//hoopla garrett is testing
		/*if(line_number >= 34 && line <= 62){
		line(line_number, &digit1[line_number * bytes_per_line], 0, true, stage);
	}
      else{
		*/ line(line_number, &image[line_number * bytes_per_line], 0, true, stage);
		//}
	}
}


//TODO: find out what frame_cb is from original code, not included here
void frame_fixed_repeat(unsigned char fixed_value, EPD_stage stage){
	
	frame_fixed(fixed_value, stage);
	delay_ms(480);
	
}

void frame_data_repeat(unsigned char *image, EPD_stage stage){
	
	frame_data(image, stage);
	delay_ms(480);
	//__delay_cycles(MS_DELAY*480);
	
}

void line(int line_num, unsigned char *data, char fixed_value, char read_progmem, EPD_stage stage){
	spi_on();
	
	//charge pump voltage levels
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x04), 2);
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, gate_source, gate_source_length);
	
	//send data
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x0A), 2);
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	
	//CS low
	EPD_PORT_OUT &= ~(1<<CHIP_SELECT_PIN);
	spi_put_wait(0x72, BUSY_PIN);
	
	//border byte
	spi_put_wait(0x00, BUSY_PIN);
	
	//even pixels
	for(char b = bytes_per_line; b > 0; --b){
		if(0 != data){
			char pixels = data[b-1] & 0xAA;
			
			switch(stage) {
				case EPD_compensate:  // B -> W, W -> B (Current Image)
				pixels = 0xaa | ((pixels ^ 0xaa) >> 1);
				break;
				case EPD_white:       // B -> N, W -> W (Current Image)
				pixels = 0x55 + ((pixels ^ 0xaa) >> 1);
				break;
				case EPD_inverse:     // B -> N, W -> B (New Image)
				pixels = 0x55 | (pixels ^ 0xaa);
				break;
				case EPD_normal:       // B -> B, W -> W (New Image)
				pixels = 0xaa | (pixels >> 1);
				break;
			}
			spi_put_wait(pixels, BUSY_PIN);
		}
		else{
			spi_put_wait(fixed_value, BUSY_PIN);
		}
	}
	
	//scan line
	for(char b = 0; b < bytes_per_scan; ++b){
		if(line_num / 4 == b){
			spi_put_wait(0xc0 >> (2 * (line_num & 0x03)), BUSY_PIN);
		}
		else{
			spi_put_wait(0x00, BUSY_PIN);
		}
	}
	
	//odd pixels
	for(char b = 0; b < bytes_per_line; ++b){
		if(0 != data){
			char pixels = data[b] & 0x55;
			
			switch(stage) {
				case EPD_compensate:  // B -> W, W -> B (Current Image)
				pixels = 0xaa | (pixels ^ 0x55);
				break;
				case EPD_white:       // B -> N, W -> W (Current Image)
				pixels = 0x55 + (pixels ^ 0x55);
				break;
				case EPD_inverse:     // B -> N, W -> B (New Image)
				pixels = 0x55 | ((pixels ^ 0x55) << 1);
				break;
				case EPD_normal:       // B -> B, W -> W (New Image)
				pixels = 0xaa | pixels;
				break;
			}
			char p1 = (pixels >> 6) & 0x03;
			char p2 = (pixels >> 4) & 0x03;
			char p3 = (pixels >> 2) & 0x03;
			char p4 = (pixels >> 0) & 0x03;
			pixels = (p1 << 0) | (p2 << 2) | (p3 << 4) | (p4 << 6);
			spi_put_wait(pixels, BUSY_PIN);
		}
		else{
			spi_put_wait(fixed_value, BUSY_PIN);
		}
	}
	
	if(filler){
		spi_put_wait(0x00, BUSY_PIN);
	}
	
	//CS high
	EPD_PORT_OUT |= (1<<CHIP_SELECT_PIN);
	
	//output data to panel
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x70, 0x02), 2);
	delay_us(10);
	//__delay_cycles(US_DELAY*10);
	spi_send(CHIP_SELECT_PIN, fill_temp(0x72, 0x2F), 2);
	
	spi_off();
}

//clear display (anything -> white)
void epd_clear(void){
	frame_fixed_repeat(0xFF, EPD_compensate);
	frame_fixed_repeat(0xFF, EPD_white);
	frame_fixed_repeat(0xaa, EPD_inverse);
	frame_fixed_repeat(0xaa, EPD_normal);
	//frame_fixed_repeat(0xaa, EPD_normal);
}

//assuming a clear (white) screen output an image (PROGMEM data)
void image(unsigned char *image){
	frame_fixed_repeat(0xAA, EPD_compensate);
	frame_fixed_repeat(0xAA, EPD_white);
	frame_data_repeat(buf_write, EPD_inverse);
	frame_data_repeat(buf_write, EPD_normal);
}

//change from old image to new image (PROGMEM data)
void image_2(unsigned char *old_image,unsigned char *new_image){
	frame_data_repeat(buf_write, EPD_compensate);
	frame_data_repeat(buf_write, EPD_white);
	frame_data_repeat(buf_write, EPD_inverse);
	frame_data_repeat(buf_write, EPD_normal);
}


char dec_to_bcd(char data){
	while(data>99){
		data -= 100;	
	}
	char bcd = (data / 10)<<4;
	bcd |= (data%10) & 0x0F;
	return(bcd);
}


void buffer_init(void){
	int i = 0;
	for(i=0; i<1536; i++){
		
		buf_write[i] = 0x00;
	}
}

void write_buffer_time(char hour, char minute){
	int i = 0;
	int j = 0;
	int k =0;
	
	for(i=896; i<899;i++){
		for(j=i; j<(i+512);j+=16){
			
			buf_write[j] = numbers[hour>>4][k];
			k++;
			
		}
	}
	k =0;
	for(i=899; i<902;i++){
		for(j=i; j<(i+512);j+=16){
			
			buf_write[j] = numbers[hour&0x0F][k];
			k++;
			
		}
	}
	k =0;
	/*
	for(i=902; i<903; i++){
		for(j=i; j<(i+512);j+=16){
			
			buf_write[j] = numbers[10][k];
			k++;
		}
		
	}
	*/
	k =0;
	for(i=903; i<906;i++){
		for(j=i; j<(i+512);j+=16){
			
			buf_write[j] = numbers[minute>>4][k];
			k++;
			
		}
	}
	k =0;
	for(i=906; i<909;i++){
		for(j=i; j<(i+512);j+=16){
			
			buf_write[j] = numbers[minute &0x0F][k];
			k++;
			
		}
	}
}

void write_buffer_hr(char hr_data){
	int i = 0;
	int j = 0;
	char k =0;
	char data = dec_to_bcd(hr_data);
	/*
	for(i=1; i<4;i++){
		for(j=i; j<(i+512);j+=16){
			
			buf_write[j] = numbers[11][k];
			k++;
			
		}
	}
	k =0;
	*/
	for(i=4; i<7;i++){
		for(j=i; j<(i+512);j+=16){
			if(hr_data > 99){
				buf_write[j] = numbers[(hr_data/100)][k];
			} else {
				buf_write[j] = 0x00;
			}
			k++;
			
		}
	}
	k =0;
	for(i=7; i<10;i++){
		for(j=i; j<(i+512);j+=16){
			
			buf_write[j] = numbers[data>>4][k];
			k++;
			
		}
	}
	k =0;
	for(i=10; i<13;i++){
		for(j=i; j<(i+512);j+=16){
			
			buf_write[j] = numbers[data & 0x0F][k];
			k++;
			
		}
	}
	k =0;
}

void write_buffer_bt(){
	int i = 0;
	int j = 0;
	for(i=486; i<489;i++){
		for(j=i; j<(i+512);j+=16){
			
			buf_write[j] =0xff;
			
		}
	}
}