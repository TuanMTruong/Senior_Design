/* Authors: Tuan Truong & Garrett Clay
Date: 3.6.2014
Brief: I2C library

*/

#define SDA_PIN 0
#define SCL_PIN 1

void i2c_init(void);
void i2c_put(char *tx_buf, char num_byte);
char i2c_get(void);