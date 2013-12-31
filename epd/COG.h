/******************************************************************/
// Author: Tuan
// Date: 12/31/13
// has basic COG Function 
/******************************************************************/



//hardware macros
#define CS_PIN		0	//PB0
#define BORDER_PIN	4	//PB4
#define PWM_PIN		5	//PB5
#define PANELON_PIN 	6	//PB6
#define BUSY_PIN	7	//PB7

#define DISCHARGE_PIN	1	//PD1
#define RESET_PIN	4	//PD4

#define BAUD		38400	//usart speed

//protocol macros
#define REG_HEADER	0x70	//send before sending reg. index
#define DATA_HEADER	0x72	//send before sending reg. data

#define IN_DATA		0x00 	//0x00000000000FFF00
#define IN2_DATA	0x00 	//0x0000000001FFE000

#define PWM_DISABLE()	TCCR1B &= ~(7<<CS10)
#define PWM_ENABLE()	TCCR1B |= (1<<CS10)
#define PWM_SPEED	40	//200KHz

#define EPD_ROW		96
#define EPD_COLUMN	76


//function prototype

uint8_t COG_busy(void);
void COG_sendbyte(uint8_t reg_index, uint8_t data);
void COG_sendarray(uint8_t reg_index, uint8_t *data, uint8_t num_bytes);
void COG_startup(void);
void COG_init(void);
void COG_write(uint8_t (*buffer)[EPD_ROW]);
void COG_off(void);
