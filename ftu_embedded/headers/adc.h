#ifndef ADC_HEADER_H_
#define ADC_HEADER_H_
#define CONFIG0_address 0x00
#define CONFIG1_address 0x01
#define MUXSCH_address 0x02
#define MUXDIF_address 0x03
#define MUXSG0_address 0x04
#define MUXSG1_address 0x05
#define SYSRED_address 0x06
#define GPIOC_address 0x07
#define GPIOD_address 0x08
#define ADC_CLK_SPEED 15700000
#define number_of_bits_adc 16
#define adc_register_array_size 9 //24 OFFSET, 25-> ADC VCC in mV,26-> ADC TEMP in C, 27-> GAIN V/V, 28-> REF
#define adc_spi_speed 3925000  //6MHZ -> Must be less than 0.5 adc_clk(15.7MHz) best if in bunches of 1/2,1/4,1/8 of the ADC clock use (3.925MHz) or (7.85Mhz)
#define ADC_temp_sensor_coefficient 563 //563uV if ADS1158 and test PCB temperatures are forced together,
#define MAX_POSITIVE_VOLTAGE 0x7FFF //394uV if ADS1158 temperature is forced and test PCB is in free air
#define MAX_NEGATIVE_VOLTAGE 0x8000
#define ADC_RANGE 4900 //4900mV (-1.6V to 3.3V)
#define ADC_GAIN 1 //this is the normal ADC GAIN
#define ADC_VREF 4.9 //3.3V //this is VREFP-VREFN
#define CONFIG0_default 0x02
#define CONFIG1_default 0x03 //0x00 default
#define MUXSCH_default 0x00
#define MUXDIF_default 0x00
#define MUXSG0_default 0xFF //0xFF
#define MUXSG1_default 0x7F  //	0x7F
#define SYSRED_default 0x3D  //0x3D if you wanna see measurements
#define GPIOC_default 0x00
#define GPIOD_default 0x00
#define Pulse_convert_command 0x80
#define Reset_command 0xC0

const uint8_t adc_register_addresses[adc_register_array_size] = {CONFIG0_address,CONFIG1_address,MUXSCH_address,
MUXDIF_address,MUXSG0_address,MUXSG1_address,SYSRED_address,GPIOC_address,GPIOD_address};
const uint8_t adc_register_defaults[adc_register_array_size] = {CONFIG0_default,CONFIG1_default,MUXSCH_default,
MUXDIF_default,MUXSG0_default,MUXSG1_default,SYSRED_default,GPIOC_default,GPIOD_default};

uint8_t CONFIG0_value = 0;
uint8_t CONFIG1_value = 0;
uint8_t MUXSCH_value =  0;
uint8_t MUXDIF_value =  0;
uint8_t MUXSG0_value =  0;
uint8_t MUXSG1_value =  0;
uint8_t SYSRED_value =  0;
uint8_t GPIOC_value =   0;
uint8_t GPIOD_value =   0;
uint16_t raw_adc_data [32] = {0};
double converted_adc_data[32] = {0};

//CHID[4:0]  PRIORITY      CHANNEL        DESCRIPTION
//00h      1 (highest)    AIN0�AIN1     Differential 0
//01h      2              AIN2�AIN3     Differential 1
//02h      3              AIN4�AIN5     Differential 2
//03h      4              AIN6�AIN7     Differential 3
//04h      5              AIN8�AIN9     Differential 4
//05h      6              AIN10�AIN11   Differential 5
//06h      7              AIN12�AIN13   Differential 6
//07h      8              AIN14�AIN15   Differential 7
//08h      9              AIN0           Single-ended 0
//09h      10             AIN1           Single-ended 1
//0Ah      11             AIN2           Single-ended 2
//0Bh      12             AIN3           Single-ended 3
//0Ch      13             AIN4           Single-ended 4
//0Dh      14             AIN5           Single-ended 5
//0Eh      15             AIN6           Single-ended 6
//0Fh      16             AIN7           Single-ended 7
//10h      17             AIN8           Single-ended 8
//11h      18             AIN9           Single-ended 9
//12h      19             AIN10          Single-ended 10
//13h      20             AIN11          Single-ended 11
//14h      21             AIN12          Single-ended 12
//15h      22             AIN13          Single-ended 13
//16h      23             AIN14          Single-ended 14
//17h      24             AIN15          Single-ended 15
//18h      25             OFFSET         Offset
//1Ah      26             AVDD�AVSS      Power
//1Bh      27             TEMP           Temp
//1Ch      28             GAIN           Gain
//1Dh      29 (lowest)    REF            External ref

