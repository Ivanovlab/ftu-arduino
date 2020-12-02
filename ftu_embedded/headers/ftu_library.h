/*
 * ftu_library.h
 *
 * Created: 2020-12-02 11:25:07 AM
 *  Author: Rohit
 */ 


#ifndef FTU_LIBRARY_H_
#define FTU_LIBRARY_H_


/************************************************************************/
/* GLOBAL VARIABLES AND CONSTANTS                                       */
/************************************************************************/
//system FSM states
#define IDLE 0 //idle state
#define ADC_UPDATE 1 //reading data from the ADC using SPI at intervals
#define SERIAL_UPDATE 2 //sending data back to the computer through serial at intervals
#define Heater_Update 3 //running the heater_FSM and get the updates
#define Magnetic_Field_update 4 //running the magnetic field FSM and get the updates

//volatile is for variables that can always change in the code
volatile int test_time_count = 0;     //seconds of test time elapsed
volatile int system_state = IDLE;    //initialize System FSM state

//Input data from the MCU
volatile int test_id = 0;
volatile bool test_start = false;
volatile bool test_stop = false;

volatile float desired_temperature = 43;     //in C, Set to a default value but actual value comes from MCU instructions
volatile float desired_magnetic_field = 0;  //in mT, Set to a default value but actual value comes from MCU instructions
volatile int desired_time_for_test = 2;	 //time in minutes, Set to a default value but actual value comes from MCU instructions
volatile float desired_fpga_voltage =3200;     //in mV, Set to a default value but actual value comes from MCU instructions
volatile float serial_output_rate=3000;                 //rate to read data in ms, Set to a default value but actual value comes from MCU instructions


volatile bool test_error = false;
String error_message = ""; //Contains the error message to be sent to python, this catches errors
volatile bool serial_signal = false;

volatile float measured_temperature = 0; // in C
volatile float measured_magnetic_field = 0; // in mT
volatile float current_test_time = 0; //test time in minutes
//Variables for Magnetic field FSM
volatile int magnetic_pwm_duty; //duty cycle value 0-255
volatile int magnetic_fsm_state ; //idle state
//Variables for heater FSM
volatile float heater_pwm_duty; //duty cycle value 0-255
volatile int heater_fsm_state; //idle state
volatile int starting_test_count;


/**
 * \@brief   Function converts millivolts from the temperature sensor output to temperature in Celsius
 *			Formula : T = (V_out - V_offset)/Tc) + T_INFL
 * 
 * \@see Data sheet: "https://www.ti.com/lit/ds/symlink/tmp235.pdf?ts=1594142817615&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FTMP235"
 *
 * \@param milli_volts -> Digital Voltage from ADC
 * 
 * \@return float -> Temperature conversion in C from TMP235
 */
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


/**
 * \@brief  Converts voltage to corresponding magnetic field (reference is for the  magnetic sensor)
 *
 * \@see Data sheet: "https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&ved=2ahUKEwjxps3qg8PqAhXBo54KHUn5CvwQFjAAegQIBRAB&url=https%3A%2F%2Fwww.allegromicro.com%2F~%2Fmedia%2FFiles%2FDatasheets%2FA1318-A1319-Datasheet.ashx&usg=AOvVaw39zGCju7QuDLgpcH9PKde_"
 * 
 * \@param milli_volts
 * 
 * \@return float  -> Magnetic field density in milli_Tesla (1G = 0.1mT)
 */
float millivolts_to_milliTesla(double milli_volts){
	#define QUIESCENT_VOLTAGE 1650              //1.65V Low->1.638, mean-> 1.65, high-> 1.662 measure this
	#define MAGNETIC_SENSITIVITY 13.5           //1.35mv/G --> 13.5mV/mT Low->1.289, mean-> 1.3, high-> 1.411
	#define TEMPERATURE_SENSITIVITY   0.12      //0.12%/C
	#define magnetic_sensor_error 1.5							//1.5% ERROR
	#define VOLTAGE_CLAMP_HIGH 2970			    //2.97V +40 mT
	#define VOLTAGE_CLAMP_LOW 0.33			    //2.97V -40mT
	#define MAX_MAGNETIC_FIELD 40              //40mT
	#define MIN_MAGNETIC_FIELD -40              //-40mT
	
	if (milli_volts == '\0') return '\0';
	else if (milli_volts == VOLTAGE_CLAMP_HIGH ) return MAX_MAGNETIC_FIELD ; //we can find some sort of error to throw here
	else if (milli_volts == VOLTAGE_CLAMP_LOW) return MIN_MAGNETIC_FIELD ;
	else return (double)((milli_volts-QUIESCENT_VOLTAGE)/(float)MAGNETIC_SENSITIVITY);   //polarity depends on the direction of the field
}

/************************************************************************/
/* OTHER FUNCTIONS                                                      */
/************************************************************************/

/**
 * \@brief Function raises MCU error, updates the error message
 * 
 * \@param error_text -> Error message as String
 * 
 * \@return void
 */
void raise_mcu_error(String error_text){
	//This function raises and error
	test_error = true;
	error_message.concat(error_text + " ");
	return;
}

#endif /* FTU_LIBRARY_H_ */