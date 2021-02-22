#ifndef ADC_HEADER_H_
#define ADC_HEADER_H_
#define ADC_CLK_SPEED 15700000
#define adc_register_array_size 9 //24 OFFSET, 25-> ADC VCC in mV,26-> ADC TEMP in C, 27-> GAIN V/V, 28-> REF
#define adc_spi_speed 3925000  //6MHZ -> Must be less than 0.5 adc_clk(15.7MHz) best if in bunches of 1/2,1/4,1/8 of the ADC clock use (3.925MHz) or (7.85Mhz)
#define ADC_temp_sensor_coefficient 563 //563uV if ADS1158 and test PCB temperatures are forced together,
#define ADC_GAIN 1

void adc_setup(){
	delay(50);
	digitalWrite(_pwdn_adc,HIGH);
	delay(200);
	digitalWrite(_cs_adc,HIGH);
	delay(150);
	digitalWrite(start_adc,LOW);
	digitalWrite(_reset_adc,LOW);
	delay(150);
	digitalWrite(_reset_adc,HIGH);
	delay(500);
}

void adc_toggle_cs(){
	delayMicroseconds(2);
	digitalWrite(_cs_adc,HIGH);
	delayMicroseconds(2);
	digitalWrite(_cs_adc,LOW);
	delayMicroseconds(2);
}

void adc_auto_scan(double adc_auto_array[][19], int samples_per_channel, double ADC_VREF){
	uint8_t status_byte;
	uint16_t value;
	SPI.begin();
	SPI.beginTransaction(SPISettings(adc_spi_speed,MSBFIRST,SPI_MODE0));
	digitalWrite(_cs_adc,LOW);
	delayMicroseconds(2);
	SPI.transfer(0b01110000); //write multiple commands starting at CONFIG0
	SPI.transfer(0b00000010); //set auto scan and stat (CONFIG0)
	SPI.transfer(0b00000011); //set data rate (CONFIG1)
	SPI.transfer(0b00000000); //values here dont matter in auto-scan mode (MUXSCH)
	SPI.transfer(0b00000000); //select no differential measurements (MUXDIF)
	SPI.transfer(0b11111111); //enable all channel A0-A7 (MUXSG0)
	SPI.transfer(0b11111111); //enable all channel A8-A15 (MUXSG1)
	SPI.transfer(0b00110100); //enable all VREF,GAIN,VCC (SYSRED)
	adc_toggle_cs();
	for (int j=0; j<samples_per_channel; j++){
		for (int i=0; i<19; i++){
			SPI.transfer(0b10000000); //send pulse convert command
			delayMicroseconds(46); //inital delay after every pulse convert - cs toggle delay
			adc_toggle_cs();
			if(i>=0 && i<8) SPI.transfer(0b00110100);  //channel data read command for channel A0-A7 (MUXSG0)
			else if(i>=8 && i<16) SPI.transfer(0b00110101);  //channel data read command for channel A8-A15 (MUXSG1)
			else SPI.transfer(0b00110110);  //internal system reading (SYSRED)	
			status_byte = SPI.transfer(0);
			value = SPI.transfer16(0);
			if(i==16 || i==18) adc_auto_array[j][i] = (double) twos_complement_to_int(value)/3072;
			else if(i==17) adc_auto_array[j][i] = (double) twos_complement_to_int(value)/30720;
			else adc_auto_array[j][i] = (double) (twos_complement_to_int(value)*ADC_VREF)/(30.72*ADC_GAIN);			
		}
	}
	SPI.endTransaction();
}

void adc_fixed_scan(double adc_fixed_array[][150], int samples_per_channel, double ADC_VREF){
	uint8_t status_byte;
	uint16_t value;
	SPI.begin();
	SPI.beginTransaction(SPISettings(adc_spi_speed,MSBFIRST,SPI_MODE0));
	digitalWrite(_cs_adc,LOW);
	delayMicroseconds(2);
	SPI.transfer(0b01110000); //write to registers starting at CONFIG0
	SPI.transfer(0b00100010); //set fixed scan (CONFIG0)
	SPI.transfer(0b00000011); //set data rate (CONFIG1)
	adc_toggle_cs();
	for (int i=0; i<8; i++){ //the "high sample rate (HSR)" array to be sent to serial port filled once/serial_rate
		SPI.transfer(0b01100010); //write to MUXSCH
		switch(i) {
			case 0: SPI.transfer(0b00000010); //VSTR1(CH0) - GND(CH2) MOST SIGNIFICANT
			break;
			case 1: SPI.transfer(0b00010010); //VSTR2(CH1) - GND(CH2)
			break;
			case 2: SPI.transfer(0b00110100); //HS_DIFF1_POS(CH3) - HS_DIFF1_NEG(CH4)
			break;
			case 3: SPI.transfer(0b01010110); //HS_DIFF2_POS(CH5) - HS_DIFF2_NEG(CH6)
			break;
			case 4: SPI.transfer(0b01110010); //HS_SG1(CH7) - GND(CH2)
			break;
			case 5: SPI.transfer(0b10000010); //HS_SG2(CH8) - GND(CH2)
			break;
			case 6: SPI.transfer(0b10010010); //HS_SG3(CH9) - GND(CH2)
			break;
			case 7: SPI.transfer(0b11000010); //HS_SG3(CH12) - GND(CH2) LEAST SIGNIFICANT
			break;
			default: SPI.transfer(0b00100010); //GND(CH2) - GND(CH2)//choose 
		}
		digitalWrite(start_adc,HIGH); //start conversion
		delayMicroseconds(52); //initial delay - cs toggle delay
		adc_toggle_cs();
		for(int j=0; j<samples_per_channel; j++){
			SPI.transfer(0b00110010);  //channel data read command out of MUXSCH
			status_byte = SPI.transfer(0);
			value = SPI.transfer16(0);
			adc_fixed_array[i][j] = (double)(twos_complement_to_int(value)*ADC_VREF)/(30.72*ADC_GAIN);
			delayMicroseconds(10); //delay based off data rate selected
		}
	}
	digitalWrite(start_adc,LOW); //stop conversion
	digitalWrite(_cs_adc,HIGH);
	SPI.endTransaction();
}

#endif