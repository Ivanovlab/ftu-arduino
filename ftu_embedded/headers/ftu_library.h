#ifndef FTU_LIBRARY_H_
#define FTU_LIBRARY_H_
#define IDLE 0
#define ADC_UPDATE 1
#define DAC_UPDATE 2
#define HEATER_UPDATE 3
#define SERIAL_UPDATE 4
#define BAUD_RATE 115200

volatile int system_state = IDLE;
volatile int number_of_HSR_channels = 3; 
volatile int HSR_samples_per_channel = 10;//high sample rate (fixed scan)
volatile int LSR_samples_per_channel = 3;//low sample rate (auto scan)
volatile int serial_rate_desired, time_desired, temp_desired, vstr_desired;
volatile int temp_pwm, vstr_dac;
volatile bool en_vstress = false;
volatile bool en_temp = false;
volatile bool serial_signal = false;
volatile bool idle = true;

float temp_measured, vstr_measured;
float adc_vref = 4.32; //this updates once we get adc_auto_array[][18]
float sample_rate_array[2] = {0};
float adc_auto_array[10][19] = {0}; //allocate memory for array with max dimensions
uint16_t adc_fixed_array[5][1000] = {0};

int twos_complement_to_int (int value){
	if (16 <= 2 || 16 == '\0' || value > (pow(2,16)-1)) return '\0'; //error checking
	else if (value >> (16-1)) return (( value & (int)(pow(2,16-1)-1)) - (pow(2,16-1))); //negative values
	else return value; //positive values
}

int counter_value (float clock_frequency_MHz, float prescaler, float period_ms){

	return (int)(((clock_frequency_MHz*1000*period_ms)/(prescaler))-1);
}

float millivolt_to_celcius(float milli_volts){
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

float average_of_readings(float readings[], int size){
	float average=0;
	for(int i=0; i<size; i++) average+=readings[i];
	return (float)average/size;
}

void send_data_to_serial(float adc_auto_array[][19], uint16_t adc_fixed_array[][1000], float sample_rate_array[]){
	for (int i=0; i<number_of_HSR_channels; i++){
		switch(i){
			case 0: Serial.print("\nF1,"); //CH3-CH4 (DIFF1)
			break;
			case 1: Serial.print("\nF2,"); //CH5-CH6 (DIFF2)
			break;
			case 2: Serial.print("\nF3,"); //CH0-CH2 (VSTR)
			break;
			case 3: Serial.print("\nF4,"); //CH7-CH2 (SG1)
			break;
			case 4: Serial.print("\nF5,"); //CH8-CH2 (SG2)
			break;
			default: Serial.print("INVALID NUMBER OF CHANNELS");
			break;
		}
		Serial.print(millis()/1000);
		Serial.print(",");
		Serial.print(sample_rate_array[0]);
		Serial.print(",");
		for(int j=0; j<HSR_samples_per_channel; j++) {
			Serial.print(adc_fixed_array[i][j]);
			if(j!=HSR_samples_per_channel-1) Serial.print(",");
		}
	}
	for (int i=0; i<LSR_samples_per_channel; i++){
		Serial.print("\nA,");
		Serial.print(millis()/1000);
		Serial.print(",");
		Serial.print(sample_rate_array[1]);
		Serial.print(",");
		for (int j=0; j<19; j++){
			Serial.print(adc_auto_array[i][j]);
			if(j<18) Serial.print(",");
		}
	}
	Serial.println(); 
}

void receive_test_instructions(void){
	Serial.println("Waiting for JSON instruction at serial port");
	while (Serial.available()==0) continue;
	String received_instruction = Serial.readStringUntil('\n');
	char charBuf[received_instruction.length() + 1];
	received_instruction.toCharArray(charBuf, received_instruction.length()+1);
	DynamicJsonDocument doc(1000);
	deserializeJson(doc, charBuf);
	JsonObject scan = doc["adc_scan"];
	JsonObject temp = doc["heater"];
	JsonObject vstress = doc["v_stress"];
	serial_rate_desired = doc["serial_rate"];
	time_desired = doc["time"];
	number_of_HSR_channels = scan["hsr_channels"];
	HSR_samples_per_channel = scan["hsr_samples"];
	LSR_samples_per_channel = scan["lsr_samples"];
	temp_desired = temp["temperature"];
	en_temp = temp["enable"];
	vstr_desired = vstress["voltage"];
	en_vstress = vstress["enable"];
	if(en_temp) temp_pwm = 100;
	else en_temp = 0;
}

#endif