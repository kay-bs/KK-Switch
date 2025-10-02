/*
  Copyright (c) 2025 Kay Kasper
  under the MIT License (MIT)
*/

#include <Switch.h>

/*
  Mapping-Example, that shows how to use mapping values
  and different variants of instantiation.

  Prerequisite is a push button that is connected
  on one side to GND and on the other side to INPUT_PIN.
  (inverse or negative logic)
*/

#define INPUT_PIN 2

//Switch sw = Switch(2, true, INPUT_PIN, 10, 20, true);
// The above line of code is easier to use than the next two lines
// but the use of an explicit buffer definition is for microcontrollers
// more deterministic, predictable and causes no potential memory problems.
// Try it as alternative and switch between both variants.
byte mapBuffer[2];
Switch sw = Switch(2, &mapBuffer[0], INPUT_PIN, 10, 20, true);


void setup() {
  Serial.begin(9600);
  // set pinMode
  sw.configurePin();
  // define mappings
  sw.setMapping(SW_STATE_DEFAULT_ON, 'P');  // for was pushed
  sw.setMapping(SW_STATE_DEFAULT_OFF, 'R'); // for was released
}

void loop() {
  // identify changes when button is pushed 
  if(sw.hasChanged()){
    // print the current state 1=was pushed, 0=was released and the mapping value
    Serial.print(sw.getState());
    Serial.print(" -> ");
    Serial.print((char)sw.getMappedState());
    // print the previous state 1=was pushed, 0=was released and the mapping value
    Serial.print(" (prev: ");
    if(sw.getPrevState() == SW_STATE_UNDEFINED){
      Serial.print("SW_STATE_UNDEFINED");
      Serial.print(" -> ");
      Serial.print("SW_STATE_UNDEFINED");
    }
    else{
      Serial.print(sw.getPrevState());
      Serial.print(" -> ");
      Serial.print((char)sw.getPrevMappedState());
    }
    Serial.println(")");
  }
}
