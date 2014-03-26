/* Author: 	Garrett Clay
* Date:	3.8.2014
* Brief:	Calculating heart rate from pulse sensor using MSP430 F5528.
* 		Reference: http://pulsesensor.myshopify.com/pages/code-and-guide
*/

#include "msp430.h"
#include "heart.h"
#include "adc.h"

volatile char BPM;		//holds heart rate in beats per minute
volatile int signal;		//holds the incoming raw data
volatile int IBI = 600;		//holds time between beats, must be seeded
volatile char pulse = 0;	//true when pulse wave is high, false when low
volatile char QS = 0;		//true when we have a calculated heart rate
volatile int rate[10];		//array to hold last ten IBI values
volatile unsigned long sample_counter = 0;	//used to determine pulse
// timing
volatile unsigned long last_beat_time = 0;	//used to find IBI
volatile int P = 2048;	//used to find peak in pulse wave, seeded
volatile int T = 2048;	//used to find trough in pulse wave, seeded
volatile int thresh = 2048;	//used to find instant moment of heart beat
// seeded
volatile int amp = 100;		//used to hold amp of pulse waveform, seeded
volatile char first_beat = TRUE;	//used to seed rate array so we
//startup with reasonable 

volatile char second_beat = FALSE;	//used to seed rate array so we 
//startup with reasonable BPM

volatile char btn_data = 0;

char hr_ready(int check){
	if(check == TRUE){
		return QS;
	}
	QS = FALSE;
	return QS;
}

char get_hr(void){
	return BPM;
}

void hr_timer_init(void){
	TB0CCTL0 = CCIE;	//CCR0 interrupt enable
	TB0CCR0 = 65;		//set 500hz (2ms) interrupt
	TB0CTL = TBSSEL_1 + MC_1 + TBCLR;	//ACLK, upmode
	//clear TAR, enable interrupt
	
}


char get_btn(){
	return(btn_data);
}

// Timer0 B0 interrupt service routine
#pragma vector=TIMER0_B0_VECTOR
__interrupt void TIMER0_B0_ISR(void)
{
	//might as well grab button data..
	btn_data = P2IN & 0x0F;
	
	//TODO: read ADC here
	signal = get_adc();		//get adc data
	sample_counter += 2;		//read the pulse sensor
	int n = (sample_counter - last_beat_time);	//monitor the time since
	//the last beat to avoid noise
	//find peak and trough of pulse wave
	if((signal < thresh) && (n > (IBI/5)*3)){
		if(signal < T){
			T = signal;
		}
	}
	
	if((signal > thresh) && (signal > P)){
		P = signal;
	}
	
	//now begins heart rate calculation
	//signal surges up in value every time there is a pulse
	if(n > 250){		//avoid high freq. noise
		if((signal > thresh) && (pulse == FALSE) && (n > (IBI/5)*3)){
			pulse = TRUE;		//set flag when there is a pulse
			//TODO: optional, turn on led
			//P2OUT |= 0x80;
			IBI = (sample_counter - last_beat_time);	//measure time
			//between beats in ms
			last_beat_time = sample_counter;	//keep track of
			//time for next pulse
			
			if(second_beat){	//if this is the second beat, 
				//if second_beat == TRUE
				second_beat = FALSE;	//clear flag
				for(int i=0; i <= 9; i++){	//seed the 
					//running total to get realistic
					//BPM at startup
					rate[i] = IBI;
				}
			}
			
			if(first_beat){
				first_beat = FALSE;	//clear firstbeat flag
				second_beat = TRUE;	//set the second beat flag
				//TODO: return because of bad IBI value
				return;
			}
			
			//keep running total of the last 10 IBI values
			long running_total = 0;	//clear running total variable
			
			for(int i=0; i<=8; i++){	//shift data in the rate
				rate[i] = rate[i+1];	//and drop oldest value
				running_total += rate[i]; //add up 9 oldest IBI
			}
			
			rate[9] = IBI;	//add the latest IBI to teh rate array
			running_total += rate[9];	//add the latest IBI
			running_total /= 10;	//average last 10 IBI values
			
			BPM = 60000/running_total;	//how many beats can fit
			//into a minute? thats BPM
			QS = TRUE;		//set quantified self flag
			//CHECK: QS flag is not cleared inside the ISR
		}
	}
	if((signal < thresh) && (pulse == TRUE)){
		//TODO: optinal, turn off led here
		//P2OUT &= ~(0x80);
		pulse = FALSE;		//reset the pulse flag so we can do it 
		//again
		amp = P - T;		//get amplitude of teh pulse wave
		thresh = amp/2 + T;	//set tresh at 50% of the amplitude
		P = thresh;
		T = thresh;	
	}
	if(n > 2500){		//if 2.5 seconds go by without a beat
		thresh = 2048;	//set thresh default
		P = 2048;	//set P default
		T = 2048;	//set T default
		last_beat_time = sample_counter;	//bring the last beat
		//time up to date
		first_beat = TRUE;		//set these to avoid noise
		second_beat = FALSE;		//when we get teh heartbeat back
	}
}

