/* Author: Garrett Clay
   Date: 2.21.2014
   Brief: Written for an MSP430 F5528 driving a 1.44" dispaly by pervasive
          displays. Writes an image to the EPD and has ability to refresh image
          with update subsets. 
*/

#include "msp430.h"
#include "spi.h"
#include "epd.h"
#include <rtc.h>
#include <stdlib.h>
#include <stdio.h>

// 128 x 96
#define SCREEN_SIZE 144

// select two images from:  text_image text-hello cat aphrodite venus saturn
#define IMAGE_1  clockscreen3
#define IMAGE_2  tuan
#define IMAGE_3  two

// set up images from screen size2
#define EPD_SIZE EPD_1_44
#define FILE_SUFFIX _1_44.xbm
#define NAME_SUFFIX _1_44_bits

// pre-processor convert to string
#define MAKE_STRING1(X) #X
#define MAKE_STRING(X) MAKE_STRING1(X)

// other pre-processor magic
// tiken joining and computing the string for #include
#define ID(X) X
#define MAKE_NAME1(X,Y) ID(X##Y)
#define MAKE_NAME(X,Y) MAKE_NAME1(X,Y)
#define MAKE_JOIN(X,Y) MAKE_STRING(MAKE_NAME(X,Y))

// calculate the include name and variable names
#define IMAGE_1_FILE MAKE_JOIN(IMAGE_1,FILE_SUFFIX)
#define IMAGE_1_BITS MAKE_NAME(IMAGE_1,NAME_SUFFIX)
#define IMAGE_2_FILE MAKE_JOIN(IMAGE_2,FILE_SUFFIX)
#define IMAGE_2_BITS MAKE_NAME(IMAGE_2,NAME_SUFFIX)

#define IMAGE_3_FILE MAKE_JOIN(IMAGE_3,FILE_SUFFIX)
#define IMAGE_3_BITS MAKE_NAME(IMAGE_3,NAME_SUFFIX)


// Add Images library to compiler path
#include "Images.h"  // this is just an empty file

// images
PROGMEM const
#define unsigned
#define char char//uint8_t
#include IMAGE_1_FILE
#undef char
#undef unsigned

PROGMEM const
#define unsigned
#define char char//uint8_t
#include IMAGE_2_FILE
#undef char
#undef unsigned

PROGMEM const
#define unsigned
#define char char//uint8_t
#include IMAGE_3_FILE
#undef char
#undef unsigned

//RTC setup <www.ti.com/lit/an/slaa290a/slaa290a.pdf>
/*void rtc_setup(){
  setTime(0x12, 0, 0, 0);       //initialize time to 12:00:00 AM
  CCR0 = 32768 - 1;
  TACTL = TASSEL_1 + MC_1;        //ACLK, upmode
  CCTL0 |= CCIE;                //enable CCR0 interrupt
  _EINT();  
  //TODO: may need LPM3 here and in service routine
}

//Timer A0 interrupt service routine, for RTC
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void){
  incrementSeconds();
}*/

//io setup
void setup(){
//TODO: set DDRs
  P1DIR |= BIT1;        //disp pwm
  P1SEL |= BIT1;        //selected for pwm
}

static int state = 0;

void main(){
  //TODO: complete main
  WDTCTL = WDTPW + WDTHOLD;     //stop watchdog timer
  begin();      //power up EPD panel
  
 
  switch(state){
  default:
  case 0:       //clear screen
    clear();
    state = 1;
    break;
  case 1:       //clear -> text
    image(IMAGE_1_BITS, IMAGE_3_BITS);
    state++;
    break;
  case 2:       //text -> picture
    image_2(IMAGE_1_BITS, IMAGE_2_BITS, IMAGE_3_BITS);
    state++;
    break;
  case 3:       //picture -> text
    image_2(IMAGE_2_BITS, IMAGE_1_BITS, IMAGE_3_BITS);
    state = 2;  //back to picture next time
    break;
  }
  end();

    
    
}