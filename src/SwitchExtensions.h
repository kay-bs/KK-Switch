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

#ifndef SW_EXTENSIONS_H
#define SW_EXTENSIONS_H

#include "Switch.h"

// Constants for the PushButtonRepeatAnalyzer implementation based on SwitchStateAnalyzer
#define PBR_STATE_E_OFF           0x00   // external relevant output state, not pushed
#define PBR_STATE_E_SINGLE        0x01   // external relevant output state, one single push and release
#define PBR_STATE_E_CONT_A        0x02   // external relevant output state, continous push, phase A finished
#define PBR_STATE_E_CONT_B        0x03   // external relevant output state, continous push, phase B finished
#define PBR_STATE_I_OFF           0x04   // internal state, not pushed
#define PBR_STATE_I_ON            0x05   // internal state, button currently pushed, potential sequence start
#define PBR_STATE_I_CONT_A        0x06   // internal state, continous push, in A phase
#define PBR_STATE_I_CONT_B        0x07   // internal state, continous push, in B phase
#define PBR_STATE_I_UNDEFINED     0xFF   // internal state, raw state undefined

/*
  Implementation of an analyzer, that analyzes the push sequences "single push"
  and "continous push" of a button. Continous push delivers an output of two
  toggeling states, as long as the button is pushed.
  Supports up to 65 seconds push duration.

  Analysis of button input signals from 1 digital pin (2 raw states)
  to deliver the information (4 states) which complete sequences were identified:

  no push (PBR_STATE_E_OFF)
  single push completed (PBR_STATE_E_SINGLE)
  continous push phase A completed (PBR_STATE_E_CONT_A)
  continous push phase B completed (PBR_STATE_E_CONT_B)

  Prerequisite are the 2 raw input states
  SW_STATE_DEFAULT_ON and SW_STATE_DEFAULT_OFF

  The return of an PBR_STATE_E_SINGLE is based on the input sequence
  OFF -> ON (< longStartMillis) -> OFF
  
  The return of an PBR_STATE_E_CONT_A or PBR_STATE_E_CONT_B is based on the input sequence
  OFF -> ON (>= longStartMillis) -> OFF

  @seealso Switch.h
*/
class PushButtonRepeatAnalyzer : public SwitchStateAnalyzer {
  private:
    // internal storage for parameter longStartMillis, value 0 means only single push
    unsigned int _longStartMillis;
    // internal storage for parameter repeatMillis, value 0 means no repeats
    unsigned int _repeatMillis;
    // last start of sequence beginning with SW_STATE_DEFAULT_ON
    unsigned long _procStart;
    // internal state within the sequence
    byte _internalState;

    /*
      Internal initialization with handover of the constructors parameters
    */
    void init(unsigned int longStartMillis, unsigned int repeatMillis){
      _longStartMillis = (longStartMillis > 2000 ? 2000 : longStartMillis);
      _repeatMillis = (repeatMillis > 2000 ? 2000 : repeatMillis);
      reset();
    }

  public:
    /*
      Default instantiation of this analyzer.
      Only single pushes of the button will be identified.
    */
    PushButtonRepeatAnalyzer() : SwitchStateAnalyzer() {
      PushButtonRepeatAnalyzer::init(0, 0);
    }

    /*
      Instantiation of this analyzer for analyzing single and continous pushes
      Supports up to 65 seconds push duration.
      
      @param longStartMillis  Time in milliseconds that is used to differentiate
                              between single and continous push.
                              Push durations < longStartMillis mean singe push, otherwise continous push
                              Value = 0 means that no continous pushes are recognized
                              Range from 0 to 2000 allowed.
      @param repeatMillis     Duration in milliseconds of each phase A and B during continous pushes
                              Value = 0 means that no continous pushes are recognized
                              Range from 0 to 2000 allowed.
    */
    PushButtonRepeatAnalyzer(unsigned int longStartMillis, unsigned int repeatMills) : SwitchStateAnalyzer () {
      PushButtonRepeatAnalyzer::init(longStartMillis, repeatMills);
    }

    byte getReadCycleMillis() {
      return max(_repeatMillis / 20, 1);
    };

    void reset(){
      _internalState = PBR_STATE_I_UNDEFINED;
      _procStart = 0;
    }

    byte getNumAnalyzerStates(){
      return 4;
    }

    byte getNumSwitchStates(){
      return 2;
    }

