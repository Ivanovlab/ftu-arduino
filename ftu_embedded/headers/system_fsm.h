#ifndef SYSTEM_FSM_H_
#define SYSTEM_FSM_H_

void system_fsm_run (int system_fsm_state)
{
	switch (system_fsm_state){
		case IDLE :{
			pin_setup();
		}
		break;
		case ADC_UPDATE:{
			adc_auto_scan(raw_adc_data);
			adc_array_convert(raw_adc_data,converted_adc_data);
		}
		break;	
		case DAC_UPDATE:{
			vstr_measured = (float)converted_adc_data[8];
			float vstr_error = vstr_desired - vstr_measured;
			if (vstr_error > 2.5) vstr_dac++;
			else if (vstr_error < -2.5) vstr_dac--;
			else vstr_dac+=0;
			if (vstr_dac > 1023) vstr_dac = 1023;
			if (vstr_dac < 0) vstr_dac = 0;
			analogWrite(vstr, vstr_dac);
		}
		break;
		case HEATER_UPDATE:{
			temp_sum+=millivolt_to_celcius((float)converted_adc_data[22]);
			temp_samples++;
			if (temp_samples>=10){

				temp_measured = temp_sum/temp_samples;
				temp_samples = 0;
				temp_sum = 0;

				current_time = millis();
				float temp_error = temp_desired - temp_measured;
				float elapsed_time = (float)(current_time - prev_time)/1000;
				total_temp_error += temp_error*elapsed_time;
				rate_temp_error = (temp_error - prev_temp_error)/elapsed_time;
				prev_time = current_time;
				prev_temp_error = temp_error;
				
				if (temp_error > 1.5) temp_pwm++;
				else if (temp_error < -1.5) temp_pwm--;
				else temp_pwm+=0;
				if (temp_pwm > 1000) temp_pwm = 1000;
				if (temp_pwm < 0) temp_pwm = 0;

				analogWrite(heater, temp_pwm);
			}
		}
		break;
		case SERIAL_UPDATE:{
			if (serial_signal) {
				update_json_doc(converted_adc_data,temp_measured);
				send_data_to_serial();
				serial_signal = false;
			}
			if (millis() >= time_desired*1000) {
				Serial.println("FINISHED");
				idle = true;
			}		
		}
		break;
		default :system_fsm_state = IDLE;
	}
	return;
}

int system_fsm_transition(int current_system_fsm_state)
{
	int next_state;

	switch (current_system_fsm_state){
		case IDLE : {
			if (!idle) next_state = ADC_UPDATE;
			else next_state = IDLE;
		}
		break;
		case ADC_UPDATE:{
			if(en_vstress) next_state = DAC_UPDATE;
			else if(en_temp) next_state = HEATER_UPDATE;
			else next_state = SERIAL_UPDATE;
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