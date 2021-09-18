#ifndef LEDLIGHTINGCYCLE_H
#define LEDLIGHTINGCYCLE_H

#include "LEDLightingEffect.h"

#define CYCLE_OFF 0
#define CYCLE_OFF_TO_ON 1
#define CYCLE_ON 2
#define CYCLE_ON_TO_OFF 3

/**
   Base class for lighting cycle execution.
   Objects of this type govern when the LED is powered on.

   Additional effects when the LED is powered on or when it
   trasitions between the states are governed by effect classes.
*/
class LEDStaticLighting {
  protected:
    unsigned char _currentState;
    unsigned char _brightness;
    unsigned char const _ledPin;

    LEDOneShotEffect * const _offToOnEffect;
    LEDOneShotEffect * const _onToOffEffect;
    LEDCyclicEffect * const _onEffect;

    void lightOff();
    void lightOn();
    char lightOffToOn();
    char lightOnToOff();
    void resetTransitions();

  public:
    LEDStaticLighting(unsigned char const ledPin, unsigned char const brightness, unsigned char const initialState = CYCLE_ON,
                      LEDCyclicEffect * const onEffect = new LEDCyclicEffect(), LEDOneShotEffect * const offToOnEffect = 0, LEDOneShotEffect * const onToOffEffect = 0);
    virtual void execute();
};

class LEDLightingCycle: public LEDStaticLighting {
  protected:
    unsigned long _onTimeMs;
    unsigned long _offTimeMs;
    unsigned long _timeOfNextSwitchMs;

  public:
    LEDLightingCycle(unsigned char const ledPin, unsigned char const brightness,
                     unsigned long const onTimeMs, unsigned long const offTimeMs,
                     LEDCyclicEffect * const onEffect = new LEDCyclicEffect(),
                     LEDOneShotEffect * const offToOnEffect = 0,
                     LEDOneShotEffect * const onToOffEffect = 0);
    void execute();
};

class LEDRandomLightingCycle: public LEDStaticLighting {
  protected:
    unsigned long _onTimeMinMs;
    unsigned long _onTimeMaxMs;
    unsigned long _offTimeMinMs;
    unsigned long _offTimeMaxMs;
    unsigned long _timeOfNextSwitchMs;

  public:
    LEDRandomLightingCycle(unsigned char const ledPin, unsigned char const brightness,
                           unsigned long const onTimeMinMs, unsigned long const onTimeMaxMs,
                           unsigned long const offTimeMinMs, unsigned long const offTimeMaxMs,
                           LEDCyclicEffect * const onEffect = new LEDCyclicEffect(),
                           LEDOneShotEffect * const offToOnEffect = 0,
                           LEDOneShotEffect * const onToOffEffect = 0);
    void execute();
};
#endif