    /*
      @param switchState  Input must be one of the 2 raw states
                          SW_STATE_DEFAULT_ON and SW_STATE_DEFAULT_OFF
      @returns            One of the 4 output states PBR_STATE_E_OFF,
                          PBR_STATE_E_SINGLE, PBR_STATE_E_CONT_A, PBR_STATE_E_CONT_B
    */
    byte getAnalyzerState(byte switchState){
      unsigned int procDuration = 0;    // time since last start of a sequence 
      unsigned int repeatDuration = 0;  // time since last start of continous phase A
      unsigned long current = millis();
      boolean stateOn = (switchState == SW_STATE_DEFAULT_ON);

      if(_internalState == PBR_STATE_I_UNDEFINED){
        if(stateOn){
          _procStart = current;
          _internalState = PBR_STATE_I_ON;
        }
        else{
          _internalState = PBR_STATE_I_OFF;
        }
      }

      // calculate times, up to 65 seconds push duration safely supported
        procDuration = (unsigned int)(current - _procStart);

      if(_repeatMillis > 0 && procDuration > _longStartMillis){
          // calculate time difference from last start of phase A till now
        repeatDuration = (procDuration - _longStartMillis) % (2 * _repeatMillis);
      }


      // find next internal state based on current internal state and raw state
      if(_internalState == PBR_STATE_I_OFF){
        if(!stateOn){
          // all other raw states can be ignored
        }
        else{
          // start of a new sequence
          _procStart = current;
          _internalState = PBR_STATE_I_ON;
        }
      }
      else if(_internalState == PBR_STATE_I_ON){
        if(stateOn){
          if(_longStartMillis > 0 && procDuration >= _longStartMillis){
            _internalState = PBR_STATE_I_CONT_A;
            return PBR_STATE_E_CONT_A;
          }
        }
        else{
          _internalState = PBR_STATE_I_OFF;
          _procStart = 0;
          return PBR_STATE_E_SINGLE;
        }
      }
      else if(_internalState == PBR_STATE_I_CONT_A){
        if(stateOn){
          if(repeatDuration >= _repeatMillis){
            _internalState = PBR_STATE_I_CONT_B;
            return PBR_STATE_E_CONT_B;
          }
          else{
            return PBR_STATE_E_CONT_A;
          }
        }
        else{
            _internalState = PBR_STATE_I_OFF;
            _procStart = 0;
        }
      }
      else if(_internalState == PBR_STATE_I_CONT_B){
        if(stateOn){
          if(repeatDuration < _repeatMillis){
            _internalState = PBR_STATE_I_CONT_A;
            return PBR_STATE_E_CONT_A;
          }
          else{
            return PBR_STATE_E_CONT_B;
          }
        }
        else{
            _internalState = PBR_STATE_I_OFF;
            _procStart = 0;
        }
      }

      // default return value, result still unclear
      return PBR_STATE_E_OFF;
    }
};


// Constants for the PushButtonDoubleLongAnalyzer implementation based on SwitchStateAnalyzer
#define PBDL_STATE_E_OFF          0x00   // external relevant output state, not pushed
#define PBDL_STATE_E_SINGLE       0x01   // external relevant output state, one single push and release
#define PBDL_STATE_E_DOUBLE       0x02   // external relevant output state, two pushs and releases
#define PBDL_STATE_E_LONG         0x03   // external relevant output state, one long push and release
#define PBDL_STATE_I_OFF          0x04   // internal state, not pushed
#define PBDL_STATE_I_ON           0x05   // internal state, button currently pushed, potential sequence start
#define PBDL_STATE_I_DBLOFF       0x06   // internal state, OFF state in between pushes of potential double push
#define PBDL_STATE_I_DBL2ON       0x07   // internal state, 2. ON phase of potential double push
#define PBDL_STATE_I_LONGTIMEOUT  0x08   // internal state, timeout of long push reached
#define PBDL_STATE_I_UNDEFINED    0xFF   // internal state unclear


/*
  Implementation of an analyzer, that analyzes the push sequences "single push"
  and "double push" and "long push" of a button.
  Supports up to 65 seconds push duration.

  Analysis of button input signals from 1 digital pin (2 raw states)
  to deliver the information (4 states) which complete sequences were identified:

  no push (PBDL_STATE_E_OFF)
  single push completed (PBDL_STATE_E_SINGLE)
  double push completed (PBDL_STATE_E_DOUBLE)
  long push completed (PBDL_STATE_E_LONG)

  Prerequisite are the 2 raw input states
  SW_STATE_DEFAULT_ON and SW_STATE_DEFAULT_OFF

  The return of an PBDL_STATE_E_SINGLE is based on the input sequence
  OFF -> ON (< minLongMillis) -> OFF
  and no second push/release within maxDoubleMillis
  
  The return of an PBDL_STATE_E_DOUBLE is based on the input sequence
  OFF -> ON -> OFF -> ON -> OFF (completed < maxDoubleMillis)

  The return of an PBDL_STATE_E_LONG is based on the input sequence
  OFF -> ON (>= minLongMillis) -> OFF

  Details see Switch.h
*/
class PushButtonDoubleLongAnalyzer : public SwitchStateAnalyzer {
  private:
    // internal storage for parameter maxDoubleMillis, maximun time for completing a double push
    unsigned int _maxDoubleMillis;
    // internal storage for parameter minLongMillis, minimum time a long press must last
    unsigned int _minLongMillis;
    // internal storage for parameter endLongByTime,
    // defines if long push should end based on time or SW_STATE_DEFAULT_OFF input
    boolean _endLongByTime;
    // last start of sequence beginning with SW_STATE_DEFAULT_ON
    unsigned long _procStart;
    // internal state within the sequence
    byte _internalState;

    /*
      Internal initialization with handover of the constructors parameters
    */
    void init(unsigned int maxDoubleMillis, unsigned int minLongMillis, boolean endLongByTime){
      _maxDoubleMillis = maxDoubleMillis;
      _minLongMillis = minLongMillis;
      _endLongByTime = endLongByTime;
      reset();
    }

