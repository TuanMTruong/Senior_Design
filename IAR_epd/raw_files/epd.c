/* Author: Garrett Clay
   Date: 2.19.2014
   Brief: Functions to control the COG (chip on glass) driver for a 1.44" EPD
          display. Code modified from sample code provided by pervsive display 
          for use on Arduino.
*/

/* TODO: set DDR for the following
    P6.2        cs
    P6.3        panel_on
    P6.4        busy            (input to mcu)
    P6.5        reset
    P6.6        border
    P6.7        discharge

    P1.1        pwm

   Errata
    -change delay functions to use timers and interrupts to save power/cpu time

*/

#include "msp430.h"
#include "epd.h"
#include "spi.h"
#include <stdlib.h>
#include <stdio.h>

//macros        TODO: move to epd.h
                //TODO: confirm pins w/ Tuan
#define false 0
#define true 1
#define chip_select_pin 2
#define panel_on_pin    3
#define busy_pin        4
#define reset_pin       5
#define border_pin      6
#define discharge_pin   7
#define pwm_pin         1

#define ULONG_MAX       4294967295

#define ARRAY(type, ...) ((type[]){__VA_ARGS__})
#define CU8(...) (ARRAY(const char, __VA_ARGS__))
               
//changeable macros
#define ms_delay 80000   //TODO: confirm assumption of 8mhz clock
#define us_delay 8

//display parameters
int lines_per_display = 96;
int dots_per_line = 128;
int bytes_per_line = 128 / 8;
int bytes_per_scan = 96 / 4;
int filler = false;

static char channel_select[] = {0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x00};
long channel_select_length = 8;
static char gate_source[] = {0x72, 0x03};
long gate_source_length = 2;

int factored_stage_time = 480;   //no temperature data

void begin(){          
  //power up sequence
  P6OUT &= ~(1<<reset_pin);
  P6OUT &= ~(1<<panel_on_pin);
  P6OUT &= ~(1<<discharge_pin);
  P6OUT &= ~(1<<border_pin);
  P6OUT &= ~(1<<chip_select_pin);
  
  SPI_on();
  
  PWM_start(pwm_pin);
  __delay_cycles(ms_delay*5);
  P6OUT |= (1<<panel_on_pin);
  __delay_cycles(ms_delay*10);
  
  P6OUT |= (1<<reset_pin);
  P6OUT |= (1<<border_pin);
  P6OUT |= (1<<chip_select_pin);
  __delay_cycles(ms_delay*5);
  
  P6OUT &= ~(1<<reset_pin);
  __delay_cycles(ms_delay*5);
  
  P6OUT |= (1<<reset_pin);
  __delay_cycles(ms_delay*5);
  
  //wait for COG to be ready
  while(P6IN && busy_pin){}
  
  //channel select
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x01), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, channel_select, channel_select_length);
  
  //DC/DC frequency
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x06), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0xFF), 2);
  
  //high power mode osc
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x07), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x9D), 2);
  
  //disable ADC
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x08), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x00), 2);
  
  //Vcom level
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x09), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0xD0, 0x00), 3);
  
  //gate and source voltage levels
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x04), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, gate_source, gate_source_length);
  
  __delay_cycles(ms_delay*5);
  
  //drive latch on
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x03), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x01), 2);
  
  //drive latch off
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x03), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x00), 2);
  
  __delay_cycles(ms_delay*5);
  
  //charge pump positive voltage on
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x05), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x01), 2);
  
  //final delay before PWM off
  __delay_cycles(ms_delay*30);
  PWM_stop(pwm_pin);
  
  //charge pump negative voltage on
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x05), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x03), 2);
  
  __delay_cycles(ms_delay*30);
  
  //vcom driver on
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x05), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x0F), 2);
  
  __delay_cycles(ms_delay*30);
  
  //output enable to disable
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x02), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x24), 2);
  
  SPI_off();
}

