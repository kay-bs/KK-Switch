/*
  Copyright (c) 2025 Kay Kasper
  under the MIT License (MIT)
*/

#include <Switch.h>

/*
  Simple example with a push button that is connected
  on one side to GND and on the other side to INPUT_PIN.
  (inverse or negative logic)
*/

#define INPUT_PIN 2

Switch sw = Switch(2, false, INPUT_PIN, 0, 20, true);

void setup() {
  Serial.begin(9600);
  // set pinMode
  sw.configurePin();
}

void loop() {
  // identify changes when button is pushed 
  if(sw.hasChanged()){
    // print the current state (1=was pushed, 0=was released)
    Serial.println(sw.getState());
  }
}
