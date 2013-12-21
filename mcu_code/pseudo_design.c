include libraries

int main(void){
	system_initialize();
	send_debug_message("System Initialized");
	
	//attemp to pair with mobile device
	while(!pair_with_device()){
		if( timeout_excited){
			send_debug_message("paring failed");
			restart_system();
		}
	}

	//get time & date data from mobile device 
	bt_get_time();
	send_debug_message("time = " + time_data + "date = " + date_data);
	
	//write time to display
	disp_write(time + date);

	while(1){

		//if buffer is full of collected adc data start processing it
		if(adc_data_buffer == FULL){
			//convert adc data to heart rate beats per min
			//peak detection
			processed_data();
			//send new heart rate data to mobile device
			bt_send_hr();
			send_debug_message(hr_data);
			//clear buffer for new data
			clear_adc_buffer()
		}

	}


}

//write updated data to display every pre-set interval
//when timer elapse write to display
ISR (disp_write_time){
	//check if there is new data in the display buffer or is it the same
	//if there is new data write to display
	if(check_disp_buffer()){
		disp_write(disp_buffer);
	}
}

//when adc is ready to read new data
ISR (adc_ready){
	//read in value from amplified sensor
	read_adc();
	//store data into buffer until ready to be processed
	store_adc_data();
}

//When usart is ready to send data
ISR(USART_TX_vect){
	//check if usart buffer has new data to be sent out
	if(check_usart_buffer){
		//if new data send it over bluetooth module
		send_usart_data();
	}

}

//check buttons for input every time a timer reach a set count
//this allows checking button input periodically
ISR (button_sample_time){
	if(button_press){
		//so far we haven't define what each buttons do
		//but depending on which button is pressed different actions 
		//will be taken.
		carry_out_action();
	}

}

//check power status
//using a timer interrupt to sample and report power status periodically
ISR (pwr_sample_time){
	//read power status from power interface from power management
	read_pwr_status();

	//if battery is less than 10% send out warning
	if(pwr_status <= 10){
		disp_write("batt < 10%");
	}


}

system_initialize(void){
	setup_ddr();
	setup_usart();
	setup_spi();
	setup_adc();
	setup_pwm();
	setup_timer();
	setup_lowpower_mode();

	return 0;
}


