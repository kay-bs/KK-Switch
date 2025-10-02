/*
  The MIT License (MIT)

  Copyright (c) 2025 Kay Kasper

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the “Software”),
  to deal in the Software without restriction,including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  The Software is provided “as is”, without warranty of any kind, express
  or implied, including but not limited to the warranties of merchantability,
  fitness for a particular purpose and noninfringement. In no event shall
  the authors or copyright holders be liable for any claim, damages or other
  liability, whether in an action of contract, tort or otherwise, arising
  from, out of or in connection with the software or the use or other dealings
  in the Software.
*/

#ifndef SW_ROTARYENCODER_H
#define SW_ROTARYENCODER_H

#include "Switch.h"

// Constants for the RotaryEncoderAnalyzer implementation based on SwitchStateAnalyzer
#define REA_STATE_E_OFF           0x00   // external relevant output state, no movement identified
#define REA_STATE_E_DIRECTION_R   0x01   // external relevant output state, right turn finished
#define REA_STATE_E_DIRECTION_L   0x02   // external relevant output state, left turn finished
#define REA_STATE_I_OFF           0x03   // internal state, raw state off
#define REA_STATE_I_RIGHT_A       0x04   // internal state, raw state A high, right turn started
#define REA_STATE_I_LEFT_B        0x05   // internal state, raw state B high, left turn started
#define REA_STATE_I_UNDEFINED     0xFF   // internal state, raw state undefined
#define REA_SWSTATE_OFF           0x00   // raw input state off, A and B low
#define REA_SWSTATE_A             0x01   // raw input state only A high
#define REA_SWSTATE_B             0x02   // raw input state only B high
#define REA_SWSTATE_AB            0x03   // raw input state A and B high


/*
  Implementation of a rotary encoder specific state analyzer.
  
  Analysis of rotary encoder input signals from 2 digital pins (4 raw states)
  to deliver the information (3 states) which movements were done:
  
  no movement (REA_STATE_E_OFF)
  right turn one step (REA_STATE_E_DIRECTION_R)
  left turn on step (REA_STATE_E_DIRECTION_L)
  
  Prerequisite are the 4 raw input states
  REA_SWSTATE_OFF, REA_SWSTATE_A, REA_SWSTATE_B, REA_SWSTATE_AB
  
  The return of an REA_STATE_E_DIRECTION_R is based on the input sequence
  OFF -> A -> optionally AB -> optionally B -> OFF
  
  The return of an REA_STATE_E_DIRECTION_L is based on the input sequence
  OFF -> B -> optionally AB -> optionally A -> OFF
  
  @seealso Switch.h
*/
class RotaryEncoderAnalyzer : public SwitchStateAnalyzer {
  private:
    byte _internalState;

    void init(){
      reset();
    }

  public:
    RotaryEncoderAnalyzer() : SwitchStateAnalyzer() {
      RotaryEncoderAnalyzer::init();
    }

    byte getReadCycleMillis() {
      return 2;
    };

    void reset(){
      _internalState = REA_STATE_I_UNDEFINED;
    }

    byte getNumAnalyzerStates(){
      return 3;
    }

    byte getNumSwitchStates(){
      return 4;
    }

    /*
      Analysis of the state sequence.

      @param switchState  4 possible raw states
                          REA_SWSTATE_OFF, REA_SWSTATE_A, REA_SWSTATE_B, REA_SWSTATE_AB
      @returns            1 of the 3 states
                          REA_STATE_E_OFF, REA_STATE_E_DIRECTION_R, REA_STATE_E_DIRECTION_L
    */
    byte getAnalyzerState(byte switchState){

      if(_internalState == REA_STATE_I_UNDEFINED){
        _internalState = REA_STATE_I_OFF;
      }

      // find next internal state based on current internal state and raw state
      if(_internalState == REA_STATE_I_OFF){
        if(switchState == REA_SWSTATE_A){
          // start of a new sequence
          _internalState = REA_STATE_I_RIGHT_A;
        }
        else if(switchState == REA_SWSTATE_B){
          // start of a new sequence
          _internalState = REA_STATE_I_LEFT_B;
        }
        // all other raw states changes can be ignored
      }
      else if(_internalState == REA_STATE_I_RIGHT_A){
        if(switchState == REA_SWSTATE_OFF){
          _internalState = REA_STATE_I_OFF;
          return REA_STATE_E_DIRECTION_R;
        }
        // all other raw states changes can be ignored
      }
      else if(_internalState == REA_STATE_I_LEFT_B){
        if(switchState == REA_SWSTATE_OFF){
          _internalState = REA_STATE_I_OFF;
          return REA_STATE_E_DIRECTION_L;
        }
        // all other raw states changes can be ignored
      }

      // default return value, result still unclear
      return REA_STATE_E_OFF;
    }
};

/*
  Implementation of a specific switch for rotary encoders.

  Analysis of rotary encoder input signals from 2 digital pins (4 raw states)
  to deliver the information (3 states) which movements were done:

  no movement (REA_STATE_E_OFF)
  right turn one step (REA_STATE_E_DIRECTION_R)
  left turn on step (REA_STATE_E_DIRECTION_L)

  Prerequisite are the 4 raw input states
  REA_SWSTATE_OFF, REA_SWSTATE_A, REA_SWSTATE_B and REA_SWSTATE_AB

  @seealso Switch.h
*/
class RotaryEncoder : public Switch {
  private:
    // keep value of parameter inputPinB
    byte _inputPinB;

  public:
    RotaryEncoder(RotaryEncoderAnalyzer* rea, boolean enableMapping, byte inputPinA,
      byte inputPinB, byte debounceMillis, boolean invertRaw)
      : Switch(rea, enableMapping, inputPinA, debounceMillis, invertRaw) {

      _inputPinB = inputPinB;

    }

    RotaryEncoder(RotaryEncoderAnalyzer* rea, byte* bufferMapping, byte inputPinA,
      byte inputPinB, byte debounceMillis, boolean invertRaw)
      : Switch(rea, bufferMapping, inputPinA, debounceMillis, invertRaw) {

      _inputPinB = inputPinB;

    }

    /*
      Configure both input pins, 
      Not only one pin like default implementation in Switch class
    */
    void configurePins(){
      Switch::configurePin(); // for inputPinA

      if(_invertRaw){
        pinMode(_inputPinB, INPUT_PULLUP);
      }
      else{
        pinMode(_inputPinB, INPUT);
      }
    };

  protected:

    /*
      @returns 4 raw states for input pins A and B, values 0 to 3
    */
    byte getRawState(){
      return Switch::getRawState() + (digitalRead(_inputPinB) == LOW ? 0 : 2);
    }

};

#endif
