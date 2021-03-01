#include "SPI.h"
#include "ArduinoJson.h"
#include "ArduinoJson.hpp"
#include "Arduino.h"
#include "samd21/include/samd21g18a.h"
#include "math.h"
#include "strings.h"
#include "WString.h"
#include "time.h"
#include "headers/ftu_library.h"
#include "headers/pin_descriptions.h"
#include "headers/adc.h"
#include "headers/system_fsm.h"

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
  __enable_irq();          //enable interrupts
  idle = false;
}

void loop() {
 system_state = system_fsm_transition(system_state);
 system_fsm_run(system_state);
}
