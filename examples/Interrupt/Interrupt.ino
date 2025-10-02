/*
  Copyright (c) 2025 Kay Kasper
  under the MIT License (MIT)
*/

#include <Switch.h>

/*
  Example for using an interrupt as trigger for changes.
  A button push is directly contolling the build in LED
  (button pressed = LED on).

  Prerequisites:
  A push button that is connected on one side to GND and
  on the other side to INPUT_PIN (inverse or negative logic).
  LED is assumed to be always available at pin LED_BUILTIN.
  Hardware debouncing instead of software debouncing is strongly
  recommended for the input signale and interrupt. Software
  debouncing and readCycleMillis waits will fail because of
  missing continously calls in the loop.
*/

#define INPUT_PIN 2
#define INTERRUPT_PIN INPUT_PIN

// no SW debouncing and no wait cycles usefull
Switch sw = Switch(2, false, INPUT_PIN, 0, 0, true);

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  // initialize interrupt handling
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), changed, CHANGE);
  // initialize switch input pin
  sw.configurePin();
}

// switch the LED on (1) or off (0)
void setLED(byte ledOn) {
  digitalWrite(LED_BUILTIN, ledOn);
}

// interrupt routine, called when raw input state changes
void changed(){
  // identify changes when button is pushed 
  if(sw.hasChanged()){
    // switch the LED on, during button is pushed
    setLED(sw.getState());
  }
}

void loop() {
  // important things to do
}
