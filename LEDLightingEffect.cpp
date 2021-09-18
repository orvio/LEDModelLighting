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
#include "LEDLightingEffect.h"
#include <arduino.h>

unsigned char LEDLightingEffect::getBrightness( unsigned char const maxBrightness) {
  return maxBrightness;
}

/*
   KEDOneShotEffect
*/
LEDOneShotEffect::LEDOneShotEffect(unsigned short const durationMs, const unsigned short maxStartDelayMs):
  _durationMs(durationMs),
  _maxStartDelayMs(maxStartDelayMs)
{}

unsigned short LEDOneShotEffect::getDurationMs() {
  return _durationMs;
}

void LEDOneShotEffect::reset() {
  _startMs = millis();
  _startDelayMs = random(0, _maxStartDelayMs);
}

char LEDOneShotEffect::isFinished() {
  //return millis() - _startMs - _startDelayMs > _durationMs;
  return not getRemainingDuration(millis());
}

unsigned short LEDOneShotEffect::getRemainingDuration(const unsigned long currentTimeMs) {
  if ((_startMs + _durationMs + _startDelayMs) > currentTimeMs) {
    //safe to do the subtraction without risking wraparound
    return (_startMs + _durationMs + _startDelayMs) - currentTimeMs;
  }

  //current time is already after the end of the effect
  return 0;
}

unsigned short LEDOneShotEffect::getRemainingStartDelay(const unsigned long currentTimeMs) {
  if ((_startMs + _startDelayMs) > currentTimeMs) {
    //safe to do the subtraction without risking wraparound
    return (_startMs + _startDelayMs) - currentTimeMs;
  }

  //current time is already after the end of the start delay
  return 0;
}

/*
   FadeEffect
*/
FadeEffect::FadeEffect(unsigned short const durationMs, unsigned char const fadeDirection, const unsigned short maxStartDelayMs):
  LEDOneShotEffect(durationMs, maxStartDelayMs),
  _fadeDirection(fadeDirection)
{}

unsigned char FadeEffect::getBrightness( unsigned char const maxBrightness) {
  const unsigned long currentTimeMs = millis();
  if (getRemainingStartDelay(currentTimeMs)) {
    if ( _fadeDirection == FADE_OUT ) {
      return maxBrightness;
    }
    else {
      return 0;
    }
  }

  float fadeProgressPercent = (currentTimeMs - (_startMs + _startDelayMs)) / (float)_durationMs;
  if ( _fadeDirection == FADE_OUT ) {
    fadeProgressPercent = 1 - fadeProgressPercent;
  }
  return fadeProgressPercent * maxBrightness;
}

/*
   FluorescentStartEffect
*/
FluorescentStartEffect::FluorescentStartEffect(unsigned short const minDurationMs,
    unsigned short const durationMs,
    const unsigned short maxStartDelayMs):
  LEDOneShotEffect(durationMs, maxStartDelayMs),
  _minDurationMs(minDurationMs),
  _currentStageStartTimeMs(0),
  _currentStageDurationMs(0),
  _currentStage(START_UNINITIALIZED)
{}

unsigned short FluorescentStartEffect::getRemainingDuration(const unsigned long currentTimeMs) {
  if ((_startMs + _currentDurationMs + _startDelayMs) > currentTimeMs) {
    //safe to do the subtraction without risking wraparound
    return (_startMs + _currentDurationMs + _startDelayMs) - currentTimeMs;
  }

  //current time is already after the end of the effect
  return 0;
}

unsigned char FluorescentStartEffect::getNextStage(const unsigned long currentTimeMs) {
  unsigned char nextStage = START_ON; //default stage if something weird happens...
  const long randomNumber = random(0, 10);
  unsigned char isFloatAllowed = 0;

  const unsigned short elapsedTimeMs = currentTimeMs - (_startMs + _startDelayMs);
  if ( elapsedTimeMs > (_currentDurationMs  / 2)) {
    isFloatAllowed = 1;
  }


  switch (_currentStage) {
    case START_OFF:
      if (randomNumber < 8) {
        nextStage = START_FLICKER;
      }
      else {
        if (isFloatAllowed) {
          nextStage = START_FLOAT;
        }
        else {
          nextStage = START_FLICKER;
        }
      }
      break;
    case START_FLICKER:
      if (randomNumber < 8) {
        nextStage = START_OFF;
      }
      else {
        if (isFloatAllowed) {
          nextStage = START_FLOAT;
        }
        else {
          nextStage = START_OFF;
        }
      }
      break;
    case START_FLOAT:
      if (randomNumber) {
        nextStage = START_FLICKER;
      }
      else {
        nextStage = START_OFF;
      }
      break;
  }

  return nextStage;
}

