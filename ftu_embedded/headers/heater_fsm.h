/*
 * heater_fsm.h
 *
 * Created: 2020-12-02 11:30:10 AM
 *  Author: Rohit
 */ 


#ifndef HEATER_FSM_H_
#define HEATER_FSM_H_

/************************************************************************/
/* HEATER CLICK FSM                                                     */
/************************************************************************/
/*	This is a state machine for the Magnetic circuit program--
	An external function is needed to keep updating the actual and desired Magnetic field values
*/
	#define heater_fsm_OFF 0
	#define heater_fsm_idle 1 //state where temperature is maintained
	#define heater_fsm_heating 2
	#define heater_fsm_cooling 3
	#define heater_fsm_margin 2.5 //error is +/- 2.5C as we are using integers
	#define heater_threshold_temp 50 // above 40C is hot for human touch

/**
 * \@brief Function updates the heater FSM states
 * 
 * \@param current state
 * 
 * \@return int next_state
 */
int heater_fsm_transition(int current_state, int measured_temperature, int desired_temperature)
{	
	int next_state;
	float temperature_difference =  (float)(measured_temperature - desired_temperature);
		
	switch(current_state){
		case heater_fsm_OFF: {
			next_state = heater_fsm_idle;
			break;
		}
		case heater_fsm_cooling:
		case heater_fsm_heating:
		case heater_fsm_idle: {
			if ( temperature_difference > heater_fsm_margin) next_state = heater_fsm_cooling; //cool temp is above desired + threshold
			else if ( abs(temperature_difference) <= heater_fsm_margin) next_state = heater_fsm_idle; //maintain heat if it between 0.5C threshold
			else next_state = heater_fsm_heating; //heat up if is below the desired+threshold
				
			break;
		}

		default: next_state = heater_fsm_OFF;
	}
	return next_state;
}

/**
 * \@brief Function runs the heating system
 * 
 * \@param int heater_state
 * \@param int pwm_duty_cycle 
 * 
 * \@return int pwm_duty_cycle
 */
int heater_fsm_run(int heater_state, int pwm_duty_cycle, int measured_temperature ){
	if (measured_temperature < heater_threshold_temp && digitalRead(blue_led)!=HIGH) digitalWrite(blue_led,HIGH);//turn on safe to touch LED if not high already
	else digitalWrite(blue_led,LOW);  //turn off safe to touch LED
	
	switch(heater_state){
		case heater_fsm_OFF: {
			//all pins off
			digitalWrite(red_led, LOW); //turn of heating signal
			pwm_duty_cycle = 0;
			break;
		}
		case heater_fsm_idle: {
			digitalWrite(red_led, LOW); 
			break;
		}
		case heater_fsm_heating: {
			digitalWrite(red_led,HIGH); //turn on heating signal
			if (pwm_duty_cycle >= analog_resolution) pwm_duty_cycle=analog_resolution;
			else pwm_duty_cycle++;	
			break;
		}
		case heater_fsm_cooling: {
			digitalWrite(red_led, LOW); //turn of heating signal
			if (pwm_duty_cycle <= 0) pwm_duty_cycle=0;
			else pwm_duty_cycle--;	
			break;
		}
		default: pwm_duty_cycle = 0;
	}
	
	analogWrite(heater_pwm,pwm_duty_cycle);
	return pwm_duty_cycle;
}



#endif /* HEATER_FSM_H_ */