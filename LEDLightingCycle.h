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
#ifndef LEDLIGHTINGCYCLE_H
#define LEDLIGHTINGCYCLE_H

#include "LEDLightingEffect.h"

/**
   Base class for lighting cycle execution.
   Objects of this type govern when the LED is powered on.

   Additional effects when the LED is powered on or when it
   trasitions between the states are governed by effect classes.
*/
class LEDStaticLighting {
#define CYCLE_OFF 0
#define CYCLE_OFF_TO_ON 1
#define CYCLE_ON 2
#define CYCLE_ON_TO_OFF 3
  protected:
    unsigned char _currentState; ///current state of the output pin
    unsigned char _brightness; ///Brightness of the output pin. If effects are configured, this value is the maximum brightness.
    unsigned char const _ledPin; ///pin number of the output

    LEDOneShotEffect * const _offToOnEffect; ///Effect to be used when the output transitions from CYCLE_OFF to CYCLE_ON
    LEDOneShotEffect * const _onToOffEffect; ///Effect to be used when the output transitions fron CYCLE_ON to CYCLE_OFF
    LEDCyclicEffect * const _onEffect; ///Effect to be used when the output is active

    /**
       Turns the output off
    */
    void lightOff();

    /**
      @brief Sets the output to on.

      The actual brightness of the output is governed by _onEffect
    */
    void lightOn();

    /**
      @brief Sets the brightness of the out when transitioning from CYCLE_ON to CYCLE_ON

      Call this method when transitioning from CYCLE_OFF to CYCLE_ON.
      This method checks if _offToOnEffect is set and fetch the current brightness value
      from the effect class.

      @returns 0 as long as the effect has not finished executing
    */
    char lightOffToOn();

    /**
      @brief Sets the brightness of the out when transitioning from CYCLE_ON to CYCLE_OFF

      Call this method when transitioning from CYCLE_ON to CYCLE_OFF.
      This method checks if _onToOffEffect is set and fetch the current brightness value
      from the effect class.

      @returns 0 as long as the effect has not finished executing
    */
    char lightOnToOff();

    /**
      Resets both the _offToOnEffect and _onToOffEffect to setup the next effect execution cycle.
    */
    void resetTransitions();

  public:
    /**
       Creates a new LEDStaticLighting object. This lighting object will keep the output active with the
       configured brightness if the initial state is set to CYCLE_ON. All other states will result in the output
       being turned off.

       The assigned pin will be configured as OUTPUT.

       @param ledPin number of the pin to be used. Arduino defines like LED_BUILTIN are allowed
       @param brightness sets the PWM duty cycle from 0 (off) to 255 (full brightness)
       @param initialState only CYCLE_ON will result in an output
       @param onEffect sets the effect class to use when the output is active
       @param offToOnEffect set the effect class to use when the output state transitions from CYCLE_OFF to CYCLE_ON
       @param onToOffEffect set the effect class to use when the output state transitions from CYCLE_ON to CYCLE_OFF
    */
    LEDStaticLighting(unsigned char const ledPin, unsigned char const brightness, unsigned char const initialState = CYCLE_ON,
                      LEDCyclicEffect * const onEffect = new LEDCyclicEffect(), LEDOneShotEffect * const offToOnEffect = 0, LEDOneShotEffect * const onToOffEffect = 0);

    /**
       This method needs to be called in the loop() function of the sketch.
    */
    virtual void execute();
};

/**
   This class toggles between on and off state based on the minimum and maximum
   duration for each state.
*/
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

/**
   This class cycles between the on and off state with a fixed timing for each state.
*/
class LEDLightingCycle: public LEDRandomLightingCycle {
  public:
    LEDLightingCycle(unsigned char const ledPin, unsigned char const brightness,
                     unsigned long const onTimeMs, unsigned long const offTimeMs,
                     LEDCyclicEffect * const onEffect = new LEDCyclicEffect(),
                     LEDOneShotEffect * const offToOnEffect = 0,
                     LEDOneShotEffect * const onToOffEffect = 0);
};
#endif
