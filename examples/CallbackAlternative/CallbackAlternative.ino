
/*
  Copyright (c) 2025 Kay Kasper
  under the MIT License (MIT)
*/

#include <SwitchExtensions.h>

/*
  Example, where button push state change sequence is
  contolling the build in LED:
  Single push -> switch on/off permanent light (toggle)
  Long push ->   blinking while button is pushed
                 after releasing long push, LED
                 returns to state on or off as was before
                 long push.

  It is shown, how easy the implementation of function calls
  in case of changes with Switch library is. This is as easy
  as call back implementations with other libraries.

  Prerequisite is a push button that is connected
  on one side to GND and on the other side to INPUT_PIN.
  (inverse or negative logic). LED is assumed to be
  always available at pin LED_BUILTIN.
*/

#define INPUT_PIN 2
#define BLINK_CYCLE_MILLIS 100

// analyzes the state sequence and generates states for
// not pushed, single push and long push (>= 500 ms)
// long push is delivered after the definied time, not at button release
PushButtonRepeatAnalyzer bra = PushButtonRepeatAnalyzer(500, BLINK_CYCLE_MILLIS);
Switch sw = Switch(&bra, false, INPUT_PIN, 20, true);

// Status on or off independently of blinking
boolean lightOn = false;

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

// funtion to be called, in case of delivered state PBR_STATE_E_SINGLE
void callBackOnSinglePush(){
  // toggle on/off
  if(!lightOn){
    setLED(1);
    lightOn = true;
  }
  else{
    setLED(0);
    lightOn = false;
  }
}

// funtion to be called, in case of delivered state PBR_STATE_E_CONT_A
void callBackOnContinousAPush(){
  // set on
  setLED(1);
}

// funtion to be called, in case of delivered state PBR_STATE_E_CONT_B
void callBackOnContinousBPush(){
  // set off
  setLED(0);
}

// funtion to be called, in case of delivered state PBR_STATE_E_OFF
void callBackOnPushOff(){
  if(lightOn){
    setLED(1);
  }
  else{
    setLED(0);
  }
}

void loop() {
  // react on changing states, simulating callback functions
  if(sw.hasChanged()){
    switch(sw.getState()){
      case PBR_STATE_E_SINGLE:  callBackOnSinglePush(); break;
      case PBR_STATE_E_CONT_A:  callBackOnContinousAPush(); break;
      case PBR_STATE_E_CONT_B:  callBackOnContinousBPush(); break;
      case PBR_STATE_E_OFF:     callBackOnPushOff(); break;
    }
  }
}
