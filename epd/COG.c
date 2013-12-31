/******************************************************************/
// Author: Tuan
// Date: 12/31/13
// has basic COG Function 
/******************************************************************/

//includes
#include<avr/io.h>
#include<util/delay.h>
#include"spi.h"
#include"usart.h"
#include"COG.h"



/******************************************************************/
//Check to see if COG is busy
//if 1 => BUSY,  else ready to receive new data
/******************************************************************/
uint8_t COG_busy(void){
	return (PINB & (1<<BUSY_PIN));
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

	PORTB &= ~(1<<CS_PIN); 		//1. pull CS_PIN low
	spi_sendbyte(REG_HEADER);   	//2. send header for data
	spi_sendbyte(reg_index); 	//3. send reg_index

	PORTB |= (1<<CS_PIN);   	//4. toggle CS
	_delay_us(10);
	PORTB &= ~(1<<CS_PIN);

	spi_sendbyte(DATA_HEADER);  	//5. send DATA_HEADER
	spi_sendbyte(data); 		//6. send data
	PORTB |= (1<<CS_PIN);   	//7. set CS_PIN high

	return;
}



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
void COG_sendarray(uint8_t reg_index, uint8_t *data, uint8_t num_bytes){

	_delay_us(10);
	PORTB &= ~(1<<CS_PIN);  	//1. pull CS_PIN low
	spi_sendbyte(REG_HEADER);   	//2. send header for data
	spi_sendbyte(reg_index);	//3. send reg_index

	PORTB |= (1<<CS_PIN);   	//4. toggle CS
	_delay_us(10);
	PORTB &= ~(1<<CS_PIN);

	spi_sendbyte(DATA_HEADER);  	//5. send DATA_HEADER
	spi_sendarray(data, num_bytes); //6. send array
	PORTB |= (1<<CS_PIN);   	//7. set CS_PIN high

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
void COG_startup(void){
	usart_sendarray("start up COG\n", 13);

	PORTD &= ~(1<<RESET_PIN);	//reset COG
	PORTB &= ~(1<<PANELON_PIN);	//no power to COG
	PORTD &= ~(1<<DISCHARGE_PIN);	//no discharge
	PORTB &= ~(1<<BORDER_PIN);	//Border on
	PORTB &= ~(1<<CS_PIN);		//CS pin low

	//1. Enable PWM
	PWM_ENABLE();
	_delay_ms(5);

	//2. Supply voltage
	PORTB |= (1<PANELON_PIN);
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




/******************************************************************/
// Initialize COG by setting up the correct register 
/******************************************************************/
void COG_init(void){
	usart_sendarray("COG init\n", 9);
	//COG_sendbyte(uint8_t reg_index, uint8_t data)
	//COG_sendbyte(uint8_t reg_index, uint64_t data, uint8_t num_bytes)
	//pass data as an address. ie &data
	uint8_t epd_channel_sel[8];
	epd_channel_sel[0] = 0x00;
	epd_channel_sel[1] = 0x00;
	epd_channel_sel[2] = 0x00;
	epd_channel_sel[3] = 0x00;
	epd_channel_sel[4] = 0x01;
	epd_channel_sel[5] = 0xFF;
	epd_channel_sel[6] = 0xE0;
	epd_channel_sel[7] = 0x00;

	uint8_t epd_vcom_lvl[2];
	epd_vcom_lvl[0] = 0xD0;
	epd_vcom_lvl[1] = 0x00;

	usart_sendarray("busy\n", 5);
	while(COG_busy());  //Check if COG is busy
	usart_sendarray("not busy\n", 9);

	COG_sendarray(0x01, epd_channel_sel, 8);	//Channel Select for EPD 2"
	usart_sendarray("channel sel\n", 12);
	COG_sendbyte(0x06, 0xFF);   			//set DC/DC Frequency 
	COG_sendbyte(0x07, 0x9D);   			//High power mode osc setting
	COG_sendbyte(0x08, 0x00);   			//Disable ADC
	COG_sendarray(0x09, epd_vcom_lvl, 2);   	//Set Vcom Level
	COG_sendbyte(0x04,0x03);    			//Gate and source Voltage level
	_delay_ms(5);
	COG_sendbyte(0x03, 0x01);   			//Driver Latch on, Cancel register noise
	COG_sendbyte(0x03, 0x00);   			//Driver latch off
	COG_sendbyte(0x05, 0x01);  			//Start Charge pump positive Voltage
	//VGH & VDH enable (VGh>12V & VDH >8V)
	_delay_ms(30);
	PWM_DISABLE();  				//disable pwm
	COG_sendbyte(0x05, 0x03);   			//Start charge pump negative voltage
	//VGL < -12v & VDL < -8V
	_delay_ms(30);
	COG_sendbyte(0x05, 0x0F);   			//Set charge pump Vcom_Driver to ON
	_delay_ms(30);
	COG_sendbyte(0x02, 0x24);  			//output enable to disable

	usart_sendarray("init done\n", 10);
	//if all went well the COG should be initialized and ready for image 
	//data. if not... get ready to debug ;)
	return;
}



/******************************************************************/
//after COG is initialized began writing data from buffer to COG to be drawn
/******************************************************************/
void COG_write(uint8_t (*buffer)[EPD_ROW]){
	usart_sendarray("writing to COG\n", 15);

	uint8_t row_counter = 0;    //keeps track of which line is being written to

	for( row_counter = 0; row_counter<EPD_ROW; row_counter++){
		COG_sendbyte(0x04, 0x03);   	//start charge pump
		//send out each line, total of 96 lines
		COG_sendarray(0x0A, buffer[row_counter], EPD_COLUMN);
		COG_sendbyte(0x02, 0x2F);   	//Turn on output enable
	}
}



/******************************************************************/
// The COG needs to be turned off properly 
// Nothing display needs to be written to screen
// latches reset
// chargeump discharged and off
/******************************************************************/
void COG_off(void){
	//TODO add a fill and write nothing screen
	//fill_image_buff(test_buffer_nth, 0x55, 0xFF);	//fill array with nothing
	//COG_write(test_buffer_nth);		//write nothing array to display
	_delay_ms(25);				//wait 
	PORTB &= ~(1<<BORDER_PIN);		//Turn on border (active low)
	_delay_ms(250);				//wait
	PORTB |= (1<< BORDER_PIN);		//turn off border
	COG_sendbyte(0x03, 0x01);		//reset latch
	COG_sendbyte(0x02, 0x05); 		//output enable off
	COG_sendbyte(0x05, 0x0E);		//chargepump vcom off
	COG_sendbyte(0x05, 0x02);		//chargepump negative votlage off
	COG_sendbyte(0x04, 0x0C);		//Discharge
	_delay_ms(125);
	COG_sendbyte(0x05, 0x00);		//all other chargepump off
	COG_sendbyte(0x07, 0x0D);		//turn off osc
	COG_sendbyte(0x04, 0x50); 		//discharge internals
	_delay_ms(40);
	COG_sendbyte(0x04, 0xA0);		//discharge internals
	_delay_ms(40);
	COG_sendbyte(0x04, 0x00);		//discharge internals

	PORTB &= ~(1<<PANELON_PIN);			//disable power

	return;

}