  public:
    /*
      Default instantiation of this analyzer.
      Only single pushes of the button will be identified.
    */
    PushButtonDoubleLongAnalyzer() : SwitchStateAnalyzer() {
      PushButtonDoubleLongAnalyzer::init(0, 0, false);
    }

    /*
      Instantiation of this analyzer for analyzing single, double and long pushes.
      Supports up to 65 seconds push duration.

      @param maxDoubleMillis  Maximun time in milliseconds for completing a double push.
                              Value 0 means, that double push analysis is switched off
      @param minLongMillis    Minimum time in milliseconds that a push must last
                              to be identified as long push.
                              Value 0 means, that long push analysis is switched off
      @param endLongByTime    Only relevant for long push analysis.
                              Defines if long push should end based on minLongMillis
                              time (tue) or SW_STATE_DEFAULT_OFF input (false)
    */
    PushButtonDoubleLongAnalyzer(unsigned int maxDoubleMillis, unsigned int minLongMillis,
      boolean endLongByTime) : SwitchStateAnalyzer () {
      PushButtonDoubleLongAnalyzer::init(maxDoubleMillis, minLongMillis, endLongByTime);
    }

    byte getReadCycleMillis() {
      return max(_maxDoubleMillis, _minLongMillis) / 20;
    };

    void reset(){
      _internalState = PBDL_STATE_I_UNDEFINED;
      _procStart = 0;
    }

    byte getNumAnalyzerStates(){
      return 4;
    }

    byte getNumSwitchStates(){
      return 2;
    }

    /*
      @param switchState  Input must be one of the 2 raw states
                          SW_STATE_DEFAULT_ON and SW_STATE_DEFAULT_OFF
      @returns            One of the 4 output states PBDL_STATE_E_OFF,
                          PBDL_STATE_E_SINGLE, PBDL_STATE_E_DOUBLE, PBDL_STATE_E_LONG
    */
    byte getAnalyzerState(byte switchState){
      unsigned int procDuration;
      unsigned long current = millis();
      boolean stateOn = (switchState == SW_STATE_DEFAULT_ON);

      if(_internalState == PBDL_STATE_I_UNDEFINED){
        if(stateOn){
          _procStart = current;
          _internalState = PBDL_STATE_I_ON;
        }
        else{
          _internalState = PBDL_STATE_I_OFF;
        }
      }

      // calculate times, up to 65 seconds push duration safely supported
      procDuration = (unsigned int)(current - _procStart);

      // handle time outs first
      if(_minLongMillis > 0 && procDuration > _minLongMillis
          && _internalState == PBDL_STATE_I_ON && _endLongByTime){
        _internalState = PBDL_STATE_I_LONGTIMEOUT;
        return PBDL_STATE_E_LONG;
      }

      if(_maxDoubleMillis > 0 && procDuration > _maxDoubleMillis
          && (_internalState == PBDL_STATE_I_DBLOFF || _internalState == PBDL_STATE_I_DBL2ON)){
        // one push has been completed and second push comes/will come to late
        if(stateOn){
          _procStart = current;
          _internalState = PBDL_STATE_I_ON;
        }
        else{
          _procStart = 0;
          _internalState = PBDL_STATE_I_OFF;
        }
        return PBDL_STATE_E_SINGLE;
      }

      // normal state changes
      if(_internalState == PBDL_STATE_I_OFF){
        if(stateOn){
          // start of a new sequence
          _procStart = current;
          _internalState = PBDL_STATE_I_ON;
        }
      }
      else if(_internalState == PBDL_STATE_I_ON){
        if(stateOn){
          // this can be ignored
        }
        else if(_minLongMillis > 0 && procDuration > _minLongMillis){
          _internalState = PBDL_STATE_I_OFF;
          _procStart = 0;
          return PBDL_STATE_E_LONG;
        }
        else if(_maxDoubleMillis > 0 && procDuration < _maxDoubleMillis){
          _internalState = PBDL_STATE_I_DBLOFF;
        }
        else{
          _internalState = PBDL_STATE_I_OFF;
          _procStart = 0;
          return PBDL_STATE_E_SINGLE;
        }
      }
      else if(_internalState == PBDL_STATE_I_DBLOFF){
        if(stateOn){
          // start the second push, time out will be check next time above
          _procStart = current;
          _internalState = PBDL_STATE_I_DBL2ON;
        }
      }
      else if(_internalState == PBDL_STATE_I_DBL2ON){
        if(!stateOn){
          // end of the second push, time out possibility was already checked above
          _internalState = PBDL_STATE_I_OFF;
          _procStart = 0;
          return PBDL_STATE_E_DOUBLE;
        }
      }
      else if(_internalState == PBDL_STATE_I_LONGTIMEOUT){
        if(!stateOn){
          // final end of the long push, time out was already done above
          _internalState = PBDL_STATE_I_OFF;
          _procStart = 0;
        }
      }

      // default return value, result may be unclear
      return PBDL_STATE_E_OFF;
    }
};

#endif
