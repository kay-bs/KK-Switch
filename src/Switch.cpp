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

#include "Switch.h"

/*
  Default implementation of the SwitchStateAnalyzer class

  The behaviour is as if the analyzer is not existing under the
  prerequisite, that a standard switch with 2 states is used.
  All raw states are passed through and no state sequence is analyzed.
*/
SwitchStateAnalyzer::SwitchStateAnalyzer() {
};


byte SwitchStateAnalyzer::getReadCycleMillis() {
  return 0;
};


void SwitchStateAnalyzer::reset() {
};


byte SwitchStateAnalyzer::getNumAnalyzerStates() {
  return 2;
};


byte SwitchStateAnalyzer::getNumSwitchStates() {
  return 2;
};


byte SwitchStateAnalyzer::getAnalyzerState(byte switchState){
  return switchState;
};

/*
  Default implementation of the Switch class based on 2 raw states 
  (SW_STATE_DEFAULT_OFF, SW_STATE_DEFAULT_ON) of an digital input pin.
*/

Switch::Switch(byte numState, boolean enableMapping, byte inputPin,
                byte readCycleMillis, byte debounceMillis, boolean invertRaw){
  _SSA = 0;
  init(numState, enableMapping, (byte*)0, inputPin,
        readCycleMillis, debounceMillis, invertRaw);
}


Switch::Switch(byte numState, byte* bufferMapping, byte inputPin,
                byte readCycleMillis, byte debounceMillis, boolean invertRaw){
  _SSA = 0;
  init(numState, (bufferMapping != 0), bufferMapping, inputPin,
        readCycleMillis, debounceMillis, invertRaw);
}


Switch::Switch(SwitchStateAnalyzer* ssa, boolean enableMapping, byte inputPin,
                byte debounceMillis, boolean invertRaw){
  _SSA = ssa;
  if(ssa != 0){
    _SSA->reset();
    init(_SSA->getNumAnalyzerStates(), enableMapping, (byte*)0, inputPin,
        _SSA->getReadCycleMillis(), debounceMillis, invertRaw);
  }
  else{
    init(2, enableMapping, (byte*)0, inputPin, 0, debounceMillis, invertRaw);

  }
}


Switch::Switch(SwitchStateAnalyzer* ssa, byte* bufferMapping, byte inputPin, 
                byte debounceMillis, boolean invertRaw){
  _SSA = ssa;
  if(ssa != 0){
    _SSA->reset();
    init(_SSA->getNumAnalyzerStates(), (bufferMapping != 0), bufferMapping, inputPin,
        _SSA->getReadCycleMillis(), debounceMillis, invertRaw);
  }
  else{
    init(2, (bufferMapping != 0), bufferMapping, inputPin, 0, debounceMillis, invertRaw);
  }
}


void Switch::init(byte numState, boolean enableMapping, byte* bufferMapping,
                byte inputPin, byte readCycleMillis, byte debounceMillis, boolean invertRaw){

  _currentState = SW_STATE_UNDEFINED;
  _previousState = SW_STATE_UNDEFINED;
  _lastRawState = SW_STATE_UNDEFINED;
  _inputPin = inputPin;
  _readCycleMillis = readCycleMillis;
  _debounceMillis = debounceMillis;
  _lastReadMillis = 0;
  _mapValues = bufferMapping;
  _debouncing = false;
  _invertRaw = invertRaw;

  if(numState < 2){
    _numState = 2;
  }
  else if(numState > 64){
    _numState = 64;
  }
  else{
    _numState = numState;
  }

  if(enableMapping){
    if(_mapValues == 0){
      _mapValues = (byte*)malloc(_numState);
    }
    for(byte i=0 ; i<_numState ; i++){
      _mapValues[i] = i;
    }
  }
}


void Switch::configurePin(){
  if(_invertRaw){
    pinMode(_inputPin, INPUT_PULLUP);
  }
  else{
    pinMode(_inputPin, INPUT);
  }
}


byte Switch::getRawState(){
  return digitalRead(_inputPin);
}


byte Switch::getState(){
  return _currentState;
}


byte Switch::getMappedState(){
  if(_mapValues == 0 || _currentState == SW_STATE_UNDEFINED){
    return _currentState;
  }
  return _mapValues[_currentState];
}


void Switch::setMapping(byte state, byte mappingValue){
  if(_mapValues == 0 || state >= _numState){
    return;
  }
  _mapValues[state] = mappingValue;
}


boolean Switch::hasChanged(){
  byte state;
  byte rawState;
  unsigned long timeDiff;
  unsigned long current = millis();

  timeDiff = current - _lastReadMillis;

  // take care of wait times (debouncing and read cycle)
  if(_debouncing){
    if(timeDiff < _debounceMillis){
      return false;
    }
  }
  else{
    if(_readCycleMillis > 0 && timeDiff < _readCycleMillis){
      return false;
    }
  }

  // raw read and if necessary inverting of the value
  rawState = getRawState();
  _lastReadMillis = current;

  if(_invertRaw){
    rawState = (_SSA != 0 ? _SSA->getNumSwitchStates() : _numState) - 1 - rawState;
  }

  // start optionally debouncing phase
  if(!_debouncing && _debounceMillis > 0 && _lastRawState != rawState){
    _debouncing = true;
    return false;
  }

  _debouncing = false;
  _lastRawState = rawState;

  // optionally sequence analysis
  state = (_SSA != 0 ? _SSA->getAnalyzerState(rawState) : rawState);

  if(state != _currentState){
    _previousState = _currentState;
    _currentState = state;
    return true;
  }
  return false;
}


byte Switch::getPrevState(){
  return _previousState;
}


byte Switch::getPrevMappedState(){
  if(_mapValues == 0 || _previousState == SW_STATE_UNDEFINED){
    return _previousState;
  }
  return _mapValues[_previousState];
}


void Switch::reset(){
  _currentState = SW_STATE_UNDEFINED;
  _previousState = SW_STATE_UNDEFINED;
  _lastRawState = SW_STATE_UNDEFINED;
  _lastReadMillis = 0;
  _debouncing = false;

  if(_SSA != 0){
    _SSA->reset();
  }
}
