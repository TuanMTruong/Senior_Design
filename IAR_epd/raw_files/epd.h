/* Author: Garrett Clay
   Date: 2.19.2014
   Brief: 
*/

#include "spi.h"

#define PROGMEM         //TODO: what is this?

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
void begin();
void end();
void frame_fixed(unsigned char fixed_value, EPD_stage stage);
void frame_data(PROGMEM const char *image, PROGMEM const char *digit1, EPD_stage stage);
void timer_b_setup();
void frame_fixed_repeat(char fixed_value, EPD_stage stage);
void frame_data_repeat(PROGMEM const char *image, PROGMEM const char *digit1, EPD_stage stage);
void line(long line_num, const char *data, char fixed_value, int read_progmem, EPD_stage stage);
void clear();
void image(PROGMEM const char *image, PROGMEM const char *digit1);
void image_2(PROGMEM const char *old_image, PROGMEM const char *new_image, PROGMEM const char *digit1);