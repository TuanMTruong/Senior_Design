/* Author: Garrett Clay
   Date: 2.19.2014
   Brief: 
*/

#define ms_delay 80000   //TODO: confirm assumption of 8mhz clock
#define us_delay 8

//declarations
void dir_setup(void);
void spi_setup(void);
void SPI_on();
void SPI_off();
void SPI_put(char c);
void SPI_put_wait(char c, int busy_pin);
void SPI_send(char cs_pin, const char *buffer, long length);

void PWM_start(int pin);
void PWM_stop(int pin);