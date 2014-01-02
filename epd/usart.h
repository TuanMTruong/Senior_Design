/******************************************************************/
// Author: Tuan
// Date: 12/20/13
// has basic usart Function 
// TODO add Interrupt support
/******************************************************************/


//macros



//Function prototypes
void usart_setup(uint32_t baudrate);
void usart_sendbyte(uint8_t data);
void usart_sendarray(char *array, uint8_t length);
uint8_t usart_readbyte(void);