void end(){
  //dummy frame
  frame_fixed(0x55, EPD_normal);
  
  //dummy line and border
  line(0x7FFFU, 0, 0x55, false, EPD_normal);
  __delay_cycles(ms_delay*250);
  
  SPI_on();
  
  //latch reset turn on
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x03), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x01), 2);
  
  //output enable off
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x02), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x05), 2);
  
  //vcom power off
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x05), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x0e), 2);
  
  //power off negative charge pump
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x05), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x02), 2);
  
  //discharge
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x04), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x0C), 2);
  
  __delay_cycles(ms_delay*120);
  
  //all charge pumps off
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x05), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x00), 2);

  //turn off osc
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x07), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x0D), 2);
  
  //discharge internal - 1
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x04), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x50), 2);
  
  __delay_cycles(ms_delay*40);
  
  //discharge internal - 2
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x04), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0xA0), 2);
  
  __delay_cycles(ms_delay*40);
  
  //discharge internal - 3
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x04), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x00), 2);
  __delay_cycles(us_delay*10);
  
  //turn off power and all signals
  P6OUT &= ~(1<<reset_pin);
  P6OUT &= ~(1<<panel_on_pin);
  P6OUT &= ~(1<<border_pin);
  
  //ensure SPI MOSI and CLOCK are low before CS low
  SPI_off();
  P6OUT &= ~(1<<chip_select_pin);
  
  //discharge pulse
  P6OUT |= (1<<discharge_pin);
  __delay_cycles(ms_delay*150);
  P6OUT &= ~(1<<discharge_pin);
}

//one frame is made up of the number of lines * rows. 
//1.44" has 96 lines * 128 dots. ie 96 * 32 bytes (2 bits per dot)
void frame_fixed(unsigned char fixed_value, EPD_stage stage){
  for(int line_number = 0; line_number < lines_per_display; line_number++){
    line(line_number, 0, fixed_value, false, stage);
  }
}

