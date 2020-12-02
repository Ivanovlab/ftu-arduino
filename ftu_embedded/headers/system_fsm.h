/*
 * system_fsm.h
 *
 * Created: 2020-12-02 11:27:13 AM
 *  Author: Rohit
 */ 


#ifndef SYSTEM_FSM_H_
#define SYSTEM_FSM_H_


/**
 * \@brief This state_machine runs the system
 * 
 * \@param 
 * 
 * \@return void
 */
void system_fsm_run (int system_fsm_state){
	//initialize variables	
	switch (system_fsm_state){
		case IDLE : {
			if (test_stop){
				//clear and turn off all the outputs
				pin_setup(); //this will reset all the pins to their original values;
				update_json_doc(test_id,test_stop,test_start,test_error,error_message,converted_adc_data,test_time_count,measured_temperature,measured_magnetic_field);
				
				if (serial_signal) {
					send_data_to_serial();
					serial_signal = false;
					
					//Reset device after communicating with Arduino
					//NVIC_SystemReset();			
				}
				
				
			}
			else if (test_start){
				//1. Setting voltage stress for test
				analogWrite(ctrl_vstr,set_dac(desired_fpga_voltage));
				
				//2. Setting time interval for sending data rate
				
				//Temporarily changed to counter file
// 				REG_TC3_COUNT16_CC0 =  counter_value(1,1024,serial_output_rate);                     // Set the TC3 CC0 register
// 				while (TC3->COUNT16.STATUS.bit.SYNCBUSY);											// Wait for synchronization
			}
			
		}
		break;
		case ADC_UPDATE: {

			//1. Update test timer and if time is hit, stop
			if (test_time_count >= desired_time_for_test*60) test_stop=true;
			
			//2. Convert Raw ADC data to understandable values
			adc_auto_scan(raw_adc_data); //converting ADC data
			adc_array_convert(raw_adc_data,converted_adc_data); //converts the raw_adc_data into converted data
			
		}
		break;
		case SERIAL_UPDATE:{
			//1. Set Data to be sent to the user from the ADC update, data is sent in intervals in an Interrupt service routine
			update_json_doc(test_id,test_stop,test_start,test_error,error_message,converted_adc_data,test_time_count,measured_temperature,measured_magnetic_field);
			
			if (serial_signal) {
				send_data_to_serial(); //function to send json packet to serial port
				serial_signal = false; //turn off the serial_signal flag
			}
			
		}
		break;
		case Heater_Update:{
			//update actual and desired temperature, run heater click FSM
			measured_temperature = millivolt_to_celcius((float) converted_adc_data[9]); //AIN1 in ADC converted DATA
			heater_fsm_state = heater_fsm_transition(heater_fsm_state, measured_temperature, desired_temperature);
			heater_pwm_duty = heater_fsm_run(heater_fsm_state, heater_pwm_duty, measured_temperature);
			
		}
		break;
		case Magnetic_Field_update:{
			//update actual and desired magnetic field, fun magnetic field FSM
			measured_magnetic_field = millivolts_to_milliTesla(converted_adc_data[8]);
			//1.state_run
			magnetic_pwm_duty = magnetic_fsm_run(magnetic_fsm_state,magnetic_pwm_duty);
			//2. update the state
			magnetic_fsm_state = magnetic_fsm_transition(magnetic_fsm_state, measured_magnetic_field, measured_magnetic_field);
			
		}
		break;
		default :system_fsm_state = IDLE;
	}
	return;
}

/**
 * \@brief This function updates states for the System FSM
 * 
 * \@param 
 * 
 * \@return new state
 */
int system_fsm_transition(int current_system_fsm_state, int test_start, int test_stop)
{
	int next_state;
	
	if (test_stop) {
		next_state = IDLE;	//else state = idle
		return next_state;
	}
	
	switch (current_system_fsm_state){
		case IDLE : {
			if (test_start) next_state =ADC_UPDATE;
		}
		break;
		case ADC_UPDATE:{
			next_state = SERIAL_UPDATE;
		}
		break;
		case SERIAL_UPDATE:{
			next_state = Heater_Update;
		}
		break;
		case Heater_Update:{
			next_state = Magnetic_Field_update;
		}
		break;
		case Magnetic_Field_update:{
			next_state = ADC_UPDATE;
		}
		break;
		default :next_state = IDLE;
	}
	return next_state;
}




#endif /* SYSTEM_FSM_H_ */