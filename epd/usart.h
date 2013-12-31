/******************************************************************/
// Author: Tuan
// Date: 12/20/13
/******************************************************************/


//macros



//Function prototypes
void usart_setup(uint32_t baudrate);
void usart_sendbyte(uint8_t data);
void usart_sendarray(uint8_t *array, uint8_t length);
uint8_t usart_readbyte(void);