void FluorescentStartEffect::setupNextStage(const unsigned long currentTimeMs) {
  _currentStage = getNextStage(currentTimeMs);
  _currentStageStartTimeMs = currentTimeMs;

  switch (_currentStage) {
    case START_OFF:
      _currentStageDurationMs = random(START_OFF_MIN_DURATION_MS, START_OFF_MAX_DURATION_MS);
      break;
    case START_FLICKER:
      _currentStageDurationMs = random(START_FLICKER_MIN_DURATION_MS, START_FLICKER_MIN_DURATION_MS);
      break;
    case START_FLOAT:
      _currentStageDurationMs = random(START_FLOAT_MIN_DURATION_MS, START_FLOAT_MAX_DURATION_MS);
      break;
    default: //should not happen, so just stay here for the remaining duration...
      _currentStageDurationMs = getRemainingDuration(currentTimeMs);
      break;
  }
}

unsigned char FluorescentStartEffect::getBrightness( unsigned char const maxBrightness) {
  unsigned long currentTimeMs = millis();
  unsigned char brightness = 0;

  if (getRemainingStartDelay(currentTimeMs)) {
    return brightness;
  }

  if (_currentStage == START_UNINITIALIZED) {
    _currentStage = START_FLICKER;
    _currentStageStartTimeMs = currentTimeMs;
    _currentStageDurationMs = random(START_FLICKER_MIN_DURATION_MS, START_FLICKER_MAX_DURATION_MS);
  }

  if (not getRemainingDuration(currentTimeMs)) {
    //switch to solid on if the current effect duration has elapsed
    _currentStage = START_ON;
  }

  unsigned char stageTimeElaped = (currentTimeMs >= (_currentStageStartTimeMs + _currentStageDurationMs));
  switch (_currentStage) {
    case  START_OFF:
      brightness = 0;
      if (stageTimeElaped) {
        setupNextStage(currentTimeMs);
      }
      break;
    case START_FLICKER:
      brightness = maxBrightness;
      if (stageTimeElaped) {
        setupNextStage(currentTimeMs);
      }
      break;
    case  START_FLOAT:
      {
        float cycleProgressPercent = (currentTimeMs % _currentStageDurationMs) / (float)_currentStageDurationMs;
        float rad = 2 * PI * cycleProgressPercent;
        brightness = (maxBrightness / 3) +  ((maxBrightness / 10) * sin(rad));
      }
      if (stageTimeElaped) {
        _currentStage = START_ON; //always go to on after float
      }
      break;
    case START_ON:
      //no stage time for this state
      brightness = maxBrightness;
      break;
    default:
      break;
  }

  return brightness;
}

void FluorescentStartEffect::reset() {
  LEDOneShotEffect::reset(); // call parent implementation first
  _currentStage = START_UNINITIALIZED;
  _currentDurationMs = random(_minDurationMs, _durationMs);
}

/*
   BeaconEffect
*/
BeaconEffect::BeaconEffect(unsigned int const cycleTimeMs):
  _cycleTimeMs(cycleTimeMs)
{}

unsigned char BeaconEffect::getBrightness( unsigned char const maxBrightness) {
  const float rampupPercent = 0.1; //10 percent
  const float cycleProgressPercent = (millis() % _cycleTimeMs) / (float)_cycleTimeMs;

  float rad = 0;
  if (cycleProgressPercent < 0.25) {
    rad = PI * rampupPercent * cycleProgressPercent / 0.25; //linear ramp up from 0 to PI/10
  }
  else if (cycleProgressPercent < 0.5) {
    rad = PI * rampupPercent + (1 - rampupPercent) * (2 * PI * (cycleProgressPercent - 0.25) * 4); //*4 to squeeze the entire cosine into a quarter cycle.
  }
  else if (cycleProgressPercent < 0.75) {
    rad = PI * rampupPercent * (0.75 - cycleProgressPercent) / 0.25; //linear ramp down from PI/10 to 0
  }
  rad -= PI;
  double cosine = cos(rad) * 0.5   + 0.5;
  return (cosine) * maxBrightness;
}
