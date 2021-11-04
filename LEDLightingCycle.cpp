/*
    This file is part of LEDModelLighting.

    LEDModelLighting is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LEDModelLighting is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LEDModelLighting.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "LEDLightingCycle.h"
#include <Arduino.h>

/*
   LEDStaticLighting
*/
LEDStaticLighting::LEDStaticLighting(unsigned char const ledPin,
                                     unsigned char const brightness,
                                     const CycleStates initialState,
                                     LEDCyclicEffect * const onEffect,
                                     LEDOneShotEffect * const offToOnEffect,
                                     LEDOneShotEffect * const onToOffEffect):
  _brightness(brightness),
  _ledPin(ledPin),
  _currentState(initialState),
  _onEffect(onEffect),
  _offToOnEffect(offToOnEffect),
  _onToOffEffect(onToOffEffect)
{
  pinMode(_ledPin, OUTPUT);
}

void LEDStaticLighting::execute() {
  switch (_currentState) {
    case CYCLE_OFF:
      lightOff();
      break;
    case CYCLE_ON:
      lightOn();
      break;
    default:
      lightOff();
  }
}

void LEDStaticLighting::lightOn() {
  analogWrite(_ledPin, _onEffect->getBrightness(_brightness));
}

void LEDStaticLighting::lightOff() {
  digitalWrite(_ledPin, LOW);
}

void LEDStaticLighting::resetTransitions() {
  if ( _offToOnEffect) {
    _offToOnEffect->reset();
  }

  if ( _onToOffEffect) {
    _onToOffEffect->reset();
  }
}

bool LEDStaticLighting::lightOffToOn() {
  if ( not _offToOnEffect) {
    return 1;
  }

  analogWrite(_ledPin, _offToOnEffect->getBrightness(_brightness));

  return _offToOnEffect->isFinished();
}


bool LEDStaticLighting::lightOnToOff() {
  if ( not _onToOffEffect) {
    return 1;
  }

  analogWrite(_ledPin, _onToOffEffect->getBrightness(_brightness));

  return _onToOffEffect->isFinished();
}

bool LEDStaticLighting::isOutputActive() const {
  return (_currentState == CYCLE_ON) || (_currentState == CYCLE_OFF_TO_ON);
}

/*
  LEDTriggeredCycle
*/
LEDTriggeredCycle::LEDTriggeredCycle(unsigned char const ledPin,
                                     unsigned char const brightness,
                                     unsigned long const onDelayMinMs,
                                     unsigned long const onDelayMaxMs,
                                     unsigned long const offDelayMinMs,
                                     unsigned long const offDelayMaxMs,
                                     unsigned char & trigger,
                                     LEDCyclicEffect * const onEffect,
                                     LEDOneShotEffect * const offToOnEffect,
                                     LEDOneShotEffect * const onToOffEffect):
  LEDStaticLighting(ledPin, brightness, CYCLE_OFF, onEffect, offToOnEffect, onToOffEffect),
  _nextSwitchTimeMs(0),
  _onDelayMinMs(onDelayMinMs),
  _onDelayMaxMs(onDelayMaxMs),
  _offDelayMinMs(offDelayMinMs),
  _offDelayMaxMs(offDelayMaxMs),
  _trigger(trigger)
{

}

void LEDTriggeredCycle::execute() {
  const unsigned long currentTimeMs = millis();

  switch (_currentState) {
    case CYCLE_OFF:
      lightOff();
      if (_trigger) {
        if (not _nextSwitchTimeMs) {
          _nextSwitchTimeMs = currentTimeMs + random(_onDelayMinMs, _onDelayMaxMs);
        }

        if ( currentTimeMs > _nextSwitchTimeMs ) {
          _nextSwitchTimeMs = 0;
          resetTransitions();
          _currentState = CYCLE_OFF_TO_ON;
        }
      }
      break;
    case CYCLE_OFF_TO_ON:
      {
        const char isTransitionDone = lightOffToOn();
        if (not _trigger) {
          if (not _nextSwitchTimeMs) {
            _nextSwitchTimeMs = currentTimeMs + random(_offDelayMinMs, _offDelayMaxMs);
          }

          if ( currentTimeMs > _nextSwitchTimeMs ) {
            _nextSwitchTimeMs = 0;
            resetTransitions();
            _currentState = CYCLE_ON_TO_OFF;
          }
          else if (isTransitionDone) {
            _currentState = CYCLE_ON;
          }
        }
        else if ( isTransitionDone ) {
          _currentState = CYCLE_ON;
        }
      }
      break;
    case CYCLE_ON:
      lightOn();
      if (not _trigger) {
        if (not _nextSwitchTimeMs) {
          _nextSwitchTimeMs = currentTimeMs + random(_offDelayMinMs, _offDelayMaxMs);
        }

        if ( currentTimeMs > _nextSwitchTimeMs ) {
          _nextSwitchTimeMs = 0;
          resetTransitions();
          _currentState = CYCLE_ON_TO_OFF;
        }
      }
      break;
    case CYCLE_ON_TO_OFF:
      {
        const char isTransitionDone = lightOnToOff();
        if (_trigger) {
          if (not _nextSwitchTimeMs) {
            _nextSwitchTimeMs = currentTimeMs + random(_onDelayMinMs, _onDelayMaxMs);
          }

          if ( currentTimeMs > _nextSwitchTimeMs ) {
            _nextSwitchTimeMs = 0;
            resetTransitions();
            _currentState = CYCLE_OFF_TO_ON;
          }
          else if (isTransitionDone) {
            _currentState = CYCLE_OFF;
          }
        }
        else if ( isTransitionDone ) {
          _currentState = CYCLE_OFF;
        }
      }
      break;
    default:
      break;
  }
}

