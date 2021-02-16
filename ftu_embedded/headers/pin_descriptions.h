#ifndef PIN_DESCRIPTIONS_H_
#define PIN_DESCRIPTIONS_H_
#define vstr A0
#define heater A4
#define helmholtz A3
#define red_led A5
#define blue_led A6
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

void pin_setup(void)
{
	pinMode(vstr,OUTPUT);
	analogWrite(vstr,205);
    pinMode(heater,OUTPUT);
    analogWrite(heater,0);
    pinMode(helmholtz,OUTPUT);
    analogWrite(helmholtz,0);
    pinMode(_reset_adc,OUTPUT); //active low
    digitalWrite(_reset_adc,HIGH); //setting it off
    pinMode(_pwdn_adc,OUTPUT); //active low
    digitalWrite(_pwdn_adc,LOW); //setting it off
    pinMode(start_adc,OUTPUT); //active high
    digitalWrite(start_adc,LOW); //setting it off
    pinMode(_drdy_adc,INPUT);
    pinMode(red_led,OUTPUT);
    digitalWrite(red_led,HIGH);
    pinMode(blue_led,OUTPUT);
    digitalWrite(blue_led,HIGH);
    pinMode(clr_freq_divider,OUTPUT);
    digitalWrite(clr_freq_divider,LOW);
    pinMode(_cs_adc,OUTPUT);
    pinMode(mosi_adc,OUTPUT);
    pinMode(sck_adc,OUTPUT);
    pinMode(miso_adc,INPUT);
    pinMode(q11,INPUT);
    pinMode(q12,INPUT);
    pinMode(q13,INPUT);
    pinMode(q9,INPUT);
}

void clock_setup()
{
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

int counter_value (float clock_frequency_MHz,float prescaler, float period_ms) {return (int) (( (clock_frequency_MHz*1000*period_ms) / (prescaler) )-1);}

void init_tc3()
{
	//initialize TC3 timer, this timer controls the rate at which serial data is sent 
	REG_TC3_COUNT16_CC0 =  counter_value(1,1024,serial_rate); // Set the TC3 CC0 register
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

void init_tc4()
{
	// Configure TC4 (16 bit counter by default)
	REG_TC4_COUNT16_CC0 = counter_value(1,256,200);  //set period for timer
	while (TC4->COUNT16.STATUS.bit.SYNCBUSY);
	NVIC_EnableIRQ(TC4_IRQn);         // Connect TC4 to Nested Vector Interrupt Controller (NVIC)
	REG_TC4_INTFLAG |= TC_INTFLAG_OVF; //TC_INTFLAG_MC1 | TC_INTFLAG_MC0 ;        // Clear the interrupt flags (overflow flag)
	REG_TC4_INTENSET |= TC_INTENSET_OVF; //TC_INTENSET_MC1 | TC_INTENSET_MC0 ;     // Enable TC4 counter interrupts
	REG_TC4_CTRLA = TC_CTRLA_WAVEGEN_MFRQ | //this makes CC0 register have the top  value before the overflow interrupt
	TC_CTRLA_MODE_COUNT16 |         //set as a 16 bit counter
	TC_CTRLA_PRESCALER_DIV256 |     // Set prescaler to 256,
	TC_CTRLA_ENABLE;               // Enable TC4
	while (TC4->COUNT16.STATUS.bit.SYNCBUSY);
}

void init_tc5()
{
	REG_TC5_COUNT16_CC0 =  counter_value(1,64,0.1);// Set the TC4 CC1 register to  decimal 3905, 1 msec
	while (TC5->COUNT16.STATUS.bit.SYNCBUSY);
	NVIC_EnableIRQ(TC5_IRQn);         // Connect TC4 to Nested Vector Interrupt Controller (NVIC)
	NVIC_SetPriority(TC5_IRQn,0);
	REG_TC5_INTFLAG |= TC_INTFLAG_OVF;        // Clear the interrupt flags
	REG_TC5_INTENSET |= TC_INTENSET_OVF;     // Enable TC5 Match CC0 and CC1 interrupts
	REG_TC5_CTRLA = TC_CTRLA_WAVEGEN_MFRQ | //this makes CC0 register have the top  value before the overflow interrupt
	TC_CTRLA_MODE_COUNT16 |        //set as a 16 bit counter
	TC_CTRLA_PRESCALER_DIV64 |     
	TC_CTRLA_ENABLE;
	while (TC5->COUNT16.STATUS.bit.SYNCBUSY);
}

void TC3_Handler()
{
	if (TC3->COUNT16.INTFLAG.bit.OVF && TC3->COUNT16.INTENSET.bit.OVF)
	{
		serial_signal = true;
		REG_TC3_INTFLAG = TC_INTFLAG_OVF;        // Clear the OVF interrupt flag
	}
}

void TC4_Handler()
{ 
	if (TC4->COUNT16.INTFLAG.bit.OVF && TC4->COUNT16.INTENSET.bit.OVF)
	{
		REG_TC4_INTFLAG = TC_INTFLAG_OVF;
	}
}

void TC5_Handler()
{
	int count;
	if (TC5->COUNT16.INTFLAG.bit.OVF && TC5->COUNT16.INTENSET.bit.OVF)
	{
		count++;
		if(digitalRead(q13)&&digitalRead(q12)&&digitalRead(q11))//14336 pulses
		{
			digitalWrite(clr_freq_divider,HIGH);

			q13_count = count;
			count = 0;
		}
		else digitalWrite(clr_freq_divider,LOW);
		REG_TC5_INTFLAG = TC_INTFLAG_OVF;
	}
}

#endif