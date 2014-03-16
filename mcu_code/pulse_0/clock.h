/* Authors: Tuan Truong & Garrett Clay
Date: 3.1.2014
Brief: Sets master clock (MCLK) for MSP430.
*/
//CPU freq. = 16mhz

#define S_DELAY	16000000UL
#define MS_DELAY 16000UL
#define US_DELAY 16

void clock_init(void);
void delay_init(void);
char delay_ms(long milli);
char delay_us(long micro);

void rtc_init(void);

char rtc_ready(void);
char rtc_get_sec(void);
char rtc_get_min(void);
char rtc_get_hour(void);
char rtc_get_day(void);
char rtc_get_date(void);
char rtc_get_month(void);
int rtc_get_year(void);
void rtc_set_sec(char seconds);
void rtc_set_min(char minutes);
void rtc_set_hour(char hours);
void rtc_set_day(char day);
void rtc_set_date(char date);
void rtc_set_month(char month);
void rtc_set_year(int year);
char time_changed(void);
void set_tc(char data);
