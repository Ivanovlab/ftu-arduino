#ifndef ADC_HEADER_H_
#define ADC_HEADER_H_
#define ADC_CLK_SPEED 15700000
#define adc_spi_speed 7850000  //6MHZ -> Must be less than 0.5 adc_clk(15.7MHz) best if in bunches of 1/2,1/4,1/8 of the ADC clock use (3.925MHz) or (7.85Mhz)
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

void adc_invert_cs(bool is_high){
	if(is_high) digitalWrite(_cs_adc,LOW);
	else digitalWrite(_cs_adc,HIGH);
	delayMicroseconds(2);
}

float adc_auto_scan(float ADC_VREF, float adc_auto_array[][19], int samples_per_channel){
	int time_before, time_after;
	float time_elapsed=0;
	uint8_t status_byte;
	uint16_t value;
	SPI.begin();
	SPI.beginTransaction(SPISettings(adc_spi_speed,MSBFIRST,SPI_MODE0));
	adc_invert_cs(true);
	SPI.transfer(0b01110000); //write multiple commands starting at CONFIG0
	SPI.transfer(0b00000010); //set auto scan and stat (CONFIG0)
	SPI.transfer(0b00000001); //set data rate (CONFIG1)
	SPI.transfer(0b00000000); //values here dont matter in auto-scan mode (MUXSCH)
	SPI.transfer(0b00000000); //select no differential measurements (MUXDIF)
	SPI.transfer(0b11111111); //enable all channel A0-A7 (MUXSG0)
	SPI.transfer(0b11111111); //enable all channel A8-A15 (MUXSG1)
	SPI.transfer(0b00110100); //enable all VREF,GAIN,VCC (SYSRED)
	adc_toggle_cs();
	for (int j=0; j<samples_per_channel; j++){
		for (int i=0; i<19; i++){
			time_before = micros();
			SPI.transfer(0b10000000); //send pulse convert command
			delayMicroseconds(168); //inital delay after every pulse convert
			adc_toggle_cs();
			if(i>=0 && i<8) SPI.transfer(0b00110100);  //channel data read command for channel A0-A7 (MUXSG0)
			else if(i>=8 && i<16) SPI.transfer(0b00110101);  //channel data read command for channel A8-A15 (MUXSG1)
			else SPI.transfer(0b00110110);  //internal system reading (SYSRED)	
			status_byte = SPI.transfer(0);
			value = SPI.transfer16(0);
			if(i==16 || i==18) adc_auto_array[j][i] = (float) twos_complement_to_int(value)/3072; //vcc or vref same conversion
			else if(i==17) adc_auto_array[j][i] = (float) twos_complement_to_int(value)/30720; //gain conversion
			else adc_auto_array[j][i] = (float) (twos_complement_to_int(value)*ADC_VREF)/(30.72*ADC_GAIN);
			time_after = micros();
			time_elapsed += (time_after-time_before);
		}
	}
	digitalWrite(_cs_adc,HIGH);
	SPI.endTransaction();
	return 1000000/(time_elapsed/(19*samples_per_channel));
}

float adc_fixed_scan(float ADC_VREF, uint16_t adc_fixed_array[][1000], int number_of_channels, int samples_per_channel){
	uint8_t status_byte;
	int time_before, time_after;
	float time_elapsed=0;
	SPI.begin();
	SPI.beginTransaction(SPISettings(adc_spi_speed,MSBFIRST,SPI_MODE0));
	adc_invert_cs(true);
	SPI.transfer(0b01110000); //write to registers starting at CONFIG0
	SPI.transfer(0b00100010); //set fixed scan (CONFIG0)
	SPI.transfer(0b00000011); //set max sample rate (CONFIG1)
	adc_toggle_cs();
	for (int i=0; i<number_of_channels; i++){ //the "high sample rate (HSR)" array to be sent to serial port filled here once/serial_rate
		SPI.transfer(0b01100010); //write to MUXSCH
		switch(i) {
			case 0: SPI.transfer(0b00110100); //HS_DIFF1_POS(CH3) - HS_DIFF1_NEG(CH4)
			break;
			case 1: SPI.transfer(0b01010110); //HS_DIFF2_POS(CH5) - HS_DIFF2_NEG(CH6) 
			break;
			case 2: SPI.transfer(0b00000010); //VSTR1(CH0) - GND(CH2)
			break;
			case 3: SPI.transfer(0b01110010); //HS_SG1(CH7) - GND(CH2)
			break;
			case 4: SPI.transfer(0b10000010); //HS_SG2(CH8) - GND(CH2)
			break;
			default: SPI.transfer(0b00100010); //GND(CH2) - GND(CH2)//choose 
		}
		digitalWrite(start_adc,HIGH); //start conversion
		delayMicroseconds(50);
		adc_toggle_cs();
		for(int j=0; j<samples_per_channel; j++){
			time_before = micros();
			SPI.transfer(0b00110010);  //channel data read command out of MUXSCH
			status_byte = SPI.transfer(0);
			adc_fixed_array[i][j] = SPI.transfer16(0);
			delayMicroseconds(3);
			time_after = micros();
			time_elapsed += (time_after-time_before);
		}
	}
	digitalWrite(start_adc,LOW); //stop conversion
	digitalWrite(_cs_adc,HIGH);
	SPI.endTransaction();
	return 1000000/(time_elapsed/(number_of_channels*samples_per_channel));
}

#endif