uint8_t adc_register_read(uint8_t reg_address)
{
	uint8_t command = 0x40 | reg_address;
	uint8_t reg_value = NULL;
	SPI.begin(); //initialize SPI pins
	SPI.beginTransaction (SPISettings (adc_spi_speed, MSBFIRST, SPI_MODE0));
	digitalWrite(_cs_adc,LOW);
	delayMicroseconds(2);
	SPI.transfer(command); // send command
	reg_value = SPI.transfer(0x0);   // Read response
	delayMicroseconds(2);
	digitalWrite(_cs_adc,HIGH);
	SPI.endTransaction();
	return reg_value;
}

double adc_mv(int ADC_reading, double vref, double gain)
{
	if ((vref == NULL) || (vref == 0)) vref =  ADC_VREF;
	if (gain == NULL || (gain == 0)) gain = ADC_GAIN;
	//return (ADC_reading*converted_adc_data[28]*1000) / (double) ((double)30720  * converted_adc_data[27]);
	return (((double)ADC_reading*vref*1000) /( (double) 30720.00 * gain) );
}

int twos_complement_to_int (int value, int num_bits)
{
	// msb =  value >> (num_bits-1); //collecting MSB
	if (num_bits <= 2 || num_bits == '\0' || value > (pow(2,num_bits)-1)) return '\0'; //error checking
	else if (value >> (num_bits-1)) return (( value & (int)(pow(2,num_bits-1)-1)) - (pow(2,num_bits-1))); //if msb ==1, negative values
	else return value; //positive values
}

uint8_t adc_chid_status(uint8_t status_byte)
{
	//STATUS_byte BIT
	//|  NEW    |  OVF   |  SUPPLY  |  CHID 4  |  CHID 3  |  CHID 2  |  CHID 1  |  CHID 0  |
	return (uint8_t) (status_byte & 0b00011111); //return last 5 bits of STATUS_byte byte
}

uint8_t adc_return_status_byte(uint32_t raw_channel_register_read_data)
{
	//The input 32bit value contains [31:24] Don't care, [23:16] STATUS byte,[15:8] MSB of measurement, [7:0] LSB of measurement
	//We return status
	return (uint8_t)((raw_channel_register_read_data >> 16)& 0x000000FF); //0X000F = 0b0000_0000_0000_0000_0000_0000_1111_1111, this returns the status byte
}

uint16_t adc_return_raw_data(uint32_t raw_channel_register_read_data)
{
	//The input 32bit value contains [31:24] Don't care, [23:16] STATUS byte,[15:8] MSB of measurement, [7:0] LSB of measurement
	//We return MSB and LSB
	return (uint16_t)((raw_channel_register_read_data )& 0x0000FFFF); //0X0F = 0b0000_0000_0000_0000_1111_1111_1111_1111, this returns the status byte
}

boolean adc_new_status_bit(uint8_t status_byte)
{
	//STATUS_byte BIT
	//|  NEW    |  OVF   |  SUPPLY  |  CHID 4  |  CHID 3  |  CHID 2  |  CHID 1  |  CHID 0  |	
	uint8_t NEW_bit = (status_byte & 0b10000000)>>7;
	if (NEW_bit == 0) return false;
	else return true;
}

boolean adc_ovf_status_bit(uint8_t status_byte)
{
	//STATUS_byte BIT
	//|  NEW    |  OVF   |  SUPPLY  |  CHID 4  |  CHID 3  |  CHID 2  |  CHID 1  |  CHID 0  |
	uint8_t OVF_bit = (status_byte & 0b01000000)>>6;
	if (OVF_bit == 0) return false;
	else return true;
}

uint8_t adc_drate(void) {return ( 0x03 & adc_register_read(CONFIG1_address));}

