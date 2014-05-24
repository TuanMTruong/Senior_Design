/******************************************************************/
// Author: Tuan
// Date: 12/31/13
// has basic spi Function 
// TODO add Interrupt support
/******************************************************************/






// Function prototypes
void spi_setup(void);
void spi_sendbyte(uint8_t data);
void spi_sendarray(uint8_t *array, uint8_t length);
