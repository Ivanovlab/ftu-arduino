#ifndef PIN_DESCRIPTIONS_H_
#define PIN_DESCRIPTIONS_H_
#define vstr A0
#define heater A4
#define _cs_mem 0
#define q9 1
#define q13 2
#define q12 3
#define q11 4
#define clr_freq_divider 5
#define _reset_adc 6
#define clko_adc 7
#define mosi_adc 8
#define sck_adc 9
#define miso_adc 10
#define _drdy_adc 11
#define start_adc 12
#define _cs_adc 13
#define _pwdn_adc 14

void pin_setup(void){
	pinMode(_drdy_adc,INPUT);
    pinMode(miso_adc,INPUT);
    pinMode(q11,INPUT);
    pinMode(q12,INPUT);
    pinMode(q13,INPUT);
    pinMode(q9,INPUT);
    pinMode(vstr,OUTPUT);
	pinMode(heater,OUTPUT);
    pinMode(clr_freq_divider,OUTPUT);
    pinMode(_cs_adc,OUTPUT);
    pinMode(mosi_adc,OUTPUT);
    pinMode(sck_adc,OUTPUT);
    pinMode(start_adc,OUTPUT);
    pinMode(_pwdn_adc,OUTPUT);
    pinMode(_reset_adc,OUTPUT);
    analogWrite(vstr,305);
    analogWrite(heater,0);
    digitalWrite(_reset_adc,HIGH);
    digitalWrite(_pwdn_adc,LOW);
    digitalWrite(start_adc,LOW);
    digitalWrite(clr_freq_divider,LOW);
}

void clock_setup(){
	// Set up the generic clock (GCLK4) used to clock timers
	REG_GCLK_GENDIV = GCLK_GENDIV_DIV(48) |          // Divide the 48MHz clock source by divisor 8: 8MHz/8=1MHz
	GCLK_GENDIV_ID(4);            // Select Generic Clock (GCLK) 4
	while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization
	REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |           // Set the duty cycle to 50/50 HIGH/LOW
	GCLK_GENCTRL_GENEN |         // Enable GCLK4
	GCLK_GENCTRL_SRC_DFLL48M |   // Set the 48MHz clock source
	GCLK_GENCTRL_ID(4);          // Select GCLK4
	while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization
	REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |         // Enable GCLK4 to TC4 and TC5
	GCLK_CLKCTRL_GEN_GCLK4 |     // Select GCLK4
	GCLK_CLKCTRL_ID_TC4_TC5 ; // Feed the GCLK4 to TC4 and TC5
	while (GCLK->STATUS.bit.SYNCBUSY);
	REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |         // Enable GCLK4 to TC4 and TC5
	GCLK_CLKCTRL_GEN_GCLK4 |     // Select GCLK4
	GCLK_CLKCTRL_ID_TCC2_TC3; // Feed the GCLK4 to TCC2 and TC3
	while (GCLK->STATUS.bit.SYNCBUSY);
}

void init_tc3(){
	//initialize TC3 timer, this timer controls the rate at which serial data is sent 
	REG_TC3_COUNT16_CC0 =  counter_value(1,1024,1000/(serial_rate_desired)); // Set the TC3 CC0 register
	while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
	NVIC_EnableIRQ(TC3_IRQn);         // Connect TC3 to Nested Vector Interrupt Controller (NVIC)
	REG_TC3_INTFLAG |= TC_INTFLAG_OVF;        // Clear the interrupt flags
	REG_TC3_INTENSET |= TC_INTENSET_OVF;     // Enable TC3 Match CC0 and CC1 interrupts
	REG_TC3_CTRLA = TC_CTRLA_WAVEGEN_MFRQ | //this makes CC0 register have the top  value before the overflow interrupt
	TC_CTRLA_MODE_COUNT16 |        //set as a 16 bit counter
	TC_CTRLA_PRESCALER_DIV1024 |     // Set prescaler to 1024, with a 1Mhz clock 67 seconds is the slowest we can go
	TC_CTRLA_ENABLE;
	while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
}

void TC3_Handler(){
	if (TC3->COUNT16.INTFLAG.bit.OVF && TC3->COUNT16.INTENSET.bit.OVF){
		serial_signal = true;
		REG_TC3_INTFLAG = TC_INTFLAG_OVF; // Clear the OVF interrupt flag
	}
}

#endif