void adc_array_convert(uint16_t raw_data[], double converted_data[])
{
	//both converted_data and raw_data have sizes of 9
	//ADC GAIN conversion
	converted_data[27] = ((float) raw_data[27]/(double) 30720);
	//ADC REF conversion
	converted_data[28] = ((float) raw_data[28]/(double) 3072);
	// 
	//for OFFSET conversion
	converted_data[24] = raw_data[24]; //Ideally, the code from this register function is 0h, but varies because of the noise of the ADC and offsets stemming from the ADC and
	//external signal conditioning. This register can be used to calibrate or track the offset of the ADS1158 and Figure 40. Conversion Control, Auto-Scan Mode external signal conditioning.
	//for VCC conversion
	converted_data[25] = ((float) raw_data[25]/(double) 3072);
	//for ADC temp conversion
	converted_data[26] =( ((double) ( (1000*adc_mv(twos_complement_to_int(raw_data[26],number_of_bits_adc),converted_adc_data[28], converted_adc_data[27])) - 168000)/(double) ADC_temp_sensor_coefficient)+ 25);
	//for loop is at the bottom so that we can first convert useful constants used in these calculations
	for (int i =8; i <24 ; i++) converted_data[i] = adc_mv( (twos_complement_to_int(raw_data[i],number_of_bits_adc) - twos_complement_to_int(raw_data[24],number_of_bits_adc)),converted_data[28], converted_data[27]); //Vin = (Vref/GainError) * [ (OutputCode - OffsetCode) / 0x7800 ]
	return;
}

boolean adc_supply_status_bit(uint8_t status_byte)
{   //STATUS_byte BIT
	//|  NEW    |  OVF   |  SUPPLY  |  CHID 4  |  CHID 3  |  CHID 2  |  CHID 1  |  CHID 0  |	
	uint8_t SUPPLY_bit = (status_byte & 0b00100000)>>5;
	if (SUPPLY_bit == 0) return false;
	else return true; 
}

