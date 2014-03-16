/* Author: Tuan Truong & Garrett Clay
Date: 3.3.2014
Brief: Pervasive display e-ink display chip on glass (COG) driver. Intended
for use with 1.44" display. 
*/

#include "msp430.h"

#define PROGMEM				//TODO: figure out what this does
#define SCREEN_SIZE 	144		//128 by 96
#define long_time 	800000UL


// select two images from:  text_image text-hello cat aphrodite venus saturn
/*
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

// images

#define unsigned
#define char char//uint8_t
#include IMAGE_1_FILE
#undef char
#undef unsigned

#define unsigned
#define char char//uint8_t
#include IMAGE_2_FILE
#undef char
#undef unsigned

#define unsigned
#define char char//uint8_t
#include IMAGE_3_FILE
#undef char
#undef unsigned
*/
typedef enum {
	EPD_1_44,   // 128 x 96
	EPD_2_0,    // 200 x 96
	EPD_2_7     // 264 x 176
} EPD_size;

typedef enum {       //Image pixel -> display pixel
	EPD_compensate,  //B -> W, W -> B (Current Image)
	EPD_white,       // B -> N, W -> W (Current Image)
	EPD_inverse,     // B -> N, W -> B (New Image)
	EPD_normal       // B -> B, W -> W (New Image)
} EPD_stage;





//declarations
void epd_begin(void);
void epd_end(void);
void frame_fixed(unsigned char fixed_value, EPD_stage stage);
void frame_data( unsigned char *image, EPD_stage stage);
void frame_fixed_repeat(unsigned char fixed_value, EPD_stage stage);
void frame_data_repeat( unsigned char *image, EPD_stage stage);
void line(int line_num, unsigned char *data, char fixed_value, char read_progmem, EPD_stage stage);
void epd_clear(void);
void image(unsigned char *image);
void image_2(unsigned char *old_image,unsigned char *new_image);
void write_buffer_time(char hour, char minute);
void buffer_init(void);
void write_buffer_hr(char hr_data);
char dec_to_bcd(char data);
void write_buffer_bt();