/* Author: Tuan Truong & Garrett Clay
   Date: 3.1.2014
   Brief: Allow for USART communication from MSP to Bluetooth.
*/


#define USART_TX	0x10
#define USART_RX	0x20


void usart_init(void);
char usart_data_avaliable(void);
void usart_put(char data);
char usart_get(void);
