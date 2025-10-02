
/*
  Copyright (c) 2025 Kay Kasper
  under the MIT License (MIT)
*/

#include <SwitchExtensions.h>

/*
  Example, where button push state change sequence is
  contolling the build in LED:
  Single push -> switch on permanent light
  Double push -> switch off permanent light
  Long push -> switch on/off blinking light (toggeling)

  Prerequisite is a push button that is connected
  on one side to GND and on the other side to INPUT_PIN.
  (inverse or negative logic). LED is assumed to be
  always available at pin LED_BUILTIN.
*/

#define INPUT_PIN 2
#define BLINK_CYCLE_MILLIS 250

// analyzes the state sequence and generates states for
// not pushed, single push, double push (within 400 ms), long push (> 500 ms)
// long push is given as resulting state after the definied time, not when
// button is released
PushButtonDoubleLongAnalyzer bpa = PushButtonDoubleLongAnalyzer(500,800,true);
Switch sw = Switch(&bpa, false, INPUT_PIN, 20, true);

// Status on or off independently of blinking
byte stateLED = 0; // value 0 -> LED off, 1 -> LED on
// Blinking information, blinking has higher priority than normal on/off 
byte blinking = 0; // 0 -> no blinking, 1 -> LED blinking on, 2 -> LED blinking off
// start time for each blink phase
unsigned long lastBlinkToggleMillis = 0;

void setup() {
  // initialize digital pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);
  // initialize switch input pin
  sw.configurePin();
}

// switch the LED on (1) or off (0)
void setLED(byte ledOn) {
  digitalWrite(LED_BUILTIN, ledOn);
}

void loop() {
  // handle blink changes based on duration
  if(blinking > 0 && millis() - lastBlinkToggleMillis > BLINK_CYCLE_MILLIS){
    lastBlinkToggleMillis = millis();
    if(blinking == 1){
      setLED(0);
      blinking = 2;
    }
    else{
      setLED(1);
      blinking = 1;
    }
  }

  // identify changes when button is pushed 
  if(sw.hasChanged()){
    switch(sw.getState()){
      case PBDL_STATE_E_SINGLE:
        setLED(1);
        stateLED = 1;
        break;
      case PBDL_STATE_E_DOUBLE:
        setLED(0);
        stateLED = 0;
        break;
      case PBDL_STATE_E_LONG:
        // start or stop blinking
        if(blinking > 0){
          // stop blinking
          blinking = 0;
          lastBlinkToggleMillis = 0;
          if(stateLED > 0){
            setLED(1);
          }
          else{
            setLED(0);
          }
        }
        else{
          // start blinking
          blinking = 1;
          lastBlinkToggleMillis = millis();
          setLED(1);
        }
        break;
      case PBDL_STATE_E_OFF:
        break;
    }
  }
}
