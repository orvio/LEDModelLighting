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
#include <arduino.h>

/*
   LEDStaticLighting
*/
LEDStaticLighting::LEDStaticLighting(unsigned char const ledPin,
                                     unsigned char const brightness,
                                     unsigned char const initialState,
                                     LEDCyclicEffect * const onEffect,
                                     LEDOneShotEffect * const offToOnEffect,
                                     LEDOneShotEffect * const onToOffEffect):
  _brightness(brightness),
  _ledPin(ledPin),
  _currentState(initialState),
  _onEffect(onEffect),
  _offToOnEffect(offToOnEffect),
  _onToOffEffect(onToOffEffect)
{}

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

char LEDStaticLighting::lightOffToOn() {
  if ( not _offToOnEffect) {
    return 1;
  }

  analogWrite(_ledPin, _offToOnEffect->getBrightness(_brightness));

  return _offToOnEffect->isFinished();
}


char LEDStaticLighting::lightOnToOff() {
  if ( not _onToOffEffect) {
    return 1;
  }

  analogWrite(_ledPin, _onToOffEffect->getBrightness(_brightness));

  return _onToOffEffect->isFinished();
}

/*
   LEDLightingCycle
*/
LEDLightingCycle::LEDLightingCycle(unsigned char const ledPin, unsigned char const brightness,
                                   unsigned long const onTimeMs, unsigned long const offTimeMs,
                                   LEDCyclicEffect * const onEffect,
                                   LEDOneShotEffect * const offToOnEffect, LEDOneShotEffect * const onToOffEffect):
  LEDStaticLighting(ledPin, brightness, CYCLE_OFF, onEffect, offToOnEffect, onToOffEffect),
  _onTimeMs(onTimeMs),
  _offTimeMs(offTimeMs),
  _timeOfNextSwitchMs(0)
{}

void LEDLightingCycle::execute() {
  unsigned long currentTime = millis();

  //where are we in our cycle?
  switch (_currentState) {
    case CYCLE_OFF:
      lightOff();
      if (currentTime > _timeOfNextSwitchMs) {
        //time has elaped -> switch to on and reset timer
        _currentState = CYCLE_OFF_TO_ON;
        resetTransitions();
        _timeOfNextSwitchMs = currentTime + _onTimeMs;
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
        //time has elaped -> switch to off and reset timer
        _currentState = CYCLE_ON_TO_OFF;
        resetTransitions();
        _timeOfNextSwitchMs = currentTime + _offTimeMs;
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

/*
   LEDRandomLightingCycle
*/
LEDRandomLightingCycle::LEDRandomLightingCycle(unsigned char const ledPin, unsigned char const brightness,
    unsigned long const onTimeMinMs, unsigned long const onTimeMaxMs,
    unsigned long const offTimeMinMs, unsigned long const offTimeMaxMs,
    LEDCyclicEffect * const onEffect,
    LEDOneShotEffect * const offToOnEffect,
    LEDOneShotEffect * const onToOffEffect ):
  LEDStaticLighting(ledPin, brightness, CYCLE_ON, onEffect, offToOnEffect, onToOffEffect),
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
