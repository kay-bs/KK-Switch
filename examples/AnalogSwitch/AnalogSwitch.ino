
/*
  Copyright (c) 2025 Kay Kasper
  under the MIT License (MIT)
*/

#include <Switch.h>

/*
  Example with a new subclass of Switch to generate raw input
  states based on an analog input pin.

  The variable value of the analog pin will change the speed
  of blinking of the build in LED. 

  Prerequisite is an potentiometer/attenuator connected
  with the middle pin (variable voltage) to an
  analog input pin. LED is assumed to be
  always available at pin LED_BUILTIN.
*/

#define INPUT_PIN A7                  // must be analog pin A0 to A7
#define NUM_STATES 10                 // max allowed is 64
#define MAX_BLINK_CYCLE_MILLIS 1000   // min allowed is 1000


// new class for handling analog input as source for raw input states
// only an example with easy logic that cannot garantee state stability
// due to lack of logic to stabalize measuring and potentiometer instability.
class AnalogSwitch : public Switch {
  public:
    /*
      Configure input pin always with positive logic 
    */
    void configurePin(){
      // do nothing
    };

    /*
      Constructor for new AnalogSwitch object with an identical number of raw input and output states.
      Debouncing is disabled.
      Parameters see Switch.

      @seealso Switch
    */
    AnalogSwitch(byte numState, boolean enableMapping, byte analogInputPin, byte readCycleMillis, boolean invertRaw)
    : Switch(numState, enableMapping, analogInputPin, readCycleMillis, 0, invertRaw){}

  protected:
    /*
      Calculation is based on theoretical potentiometer/attenuator
      which is changing values linear. Not realistic.

      @returns raw states from analog pin in the range 0 to numStates-1
    */
    byte getRawState(){
      return analogRead(_inputPin) * _numState / 1024;
    };
};

// new AnalogSwitch object for handling the analog input and generate states changes
AnalogSwitch asw = AnalogSwitch(NUM_STATES, false, INPUT_PIN, 50, true);
// Status on or off independently of blinking
boolean lightOn = false;
// time, when the LED was toggeled
unsigned long lastLEDChange = 0;
// duration of LED on/off phases
unsigned long cycleDurationMillis = MAX_BLINK_CYCLE_MILLIS;

void setup() {
  // initialize digital pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);
  // initialize switch input pin (in AnalogSwitch not nesessary, only because standard)
  asw.configurePin();
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
  if(asw.hasChanged()){
    cycleDurationMillis = MAX_BLINK_CYCLE_MILLIS / (NUM_STATES - asw.getState());
  }
}
