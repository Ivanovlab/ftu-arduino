/*
 * magnetic_field_fsm.h
 *
 * Created: 2020-12-02 11:16:51 AM
 *  Author: Rohit
 */ 


#ifndef MAGNETIC_FIELD_FSM_H_
#define MAGNETIC_FIELD_FSM_H_

/************************************************************************/
/* MAGNETIC FIELD FSM                                                   */
/************************************************************************/

#define magnetic_fsm_idle_state 0
#define magnetic_fsm_increase_state 1
#define magnetic_fsm_decrease_state 2
#define magnetic_error 0.015 //error is 1.5% gotten from the magnetic field data sheet in mT

/**
 * \@brief  This function updates the states for the state machine of the Magnetic circuit program--
 *		   An external function is needed to keep updating the actual and desired Magnetic field values
 * 
 * \@param current_state
 * 
 * \@return int next state
 */
int magnetic_fsm_transition(int current_state, float measured_magnetic_field, float desired_magnetic_field)
{
	float magnetic_field_difference = (float) (measured_magnetic_field - desired_magnetic_field); //getting the error
	int next_state;
	
	 switch(current_state){
		 case magnetic_fsm_idle_state:
		 case magnetic_fsm_increase_state:
		 case magnetic_fsm_decrease_state: {
			 if ( magnetic_field_difference > magnetic_error) next_state = magnetic_fsm_decrease_state; //reduce PWM duty if above desired + threshold
			 else if ( abs(magnetic_field_difference) <= magnetic_error) next_state = magnetic_fsm_idle_state; //maintain PWM if between threshold
			 else next_state = magnetic_fsm_increase_state; //PWM duty increase if below the desired+threshold
		 }
		default: next_state = magnetic_fsm_idle_state; 
	 }
	
	return next_state;
}

/**
 * \@brief This function prepares the JSON document that is to be sent to the serial
 * 
 * \@param test_id
 * \@param test_stop
 * \@param test_start
 * \@param test_error
 * \@param error_message
 * \@param adc_data
 * \@param test_time
 * \@param temperature
 * \@param measured_magnetic_field
 * 
 * \@return void
 */
void update_json_doc(int test_id, bool test_stop, bool test_start, bool test_error, String error_message, 
double adc_data[], float test_time, float temperature, float magnetic_field){
	//Preparing json file
	String str_testid = "test id";
	String str_teststop = "test stop";
	String str_testeror = "test error";
	String str_errormessage = "error message";
	String str_ADCdatastring = "ADC data";
	String str_testdata = "test data";
				
	doc.clear(); //clear the document as this frees up the memory
	// Add values to the document

	
	doc[str_testid] = test_id;
	//doc["test stop"] = TEST_STOP; //1 test is stopped, 0 test is running
	if (test_stop) doc[str_teststop] = 1;
	else doc[str_teststop] = 0;
	//doc["test error"] = TEST_ERROR; //0 no error, 1 there was an error
	if (test_error) doc[str_testeror]=1;
	else doc[str_testeror]=0;
				
	doc[str_errormessage] = error_message;
	doc["number of adc arrays"] = number_of_adc_arrays;
	
	for (int i=0;i<number_of_adc_arrays;i++){
		//send multiple arrays in adc doc
		str_ADCdatastring.concat(String(i));
		str_testdata.concat(String(i));
			
		// Add an array.
		JsonArray ADCdata = doc.createNestedArray(str_ADCdatastring);
		for (int i =8; i < 29; i++){ //starting from index 8 as that is how much we use in the ADC array, this is to save space
			ADCdata.add(adc_data[i]);
		}
		//add another array
		JsonArray TESTdata = doc.createNestedArray(str_testdata);

		TESTdata.add(test_time); //current test time
		TESTdata.add(temperature); //temperature (C)
		TESTdata.add(magnetic_field); //magnetic field (mT)
		//out current_test_timer, ADC converted data at intervals
		
		str_testid.remove(str_testid.lastIndexOf(String(i)));
		str_teststop.remove(str_teststop.lastIndexOf(String(i)));
		str_testeror.remove(str_testeror.lastIndexOf(String(i)));
		str_errormessage.remove(str_errormessage.lastIndexOf(String(i)));
		str_ADCdatastring.remove(str_ADCdatastring.lastIndexOf(String(i)));
		str_testdata.remove(str_testdata.lastIndexOf(String(i)));
	}
	
	return;
}

/**
 * \@brief Function runs the magnetic Helmholtz coil
 * 
 * \@param current_state
 * \@param pwm_duty
 * 
 * \@return int magnetic_pwm_duty
 */
int magnetic_fsm_run(int current_state, int pwm_duty)
{
	if (current_state == magnetic_fsm_idle_state){	} //no change
	else if (current_state == magnetic_fsm_increase_state) 
	{
		//PWM is increased until desired state
		if (pwm_duty >= analog_resolution) pwm_duty=analog_resolution;
		else pwm_duty++;
	}
	else if (current_state == magnetic_fsm_decrease_state) 
	{
		//PWM is decreased until desired state
		if (pwm_duty <= 0) pwm_duty=0;
		else pwm_duty--;
	}
	analogWrite(helmholtz_pwm, pwm_duty); //write to the pin after changing the duty
	
	return pwm_duty;
}



#endif /* MAGNETIC_FIELD_FSM_H_ */