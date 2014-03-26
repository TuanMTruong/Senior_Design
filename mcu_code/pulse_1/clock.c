/* Authors: Tuan Truong & Garrett Clay
Date: 3.1.2014
Brief: Sets master clock (MCLK) for MSP430. 

CPU freq. = 16mhz
*/
#include "msp430.h"
#include "usart.h"
#include "clock.h"
#include "epd.h"

volatile char delay_complete = 0;
volatile char time_update = 0;

void clock_init(void){
	UCSCTL3 = SELREF_2;		//set DCO FLL reference = REFO
	UCSCTL4 |= SELA_2;		//set ACLK = REFO
	UCSCTL0 = 0x0000;		//set lowest possible DC)x, MODx
	//UCSCTL6 |= XCAP_3;            //internal cap
	
	do
	{
		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
		// Clear XT2,XT1,DCO fault flags
		SFRIFG1 &= ~OFIFG;	// Clear fault flags
	}while (SFRIFG1&OFIFG);		// Test oscillator fault flag
	
	__bis_SR_register(SCG0);	//disable FLL control loop
	UCSCTL1 = DCORSEL_5;		//select DCO range 16mhz operation
	UCSCTL2 |= 487;	//487 _5 = 16	//set DCO multiplier for 16mhz
	
	/*
	ucSCTL2 = 548 w/ dcorsel_6 for 18mhz
	UCSCTL2 = 487 w/ dcorsel_5 for 16mhz
	*/
	
	//(N+1) * FLLRef = Fdco
	//(487 + 1) * 32768 = 16mhz
	__bic_SR_register(SCG0);	//enable the FLL control loop
}


void delay_init(void){
	TA1CTL = TASSEL_2 | MC_0 | TACLR | ID_1;
	TA1CCTL0 = 0;
	TA1CCR0  = 0;
}

char delay_ms(long milli){
	if(milli> 30){
		milli -= 30;
		delay_ms(milli);
	}
	delay_complete = 0;
	TA1CCTL0 |= CCIE;
	TA1CTL = TASSEL_2| MC_1| TACLR| ID_3;
	
	TA1CCR0 = milli*2000+600;
	while (!delay_complete) {}
	return 1;
}

char delay_us(long micro){
	delay_complete = 0;
	TA1CCTL0 |= CCIE;
	TA1CTL = TASSEL_2 |MC_1| TACLR| ID_0;
	
	TA1CCR0 = micro * 16;
	while (!delay_complete) {}
	return 1;
}

void rtc_init(void){
	
	RTCCTL01 = RTCMODE | RTCBCD | RTCTEV_0 | RTCSSEL_0| RTCTEVIE;
	//RTCCTL2 = 0x0F;
	//RTCCTL3 = 1;
	
}

char rtc_ready(void){
	return(!(RTCCTL01 & RTCRDY));
}


char rtc_get_sec(void){
	while(rtc_ready());
	return(RTCSEC);
}

char rtc_get_min(void){
	while(rtc_ready());
	return(RTCMIN);
}

char rtc_get_hour(void){
	while(rtc_ready());
	return(RTCHOUR);
}

char rtc_get_day(void){
	while(rtc_ready());
	return(RTCDOW);
}

char rtc_get_date(void){
	while(rtc_ready());
	return(RTCDAY);
}

char rtc_get_month(void){
	while(rtc_ready());
	return(RTCMON);
}

int rtc_get_year(void){
	while(rtc_ready());
	return(RTCYEAR);
}

void rtc_set_sec(char seconds){
	while(rtc_ready());
	RTCSEC = seconds;
}

void rtc_set_min(char minutes){
	while(rtc_ready());
	RTCMIN = minutes;
}

void rtc_set_hour(char hours){
	while(rtc_ready());
	RTCHOUR = hours;
}

void rtc_set_day(char day){
	while(rtc_ready());
	RTCDOW = day;
}

void rtc_set_date(char date){
	while(rtc_ready());
	RTCDAY = date;
}

void rtc_set_month(char month){
	while(rtc_ready());
	RTCMON = month;
}

void rtc_set_year(int year){
	while(rtc_ready());
	RTCYEAR = year;
}

char time_changed(void){
	return(time_update);
}

void set_tc(char data){
	time_update = data;
}

#pragma vector=RTC_VECTOR
__interrupt void handle_rtc_interrupt(void){
	switch(__even_in_range(RTCIV,8)){
		case 4:
		write_buffer_time(rtc_get_hour(), rtc_get_min());
		//epd_begin();
		//image(0x00);
		//epd_end();
		time_update = 1;
		break;
		default:
		break;
		
		
	}
	
	
	
}


#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
	delay_complete = 1;
	delay_init();
}


//timerB used to replace millis() arduino function
/* TODO: attempting to not use timing fuction millis()
void timer_b_setup(){
UCSCTL3 = SELREF_2;   //set DC0 FLL reference = REFO
UCSCTL4 |= SELA_2;    //set ACLK = REFO
__bis_SR_register(SCG0);      //disable the FLL control loop
UCSCTL0 = 0x0000;             //set lowest possible DC0x, MODx
UCSCTL1 = DCORSEL_7;          //select DC0 range 50mhz operation
UCSCTL2 = FLLD_1 + 762;       //set DC0 multiplier for 25mhz
//(N + 1) * FLLRef = Fdco
//(762 + 1) * 32768 = 25mhz
//set FLL div = fDC0CLK/2
__bic_SR_register(SCG0);      //enable the FLL control loop

//worst case settling time for the DC0 when the DC0 range bits have been
//changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
//UG for optimization. 
//32 x 32 x 25 mhz / 32,768 hz ~ 780k MCLK cycles for DC0 to settle
__delay_cycles(782000);

do {
UCSCTL7 &= (XT2OFFG + XT1LFOFFG + DCOFFG);
//clear XT2, XT1, DC0 fault flags
SFRIFG1 &= ~OFIFG;  //clear fault flags
  }while (SFRIFG1&OFIFG);       //test oscillator fault flags

//omitted port_mapping function here

TBCCTL0 = OUTMOD_4 + CCIE;    //CCR0 toggle, interrupt enabled
TBCTL = TBSSEL_2 + MC_2 + TBCLR + TBIE;       //SMCLK, contmode, clear TAR,
//interrupt enabled
__bis_SR_register(LPM0_bits + GIE);   //enter LPM3, interrupts enabled
__no_operation();                     //for debugger
}

//timer B0 interrupt service routine
#pragma vector=TIMERB0_VECTOR
__interrupt void TIMERB0_ISR (void){
TBCCR0 += 200;                //relode period
}
*/
