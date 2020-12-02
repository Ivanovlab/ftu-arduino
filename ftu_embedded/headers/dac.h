/*
 * dac.h
 *
 * Created: 2020-12-02 11:32:22 AM
 *  Author: Rohit
 */ 


#ifndef DAC_H_
#define DAC_H_


/************************************************************************/
/* DAC FUNCTIONS                                                        */
/************************************************************************/

/**
 * \@brief This function converts a user entered voltage value into a 10 bit DAC value
 * 
 * \@param volt -> desired voltage in mini_volts
 * 
 * \@return int -> DAC value
 */
int set_dac(float volt) {
	//formula for calculating DAC output voltage Vdac = (dVal / 1023)*2.23V
	return (int)((volt*1023)/2230);
}




#endif /* DAC_H_ */