/*
   LEDChainedCycle
*/
LEDChainedCycle::LEDChainedCycle(unsigned char const ledPin,
                                 unsigned char const brightness,
                                 LEDStaticLighting const * const  masterCycle,
                                 const unsigned long onDelayMinMs,
                                 const unsigned long  onDelayMaxMs,
                                 const unsigned long  onTimeMinMs,
                                 const unsigned long  onTimeMaxMs,
                                 LEDCyclicEffect * const onEffect,
                                 LEDOneShotEffect * const offToOnEffect,
                                 LEDOneShotEffect * const onToOffEffect):
  LEDStaticLighting(ledPin, brightness, CYCLE_OFF, onEffect, offToOnEffect, onToOffEffect),
  _onDelayMinMs(onDelayMinMs),
  _onDelayMaxMs(onDelayMaxMs),
  _onTimeMinMs(onTimeMinMs),
  _onTimeMaxMs(onTimeMaxMs),
  _masterCycle(masterCycle),
  _nextSwitchTimeMs(0),
  _outputWasOn(false)
{

}

void LEDChainedCycle::execute() {
  const unsigned long currentTimeMs = millis();

  switch (_currentState) {
    case CYCLE_OFF:
      lightOff();
      if (_masterCycle->isOutputActive()) {
        if ( not _outputWasOn ) {
          if (not _nextSwitchTimeMs) {
            _nextSwitchTimeMs = currentTimeMs + random(_onDelayMinMs, _onDelayMaxMs);
          }

          if ( currentTimeMs > _nextSwitchTimeMs ) {
            _nextSwitchTimeMs = 0;
            resetTransitions();
            _currentState = CYCLE_OFF_TO_ON;
            _outputWasOn = true;
          }
        }
      }
      else {
        _outputWasOn = false;
      }
      break;
    case CYCLE_OFF_TO_ON:
      if (not _masterCycle->isOutputActive()) {
        resetTransitions();
        _currentState = CYCLE_ON_TO_OFF;
      }
      else if (lightOffToOn()) {
        _currentState = CYCLE_ON;
      }
      break;
    case CYCLE_ON:
      lightOn();
      if (not _masterCycle->isOutputActive()) {
        _nextSwitchTimeMs = 0;
        resetTransitions();
        _currentState = CYCLE_ON_TO_OFF;
      } else {
        if (not _nextSwitchTimeMs) {
          _nextSwitchTimeMs = currentTimeMs + random(_onTimeMinMs, _onTimeMaxMs);
        }

        if ( currentTimeMs > _nextSwitchTimeMs ) {
          _nextSwitchTimeMs = 0;
          resetTransitions();
          _currentState = CYCLE_ON_TO_OFF;
        }
      }
      break;
    case CYCLE_ON_TO_OFF:
      if ( lightOnToOff() ) {
        _currentState = CYCLE_OFF;
      }
      break;
    default:
      _currentState = CYCLE_OFF;
  }
}

/*
   LEDLightingCycle
*/
LEDLightingCycle::LEDLightingCycle(unsigned char const ledPin, unsigned char const brightness,
                                   unsigned long const onTimeMs, unsigned long const offTimeMs,
                                   LEDCyclicEffect * const onEffect,
                                   LEDOneShotEffect * const offToOnEffect, LEDOneShotEffect * const onToOffEffect):
  LEDRandomLightingCycle(ledPin, brightness, onTimeMs, onTimeMs, offTimeMs, offTimeMs, onEffect, offToOnEffect, onToOffEffect)
{}

/*
   LEDRandomLightingCycle
*/
LEDRandomLightingCycle::LEDRandomLightingCycle(unsigned char const ledPin, unsigned char const brightness,
    unsigned long const onTimeMinMs, unsigned long const onTimeMaxMs,
    unsigned long const offTimeMinMs, unsigned long const offTimeMaxMs,
    LEDCyclicEffect * const onEffect,
    LEDOneShotEffect * const offToOnEffect,
    LEDOneShotEffect * const onToOffEffect ):
  LEDStaticLighting(ledPin, brightness, CYCLE_OFF, onEffect, offToOnEffect, onToOffEffect),
  _onTimeMinMs(onTimeMinMs),
  _onTimeMaxMs(onTimeMaxMs),
  _offTimeMinMs(offTimeMinMs),
  _offTimeMaxMs(offTimeMaxMs),
  _timeOfNextSwitchMs(0)
{}

void LEDRandomLightingCycle::execute() {
  unsigned long currentTime = millis();

  //where are we in our cycle?
  switch (_currentState) {
    case CYCLE_OFF:
      lightOff();
      if (currentTime > _timeOfNextSwitchMs) {
        //time has elaped -> switch to on and calculate duration
        _currentState = CYCLE_OFF_TO_ON;
        resetTransitions();
        _timeOfNextSwitchMs = currentTime + _onTimeMinMs
                              + random(0, _onTimeMaxMs - _onTimeMinMs);
      }
      break;
    case CYCLE_OFF_TO_ON:
      if ( lightOffToOn() ) {
        _currentState = CYCLE_ON;
      }
      break;
    case CYCLE_ON:
      lightOn();
      if (currentTime > _timeOfNextSwitchMs) {
        //time has elaped -> switch to off and calculate duration
        _currentState = CYCLE_ON_TO_OFF;
        resetTransitions();
        _timeOfNextSwitchMs = currentTime + _offTimeMinMs
                              + random(0, _offTimeMaxMs - _offTimeMinMs);;
      }
      break;
    case CYCLE_ON_TO_OFF:
      if (lightOnToOff()) {
        _currentState = CYCLE_OFF;
      }
      break;
    default:
      _currentState = CYCLE_OFF;
  }
}
