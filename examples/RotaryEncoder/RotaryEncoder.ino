
/*
  Copyright (c) 2025 Kay Kasper
  under the MIT License (MIT)
*/

#include <RotaryEncoder.h>

/*
  Example with a subclass of Switch for rotary encoders.

  The turning of the rotary encoder will change the speed
  (+/-) of blinking of the build in LED.

  Prerequisite is an rotary encoder connected with the pins
  A and B to two digital input pins, that pull the level to
  GND (negative logic) if connected.
  LED is assumed to be always available at pin LED_BUILTIN.

  Hardware debouncing is recommended due to fast raw input
  state changes. Software debouncing is to slow or will be not
  precise enough. But SW debouncing is used here to simplify
  the electrical design.
*/

#define INPUT_PIN_A 2                 // digital input pin A (first contact for right turn)
#define INPUT_PIN_B 3                 // digital input pin B (first contact for left turn)
#define MAX_BLINK_CYCLE_MILLIS 1000   // min allowed is 1000
#define STEPS 10                      // number of rotation steps used for calculation


// input state sequence analyzer for rotary encoders
RotaryEncoderAnalyzer rea = RotaryEncoderAnalyzer();
// new AnalogSwitch object for handling the analog input and generate states changes
RotaryEncoder re = RotaryEncoder(&rea, false, INPUT_PIN_A, INPUT_PIN_B, 5, true);
// Status on or off independently of blinking
boolean lightOn = false;
// time, when the LED was toggeled
unsigned long lastLEDChange = 0;
// duration of LED on/off phases
unsigned long cycleDurationMillis = MAX_BLINK_CYCLE_MILLIS;
// counter for division, default is medium value
byte counter = STEPS / 2;

void setup() {
  // initialize digital pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);
  // initialize switch input pins
  re.configurePins();
}

// switch the LED on (1) or off (0)
void setLED(byte ledOn) {
  digitalWrite(LED_BUILTIN, ledOn);
}

void loop() {
  // toggle the LED for blinking
  if(millis() - lastLEDChange > cycleDurationMillis){
    if(lightOn){
      setLED(0);
    }
    else{
      setLED(1);
    }
    lightOn = !lightOn;
    lastLEDChange = millis();
  }

  // react on changing states by calculating the cycle duration new
  if(re.hasChanged()){
    switch(re.getState()){
      case REA_STATE_E_DIRECTION_R:
        counter = (counter < STEPS -1 ? ++counter : counter);
        break;
      case REA_STATE_E_DIRECTION_L:
        counter = (counter > 0 ? --counter : counter);
        break;
      case REA_STATE_E_OFF:
        // ignore
        break;
    }
    cycleDurationMillis = MAX_BLINK_CYCLE_MILLIS / (STEPS - counter);
  }
}
