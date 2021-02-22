#ifndef FTU_LIBRARY_H_
#define FTU_LIBRARY_H_
#define IDLE 0
#define ADC_UPDATE 1
#define DAC_UPDATE 2
#define HEATER_UPDATE 3
#define SERIAL_UPDATE 4
#define BAUD_RATE 9600

volatile int system_state = IDLE;
volatile int serial_rate = 1000;
volatile int number_of_HSR_channels = 8;
volatile int HSR_samples_per_channel = 10;
volatile int LSR_samples_per_channel = 1;
volatile bool en_vstress = false;
volatile bool en_temp = false;
volatile bool serial_signal = false;
volatile bool idle = true;
volatile int time_desired, temp_desired, vstr_desired;
volatile int temp_pwm, vstr_dac;
volatile int current_time, prev_time;
volatile float q13_count, q12_count, q11_count, q9_count;
volatile float temp_measured, vstr_measured;
volatile float prev_temp_error, total_temp_error, rate_temp_error;

double adc_auto_array[10][21] = {0};
double adc_fixed_array[8][100] ={0};
DynamicJsonDocument doc(800);

void receive_test_instructions(void){
	Serial.println("Waiting for JSON instruction at serial port");
	while (Serial.available()==0) continue;
	String received_instruction = Serial.readStringUntil('\n');
	char charBuf[received_instruction.length() + 1];
	received_instruction.toCharArray(charBuf, received_instruction.length()+1);
	deserializeJson(doc, charBuf);
	idle = false;
	JsonObject temp = doc["temp"];
	temp_desired = temp["temperature"];
	en_temp = temp["enable"];
	JsonObject vstress = doc["v_stress"];
	vstr_desired = vstress["voltage"];
	en_vstress = vstress["enable"];
	time_desired = doc["time"];
	serial_rate = doc["serial_rate"];
}	

void send_data_to_serial(double adc_auto_array[][21], double adc_fixed_array[][100]){
	doc.clear();
	JsonArray ADC_HSR = doc.createNestedArray("HSR");
	JsonArray ADC_LSR = doc.createNestedArray("LSR");
	for (int i=0; i<number_of_HSR_channels; i++) for(int j=0; j<HSR_samples_per_channel; j++) ADC_HSR.add(adc_fixed_array[i][j]);
	for (int i=0; i<LSR_samples_per_channel; i++) for (int j=0; j<21; j++) ADC_LSR.add(adc_auto_array[i][j]);
	Serial.begin(BAUD_RATE);
	serializeJson(doc, Serial);
	Serial.flush();
	Serial.write("\n");
	Serial.flush();
	Serial.end();
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

int counter_value (float clock_frequency_MHz,float prescaler,float period_ms){return (int)(((clock_frequency_MHz*1000*period_ms)/(prescaler))-1);}

#endif