void adc_register_write(uint8_t reg_address, uint8_t value)
{
    uint8_t command = 0x60| reg_address; // 8b'0100_0000 | REG_address
    SPI.begin(); //initialize SPI pins
    SPI.beginTransaction (SPISettings (adc_spi_speed, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs_adc,LOW);
    delayMicroseconds(2);
    SPI.transfer(command); // send the command byte
    SPI.transfer(value);   // send the value of register 
    delayMicroseconds(2);
    digitalWrite(_cs_adc,HIGH);
	SPI.endTransaction();
}
  
unsigned int count_set_bits(int n)
{
	unsigned int count = 0;
	while (n)
	{
		n &= (n - 1);
		count++;
	}
	return count;
} 

void adc_send_command(uint8_t command)
{
	SPI.begin(); //initialize SPI pins
	SPI.beginTransaction (SPISettings (adc_spi_speed, MSBFIRST, SPI_MODE0));
	digitalWrite(_cs_adc,LOW);
	delayMicroseconds(2);
	SPI.transfer(command); // send the command byte
	delayMicroseconds(2);
	digitalWrite(_cs_adc,HIGH);
	SPI.endTransaction(); 
}

uint32_t adc_channel_read_register_format(void)
{
	//adc_toggle_start_pin(); //Pulse the start pin, this moves ADC the conversion to the next channel to be converted
	adc_send_command(Pulse_convert_command); //send pulse convert command
			
	//Channel data read - register format
	uint8_t command = 0x30;
	uint32_t STATUS_byte = 0xFFFFFFFF;    //STATUS_byte byte
	uint32_t MSB_data = 0xFFFFFFFF;  //upper 8 bits
	uint32_t LSB_data = 0xFFFFFFFF;  //lower 8 bits
	uint32_t Channel_data = 0;
			
	SPI.begin(); //initialize SPI pins
	SPI.beginTransaction (SPISettings (adc_spi_speed, MSBFIRST, SPI_MODE0));
	digitalWrite(_cs_adc,LOW);
	delayMicroseconds(3); //CS low to first clock is minimum 2.5/15.7Mhz (we need 0.19us) 
			
	SPI.transfer(command); // send command
	STATUS_byte = SPI.transfer(0x0);   // Read STATUS_byte byte
	MSB_data = SPI.transfer(0x0); // Read MSB byte
	LSB_data = SPI.transfer(0x0); // Read LSB byte
			
	delayMicroseconds(2);
	digitalWrite(_cs_adc,HIGH);
	SPI.endTransaction();

	Channel_data |= (STATUS_byte<<16)|(MSB_data<<8)|LSB_data;
	return Channel_data;	
}

float adc_initial_delay_time(void)
{
	float delay_time = 0;
	
	switch( adc_drate() )
	{ // get the adc DRATE
		case 0b00: {
			delay_time = ( (8772.00 * 1000000)/ADC_CLK_SPEED );
			break;
		}
		case 0b01: {
			delay_time = ( (2628.00 * 1000000)/ADC_CLK_SPEED );
			break;
		}
		case 0b10: {
			delay_time = ( (1092.00 * 1000000)/ADC_CLK_SPEED );
			break;
		}
		case 0b11: {
			delay_time = ( (708.00 * 1000000)/ADC_CLK_SPEED );
			break;
		}

		default: delay_time = ( (8772.00 * 1000000)/ADC_CLK_SPEED );
	}
	return delay_time;
}

void adc_auto_scan(uint16_t raw_data_array[])
{
	float switch_time_delay = adc_initial_delay_time();
	
	int count = count_set_bits(MUXDIF_address) + count_set_bits(MUXSG0_default) + count_set_bits(MUXSG1_default); //this is the number of loops to make, by measuring 
	for (int p = 0; p < count; p++){
		uint32_t Channel_data = adc_channel_read_register_format();
		uint8_t Status_byte = adc_return_status_byte(Channel_data);
		int CHID = adc_chid_status(Status_byte);
		uint16_t Raw_data = adc_return_raw_data(Channel_data);
	
		if ((adc_new_status_bit(Status_byte) == true) && (adc_supply_status_bit(Status_byte)== false) && (adc_ovf_status_bit(Status_byte) == false)) {
			//store the data in the array
			if (CHID > 0x19){ //CHID skips address 0x19h according to data sheet
				raw_data_array[CHID-1] = Raw_data; 
			}
			else {
				raw_data_array[CHID] = Raw_data; //store the value
			}
		}
		else {
			//try to report error
			if (adc_supply_status_bit(Status_byte)) {//Serial.println("Error: ADC Status SUPPLY bit set");}
				//Raise MCU error
				String error_text = "Error: ADC Status SUPPLY bit set";
				raise_mcu_error(error_text);
				}
			else if (adc_ovf_status_bit(Status_byte)) {//Serial.println("Error: ADC Status OVF bit set");}
				String error_text = "Error: ADC Status OVF bit set";
				raise_mcu_error(error_text);
				}
		}
	
		delayMicroseconds(switch_time_delay);
	}
}

void adc_setup()
{
	converted_adc_data[27] = ADC_GAIN; //ADC GAIN
	converted_adc_data[28] = ADC_VREF; //Vref = 3.33V
	delay(50); //Settin87g up ADC
	digitalWrite(_pwdn_adc,HIGH); //setting it off
	delay(200);
	digitalWrite(_cs_adc,HIGH); //1. Reset the SPI interface
	delay(150);
	digitalWrite(start_adc,LOW); //2.Stop the converter by setting start pin low
	digitalWrite(_reset_adc,LOW); //3.Reset the converter, Pulse the reset button low
	delay(150);
	digitalWrite(_reset_adc,HIGH);
	delay(500); //delay for ADC startup time
	//4. Configure registers
	for (int i = 0; i < adc_register_array_size; i++) adc_register_write(adc_register_addresses[i],adc_register_defaults[i]); //write values in registers
	//5. Check register values
	for (int j = 0; j < adc_register_array_size; j++)
	{
		if (adc_register_read(adc_register_addresses[j]) != adc_register_defaults[j])
		{ 
			String error_text = "Error in register address : "; //Raise MCU error
			error_text.concat(adc_register_addresses[j]);
			raise_mcu_error(error_text);
		}
	}
	//6. Start the converter
	for (int i = 0; i<5 ; i++)
	{
		adc_auto_scan(raw_adc_data);
		adc_array_convert(raw_adc_data,converted_adc_data);
		delay(10);
	}
}	

void adc_reset(void)
{
	digitalWrite(_reset_adc,LOW);
	delay(150);
	digitalWrite(_reset_adc,HIGH);
	delay(500); //delay for ADC startup time
}

void adc_toggle_start_pin(void)
{
	digitalWrite(start_adc,HIGH); //starts conversion
	delayMicroseconds(2); //wait for data to settle
	digitalWrite(start_adc,LOW); //stops conversion so that we can go to the next Channel ID, Check channel ID in Table 10 of the ADC manual
}

#endif