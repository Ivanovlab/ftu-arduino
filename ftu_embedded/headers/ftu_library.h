#ifndef FTU_LIBRARY_H_
#define FTU_LIBRARY_H_

#define IDLE 0
#define ADC_UPDATE 1
#define DAC_UPDATE 2
#define HEATER_UPDATE 3
#define SERIAL_UPDATE 4
#define BAUD_RATE 9600
#define number_of_adc_arrays 1

volatile int system_state = IDLE;
volatile int serial_rate = 1000;
volatile bool en_vstress = false;
volatile bool en_temp = false;
volatile bool test_error = false;
volatile bool serial_signal = false;
volatile bool idle = true;
volatile int time_desired, temp_desired, vstr_desired;
volatile int temp_pwm, temp_samples, vstr_dac;
volatile int current_time, prev_time;
volatile float q13_count, q12_count, q11_count, q9_count;
volatile float temp_measured, temp_sum, vstr_measured;
volatile float prev_temp_error, total_temp_error, rate_temp_error;

String error_message = "";
DynamicJsonDocument  doc(800*number_of_adc_arrays); //650 per doc

void raise_mcu_error(String error_text)
{
	test_error = true;
	error_message.concat(error_text + " ");
}

void receive_test_instructions(void)
{
	Serial.println("Waiting for JSON instruction at serial port");
	while (Serial.available()==0) continue;
	String received_instruction = Serial.readStringUntil('\n');
	char charBuf[received_instruction.length() + 1];
	received_instruction.toCharArray(charBuf, received_instruction.length()+1);
	DeserializationError error = deserializeJson(doc, charBuf); // De serialize the JSON document
	if (error) raise_mcu_error(error.c_str());

	JsonObject temp = doc["temp"];
	temp_desired = temp["temperature"];
	en_temp = temp["enable"];

	JsonObject vstress = doc["v_stress"];
	vstr_desired = vstress["voltage"];
	en_vstress = vstress["enable"];

	time_desired = doc["time"];
	serial_rate = doc["serial_rate"];

	Serial.print("\nSerial Rate: ");
	Serial.print(serial_rate);
	Serial.print("\nTest Time: ");
	Serial.print(time_desired);
	Serial.print("s\nHeater Temperature: ");
	Serial.print(temp_desired);
	Serial.print("C\nStress Voltage: ");
	Serial.print(vstr_desired);
	Serial.print("mV\n\n");
	idle = false;
}	

void update_json_doc(double adc_data[], float temperature)
{
	String str_ADCdatastring = "ADC";
	String str_temp = "Temp";		
	doc.clear();
	doc["temp"] = temperature;
	JsonArray ADCdata = doc.createNestedArray(str_ADCdatastring); //adc_data[0-7] are differential readings between channel 0-1, 2-3 ect..
	ADCdata.add(adc_data[8]);
	ADCdata.add(adc_data[9]);
	ADCdata.add(adc_data[10]);
	//ADCdata.add(adc_data[11]);
	//ADCdata.add(adc_data[12]);
	//ADCdata.add(adc_data[13]);
	//ADCdata.add(adc_data[14]);
	//ADCdata.add(adc_data[15]);
	//ADCdata.add(adc_data[16]);
	//ADCdata.add(adc_data[17]);
	//ADCdata.add(adc_data[18]);
	//ADCdata.add(adc_data[19]);
	//ADCdata.add(adc_data[20]);
	//ADCdata.add(adc_data[21]);
	ADCdata.add(adc_data[22]);
	//ADCdata.add(adc_data[23]); idk why channel 15 not worky
}

void send_data_to_serial()
{
	Serial.begin(BAUD_RATE);
	serializeJson(doc, Serial);
	Serial.flush();
	Serial.write("\n");
	Serial.flush();
	Serial.end();
}

float millivolt_to_celcius(float milli_volts)
{
	#define V_OFFSET_0 500
	#define V_OFFSET_100 1500
	#define V_OFFSET_125 1752.5
	#define T_INFL_0 0
	#define T_INFL_100 100
	#define T_INFL_125 125
	#define T_COEFF_0 10
	#define T_COEFF_100 10.1
	#define T_COEFF_125 10.6

	if (milli_volts < V_OFFSET_100) return (((milli_volts - V_OFFSET_0)/T_COEFF_0) + T_INFL_0); // -40 to 100C
	else if (milli_volts >= V_OFFSET_100 && milli_volts <= V_OFFSET_125) return (((milli_volts - V_OFFSET_100)/T_COEFF_100) + T_INFL_100); // 100 to 125C
	else return (((milli_volts - V_OFFSET_125)/T_COEFF_125) + T_INFL_125); // 125 to 150C
}

float millivolts_to_milliTesla(double milli_volts)
{
	#define QUIESCENT_VOLTAGE 1650              //1.65V Low->1.638, mean-> 1.65, high-> 1.662 measure this
	#define MAGNETIC_SENSITIVITY 13.5           //1.35mv/G --> 13.5mV/mT Low->1.289, mean-> 1.3, high-> 1.411
	#define TEMPERATURE_SENSITIVITY   0.12      //0.12%/C
	#define magnetic_sensor_error 1.5			//1.5% ERROR
	#define VOLTAGE_CLAMP_HIGH 2970			    //2.97V +40 mT
	#define VOLTAGE_CLAMP_LOW 0.33			    //2.97V -40mT
	#define MAX_MAGNETIC_FIELD 40               //40mT
	#define MIN_MAGNETIC_FIELD -40              //-40mT
	
	if (milli_volts == '\0') return '\0';
	else if (milli_volts == VOLTAGE_CLAMP_HIGH ) return MAX_MAGNETIC_FIELD ; //we can find some sort of error to throw here
	else if (milli_volts == VOLTAGE_CLAMP_LOW) return MIN_MAGNETIC_FIELD ;
	else return (double)((milli_volts-QUIESCENT_VOLTAGE)/(float)MAGNETIC_SENSITIVITY);   //polarity depends on the direction of the field
}

#endif