#ifndef SYSTEM_FSM_H_
#define SYSTEM_FSM_H_

void system_fsm_run (int system_fsm_state){

	switch (system_fsm_state){
		case IDLE : pin_setup();
		break;
		case ADC_UPDATE:{
			adc_auto_scan(adc_auto_array,LSR_samples_per_channel,adc_vref);
			adc_fixed_scan(adc_fixed_array,HSR_samples_per_channel,adc_vref);
			double vref_array[10] = {0};
			for(int i=0; i<LSR_samples_per_channel; i++) vref_array[i] = adc_auto_array[i][18];
			adc_vref = average_of_readings(vref_array, LSR_samples_per_channel);
		}
		break;	
		case DAC_UPDATE:{
			double vstr_array[20] = {0};
			for(int i=0; i<LSR_samples_per_channel; i++) vstr_array[i] = adc_auto_array[i][0];
			for(int i=LSR_samples_per_channel; i<LSR_samples_per_channel*2; i++) vstr_array[i] = adc_auto_array[i-LSR_samples_per_channel][1];
			vstr_measured = average_of_readings(vstr_array, LSR_samples_per_channel*2);
			float vstr_error = vstr_desired - vstr_measured;
			if (vstr_error > 50) vstr_dac=vstr_dac+10;
			else if (vstr_error > 4) vstr_dac++;
			else if (vstr_error < -50) vstr_dac=vstr_dac-10;
			else if (vstr_error < -4) vstr_dac--;
			if (vstr_dac > 700) vstr_dac = 700;
			if (vstr_dac < 0) vstr_dac = 0;
			analogWrite(vstr, vstr_dac);
		}
		break;
		case HEATER_UPDATE:{
			double temp_array[10] = {0};
			for(int i=0; i<LSR_samples_per_channel; i++) temp_array[i] = adc_auto_array[i][14];
			temp_measured = millivolt_to_celcius(average_of_readings(temp_array,LSR_samples_per_channel));
			//current_time = millis();
			//float temp_error = temp_desired - temp_measured;
			//float elapsed_time = (float)(current_time - prev_time)/1000;     //temp PID control unused
			//total_temp_error += temp_error*elapsed_time;
			//rate_temp_error = (temp_error - prev_temp_error)/elapsed_time;
			//prev_time = current_time;
			//prev_temp_error = temp_error;	
			if (temp_error > 1.5) temp_pwm++;
			else if (temp_error < -1.5) temp_pwm--;
			if (temp_pwm > 1000) temp_pwm = 1000;
			if (temp_pwm < 0) temp_pwm = 0;
			analogWrite(heater, temp_pwm);
		}
		break;
		case SERIAL_UPDATE:{
			send_data_to_serial(adc_auto_array,adc_fixed_array);
			if (millis()>time_desired*1000) {
				Serial.println("FINISHED");
				idle = true;
			}
			serial_signal = false;
		}
		break;
		default :system_fsm_state = IDLE;
	}
	return;
}

int system_fsm_transition(int current_system_fsm_state){

	int next_state;
	switch (current_system_fsm_state){
		case IDLE : {
			if (!idle) next_state = ADC_UPDATE;
			else next_state = IDLE;
		}
		break;
		case ADC_UPDATE:{
			if(serial_signal){
				if(en_vstress) next_state = DAC_UPDATE;
				else if(en_temp) next_state = HEATER_UPDATE;
				else next_state = SERIAL_UPDATE;
			}
			else next_state = ADC_UPDATE;
		}
		break;
		case DAC_UPDATE:{
			if(en_temp) next_state = HEATER_UPDATE;
			else next_state = SERIAL_UPDATE;
		}
		break;
		case HEATER_UPDATE:{
			next_state = SERIAL_UPDATE;
		}
		break;
		case SERIAL_UPDATE:{
			if(idle) next_state = IDLE;
			else next_state = ADC_UPDATE;
		}
		break;
		default: next_state = IDLE;
	}
	return next_state;
}

#endif