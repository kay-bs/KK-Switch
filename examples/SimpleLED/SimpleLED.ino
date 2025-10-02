/*
  Copyright (c) 2025 Kay Kasper
  under the MIT License (MIT)
*/

#include <Switch.h>

/*
  Simple example, where button push is directly
  contolling the buildin LED (button pressed = LED on).

  Prerequisite is a push button that is connected
  on one side to GND and on the other side to INPUT_PIN.
  (inverse or negative logic). LED is assumed to be
  always available at pin LED_BUILTIN.
*/

#define INPUT_PIN 2

Switch sw = Switch(2, false, INPUT_PIN, 0, 20, true);

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // initialize switch input pin
  sw.configurePin();
}

// switch the LED on (1) or off (0)
void setLED(byte ledOn) {
  digitalWrite(LED_BUILTIN, ledOn);
}

void loop() {
  // identify changes when button is pushed 
  if(sw.hasChanged()){
    // switch the LED on, during button is pushed
    setLED(sw.getState());
  }
}
