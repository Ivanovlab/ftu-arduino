/************************************************************************/
/*                    FILE DESCRIPTION                                  */
/************************************************************************/

/*H**********************************************************************
* \@filename :        
* FTU_system_code.ino            
*
* \@description :
* This file controls a SAMD21G18A MCU on Arduino MKR1000
* Used to run a HTOL Test (High-temperature operating life) for Accelerated Life Testing
*
* \@notes : 
* This code connects to a python script
*
*
* \@authors :    
* 1. Valentine Ssebuyungo        
* 2. Rohit Singh
*
* \@start_date : 1st June 2020
*
* \@change_log : 
* tc3 timer removed
*
* \@documentation_style :	 
* 1. "https://developer.lsst.io/cpp/api-docs.html#cpp-doxygen-see-also"
* 2. "http://www.edparrish.net/common/cppdoc.html#functioncomment"
*
* \@future_improvements :
*Add a header file to contain all data
*IoT communication
*RTC timer for time
*PID control for magnetic and heater system
*Emergency stop
*WDT
*Faster Serial communication - 0.1uS
*Reset Arduino after test stops
*Frequency divider functionality
*
*H*/
#include "SPI.h"
#include "ArduinoJson.h"
#include "ArduinoJson.hpp"
#include "Arduino.h"
#include "samd21/include/samd21g18a.h"
#include "math.h"
#include "strings.h"
#include "WString.h"
#include "time.h"

#include <RTCZero.h>

#include "headers/ftu_library.h"
#include "headers/pin_descriptions.h"
#include "headers/adc.h"
#include "headers/dac.h"
#include "headers/rtc_clock.h"
#include "headers/magnetic_field_fsm.h"
#include "headers/heater_fsm.h"
#include "headers/system_fsm.h"

// Create an RTC object
RTCZero rtc;

/*How values are stored in these arrays
The Bits CHID is the Array[index] up to index 24,
Index 25 is CHID 26 -> OFFSET
Index 26 is CHID 27 -> Temperature
Index 27 is CHID 28 -> GAIN
Index 28 is CHID 29 -> External reference


From Table 10 in data sheet
BITS CHID[4:0]          PRIORITY        CHANNEL         DESCRIPTION
00h                     1 (highest)     DIFF0 (AIN0–AIN1) Differential 0
01h                     2               DIFF1 (AIN2–AIN3) Differential 1
02h						3				DIFF2 (AIN4–AIN5) Differential 2
03h				        4               DIFF3 (AIN6–AIN7) Differential 3
04h						5			    DIFF4 (AIN8– AIN9) Differential 4
05h						6				DIFF5 (AIN10–AIN11) Differential 5
06h						7				DIFF6 (AIN12–AIN13) Differential 6
07h						8				DIFF7 (AIN14–AIN15) Differential 7
08h						9				AIN0 Single-ended 0
09h						10				AIN1 Single-ended 1
0Ah						11				AIN2 Single-ended 2
0Bh						12				AIN3 Single-ended 3
0Ch						13				AIN4 Single-ended 4
0Dh						14				AIN5 Single-ended 5
0Eh						15				AIN6 Single-ended 6
0Fh						16				AIN7 Single-ended 7
10h						17				AIN8 Single-ended 8
11h						18				AIN9 Single-ended 9
12h						19				AIN10 Single-ended 10
13h						20				AIN11 Single-ended 11
14h						21				AIN12 Single-ended 12
15h						22				AIN13 Single-ended 13
16h						23				AIN14 Single-ended 14
17h						24			    AIN15 Single-ended 15
18h						25				OFFSET Offset
1Ah						26				VCC AVDD – AVSS supplies
1Bh						27				TEMP Temperature
1Ch						28				GAIN Gain
1Dh						29 (lowest)		REF External reference

/**
 * This setup function is run before a test starts and sets up the system
 * 
 * @param void
 * @return void
 */
void setup() {
  
  analogWriteResolution(10);
  analogReference(AR_INTERNAL2V23); //sets 3.3V reference
  pin_setup(); //Setup MCU pins
  delay(1000);
  adc_setup(); //function sets up the ADC
  
  Serial.begin(BAUD_RATE); 
  
  while (!Serial) continue;//if not connected stall test until connected
  receive_test_instructions(); //run function for handshaking to receive instructions
  Serial.end();

  clock_setup();           //set up 1MHz clock for the timers
  init_tc3();            //set up TC3 whose interrupt sends serial data
  init_tc4();              //initialize TC4 timer, TC4 generates the ADC read interrupt
  init_tc5();              //TC5 is a 1 second counter
  __enable_irq();          //enable interrupts
  test_time_count = 0; //reset test time
  
}

/**
 * This function runs for the whole test
 * 
 * @param void
 * @return void
 */
void loop() {
 system_state = system_fsm_transition(system_state,test_start,test_stop);
 system_fsm_run(system_state);
}