void frame_data(PROGMEM const char *image, PROGMEM const char *digit1, EPD_stage stage){
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

//timerB used to replace millis() arduino function
/* TODO: attempting to not use timing fuction millis()
void timer_b_setup(){
  UCSCTL3 = SELREF_2;   //set DC0 FLL reference = REFO
  UCSCTL4 |= SELA_2;    //set ACLK = REFO
  __bis_SR_register(SCG0);      //disable the FLL control loop
  UCSCTL0 = 0x0000;             //set lowest possible DC0x, MODx
  UCSCTL1 = DCORSEL_7;          //select DC0 range 50mhz operation
  UCSCTL2 = FLLD_1 + 762;       //set DC0 multiplier for 25mhz
                                //(N + 1) * FLLRef = Fdco
                                //(762 + 1) * 32768 = 25mhz
                                //set FLL div = fDC0CLK/2
  __bic_SR_register(SCG0);      //enable the FLL control loop
  
  //worst case settling time for the DC0 when the DC0 range bits have been
  //changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
  //UG for optimization. 
  //32 x 32 x 25 mhz / 32,768 hz ~ 780k MCLK cycles for DC0 to settle
  __delay_cycles(782000);
  
  do {
    UCSCTL7 &= (XT2OFFG + XT1LFOFFG + DCOFFG);
                        //clear XT2, XT1, DC0 fault flags
    SFRIFG1 &= ~OFIFG;  //clear fault flags
  }while (SFRIFG1&OFIFG);       //test oscillator fault flags
  
  //omitted port_mapping function here
  
  TBCCTL0 = OUTMOD_4 + CCIE;    //CCR0 toggle, interrupt enabled
  TBCTL = TBSSEL_2 + MC_2 + TBCLR + TBIE;       //SMCLK, contmode, clear TAR,
                                                    //interrupt enabled
  __bis_SR_register(LPM0_bits + GIE);   //enter LPM3, interrupts enabled
  __no_operation();                     //for debugger
}

//timer B0 interrupt service routine
#pragma vector=TIMERB0_VECTOR
__interrupt void TIMERB0_ISR (void){
  TBCCR0 += 200;                //relode period
}
*/

//TODO: find out what frame_cb is from original code, not included here
void frame_fixed_repeat(char fixed_value, EPD_stage stage){
  //long stage_time = factored_stage_time;
  //do {          //TODO: need timing function
    //unsigned long t_start = millis();
    frame_fixed(fixed_value, stage);
    __delay_cycles(ms_delay*480);
    //unsigned long t_end = millis();
    //if(t_end > t_start){
    //  stage_time -= t_end - t_start;
    //}
    //else{
    //  stage_time -= t_start - t_end + 1 + ULONG_MAX;
    //}
  //}while(stage_time > 0);
}

void frame_data_repeat(PROGMEM const char *image, PROGMEM const char *digit1, EPD_stage stage){
  //long stage_time = factored_stage_time;
  //do {
    //TODO: need timing function
    //unsigned long t_start = millis();
    frame_data(image, digit1, stage);
    __delay_cycles(ms_delay*480);
    //unsigned long t_end = millis();
    //if(t_end > t_start){
    //  stage_time -= t_end - t_start;
   //}
    //else{
    //  stage_time -= t_start - t_end + 1 + ULONG_MAX;
    //}
  //}while(stage_time > 0);
}

void line(long line_num, const char *data, char fixed_value, int read_progmem, EPD_stage stage){
  SPI_on();
  
  //charge pump voltage levels
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x04), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, gate_source, gate_source_length);
  
  //send data
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x0A), 2);
  __delay_cycles(us_delay*10);
  
  //CS low
  P6OUT &= ~(1<<chip_select_pin);
  SPI_put_wait(0x72, busy_pin);
  
  //border byte
  SPI_put_wait(0x00, busy_pin);
  
  //even pixels
  for(long b = bytes_per_line; b > 0; b--){
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
      SPI_put_wait(pixels, busy_pin);
    }
    else{
      SPI_put_wait(fixed_value, busy_pin);
    }
  }
  
  //scan line
  for(long b = 0; b < bytes_per_scan; b++){
    if(line_num / 4 == b){
      SPI_put_wait(0x00 >> (2 * (line_num & 0x03)), busy_pin);
    }
    else{
      SPI_put_wait(0x00, busy_pin);
    }
  }
  
  //odd pixels
  for(long b = 0; b < bytes_per_line; b++){
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
      SPI_put_wait(pixels, busy_pin);
    }
    else{
      SPI_put_wait(fixed_value, busy_pin);
    }
  }
  
  if(filler){
    SPI_put_wait(0x00, busy_pin);
  }
  
  //CS high
  P6OUT |= (1<<chip_select_pin);
  
  //output data to panel
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x70, 0x02), 2);
  __delay_cycles(us_delay*10);
  SPI_send(chip_select_pin, CU8(0x72, 0x2F), 2);
  
  SPI_off();
}
 
//clear display (anything -> white)
void clear(){
  frame_fixed_repeat(0xFF, EPD_compensate);
  frame_fixed_repeat(0xFF, EPD_white);
  frame_fixed_repeat(0xAA, EPD_inverse);
  frame_fixed_repeat(0xAA, EPD_normal);
}
  
//assuming a clear (white) screen output an image (PROGMEM data)
void image(PROGMEM const char *image, PROGMEM const char *digit1){
  frame_fixed_repeat(0xAA, EPD_compensate);
  frame_fixed_repeat(0xAA, EPD_white);
  frame_data_repeat(image, digit1, EPD_inverse);
  frame_data_repeat(image, digit1, EPD_normal);
}

//change from old image to new image (PROGMEM data)
void image_2(PROGMEM const char *old_image, PROGMEM const char *new_image, PROGMEM const char *digit1){
  frame_data_repeat(old_image, digit1, EPD_compensate);
  frame_data_repeat(old_image, digit1, EPD_white);
  frame_data_repeat(new_image, digit1, EPD_inverse);
  frame_data_repeat(new_image, digit1, EPD_normal);
}

  
  
