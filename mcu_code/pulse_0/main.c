/* Author: Tuan Truong & Garrett Clay
* Date: 3.1.2014
* Brief: Senior design project '13-'14. MSP430F5528 is microcontroller used
* 		for the wearable heart monitoring watch. Microcontroller uses
* 		spi, usart, i2c, ADC, pwm, etc. to control e-ink display from 
* 		pervasive display, pulse sensor, communication with bluetooth
* 		etc.
*/

/* Hardware Description:
* Ext. Interface	Port #		I/O to MSP
--------------		------		----------
* DISP_PWM 		P1.1		O
* BUTTONS 		P2.0-2.3	I
* LED 			P2.4-2.7	O
* EPD_SPI_STE		P4.0		O
* EPD_SPI_MOSI		P4.1		O
* EPD_SPI_MISO		P4.2		I
* EPD_SPI_CLK		P4.3		O
* ADC_READ		P6.0		I
* ADC_READ_2		P6.1		I
* EPD_CHIP_SEL 		P6.2		O
* EPD_PANEL_ON 		P6.3		O
* EPD_BUSY		P6.4		I
* EPD_RESET 		P6.5		O
* EPD_BORDER 		P6.6		O
* EPD_DISCHARGE		P6.7		O
*
*
*
*/

/* ERRATA
*
*/


/*includes*/
#include "msp430.h"
#include "pwm.h"
#include "spi.h"
#include "clock.h"
#include "usart.h"
#include "adc.h"
#include "epd.h"
#include "heart.h"
//#include "i2c.h"



/*macros*/
#define LEDOUT 	P2OUT
#define LED0 	0xE0
#define LED1 	0xD0
#define LED2 	0xB0
#define LED3 	0x70
#define LED_ALL	0x00
#define BTN0	0x01
#define BTN1	0x02
#define BTN2	0x04
#define BTN3	0x08
#define true 	1
#define false 	0


#define OPEN_COM	0xF0
#define SEND_HR		0xAB
#define TIME_REQ	0xAC

volatile char temp = 0;
unsigned char HR = 0;

void ddr_init(void){
	P2DIR = 0xF0;	//set leds & set buttons
	P2REN = 0x0F;	//enable  resistor
	P2OUT |= 0xF;	//set pull up resistor 
	P1DIR |= 0xFF;	//set all port 1 to output
	P6DIR = 0xEC;	//set up display pins
	P6SEL |= 0x01;	//enable A/D channel A0
	
	//set baud rate to 115200 for Bluetooth module 
	P4DIR |= (1<<7);
	P4DIR &= (1<<6);
	P4OUT &= ~(1<<7);
}

void led_set(char data){
	data |= 0x0F;
	LEDOUT = data;
}


void led_startup(void){
	led_set(0xFF);
	delay_ms(220);
	led_set(LED0);
	delay_ms(220);
	led_set(LED1);
	delay_ms(220);
	led_set(LED2);
	delay_ms(220);
	led_set(LED3);
	delay_ms(220);
	led_set(LED_ALL);
	delay_ms(220);
	led_set(0xFF);
	delay_ms(220);
	led_set(LED_ALL);
	delay_ms(220);
	led_set(0xFF);
	
}



int main( void )
{
	// Stop watchdog timer to prevent time out reset
	WDTCTL = WDTPW + WDTHOLD;
	
	ddr_init();	//set ddr for LED 
	clock_init();	//set clock to 16MHz
	pwm_init();	//set timer for PWM
	spi_init();	//set SPI
	usart_init();
	adc_init();
	rtc_init();
	hr_timer_init();
	delay_init();
	
	char dummy =0; 
				 
	__bis_SR_register(GIE);	// Enter LPM0, enable interrupts
	led_startup();	//run LED start up pattern
	
	//__no_operation();			// For debugger
	
	
	epd_begin();
	epd_clear();
	epd_end();
	
	
	write_buffer_bt();
	epd_begin();      //power up EPD panel
	image(0x00);
	epd_end();
	while(!(P4IN & 0x40));
	usart_put(OPEN_COM);
	usart_put(TIME_REQ);
	delay_ms(30);
	
	rtc_set_hour(dec_to_bcd(usart_get()));
	rtc_set_min(dec_to_bcd(usart_get()));
	rtc_set_sec(dec_to_bcd(usart_get()));
	epd_begin();
	epd_clear();
	epd_end();

	buffer_init();
	if(hr_ready(true) == true){
		HR = get_hr();
		hr_ready(false);
	}
    
	write_buffer_time(rtc_get_hour(), rtc_get_min());
	write_buffer_hr(HR);
	
	while(1){
		set_tc(0);
		led_set(get_btn()<<4);
		epd_begin();      //power up EPD panel
		image(0x00);
		epd_end();
		
		
		
		while(!time_changed()){
			if(get_btn() == 0x0E){
				set_tc(1);
				
			}
			
			if(get_btn() == 0x07){
				usart_put(OPEN_COM);
				usart_put(TIME_REQ);
				delay_ms(30);
				delay_ms(30);
				rtc_set_hour(dec_to_bcd(usart_get()));
				rtc_set_min(dec_to_bcd(usart_get()));
				rtc_set_sec(dec_to_bcd(usart_get()));
				
				delay_ms(30);
				delay_ms(30);
			}
			
			if(get_btn() == 0x0D){
				dummy = 1;
				set_tc(1);
			}
			led_set(get_btn()<<4);
		}
		if(hr_ready(true) == true){
			HR = get_hr();
			hr_ready(false);
		}
		if (dummy ==1){
			HR = 200;
			dummy =0;
		}
		if(HR>140){
			led_set(0xFF);
			delay_ms(220);
			led_set(LED_ALL);
			delay_ms(220);
			led_set(0xFF);
			delay_ms(220);
		}
		write_buffer_hr(HR);
		
		usart_put(OPEN_COM);
		delay_ms(5);
		usart_put(SEND_HR);
		delay_ms(5);
		usart_put(HR);
		
		
	}

}

