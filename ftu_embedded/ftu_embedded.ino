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
#include "headers/system_fsm.h"

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

RTCZero rtc;

void setup() {
  
  analogWriteResolution(10);
  analogReference(AR_INTERNAL2V23);
  pin_setup();
  delay(1000);
  adc_setup();
  
  Serial.begin(BAUD_RATE); 
  while (!Serial) continue;
  receive_test_instructions();
  Serial.end();

  clock_setup();           //set up 1MHz clock for the timers
  init_tc3();              //set up TC3 whose interrupt sends serial data
  init_tc4();              //initialize TC4 timer, TC4 generates the ADC read interrupt
  init_tc5();              //TC5 is a 1 second counter
  __enable_irq();          //enable interrupts
}

void loop() {
 system_state = system_fsm_transition(system_state);
 system_fsm_run(system_state);
}
