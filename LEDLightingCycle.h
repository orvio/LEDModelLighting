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
   @brief Base class for lighting cycle execution.

   Objects of this type govern when the LED is powered on.

   Additional effects when the LED is powered on or when it
   transitions between the states are governed by effect classes.
*/
class LEDStaticLighting {
  public:
    ///State enumeration for standard states of a lighting object
    enum CycleStates {
      ///The output is off
      CYCLE_OFF,
      ///The output is transitioning from off to on using #_offToOnEffect
      CYCLE_OFF_TO_ON,
      ///The output is transitioning from on to off using #_onToOffEffect
      CYCLE_ON,
      ///The output is on using the #_onEffect
      CYCLE_ON_TO_OFF
    };

    /**
       @brief Creates a new LEDStaticLighting object.

       This lighting object will keep the output active with the
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
    LEDStaticLighting(unsigned char const ledPin, unsigned char const brightness, const CycleStates initialState = CYCLE_ON,
                      LEDCyclicEffect * const onEffect = new LEDCyclicEffect(), LEDOneShotEffect * const offToOnEffect = 0, LEDOneShotEffect * const onToOffEffect = 0);

    /**
       @brief This method needs to be called in the loop() function of the sketch.

       The default implementation calls either #lightOn() or #lightOff() based on #_currentState.
    */
    virtual void execute();

    /**
      @brief returns true if the output is active.

      The output counts as active as soon as #_offToOnEffect starts executing.

      @return true if the output is active
    */
    bool isOutputActive() const;

  protected:
    ///current state of the output pin
    CycleStates _currentState;
    ///Brightness of the output pin. If effects are configured, this value is the maximum brightness.
    unsigned char _brightness;
    ///pin number of the output
    unsigned char const _ledPin;

    ///Effect to be used when the output transitions from CYCLE_OFF to CYCLE_ON
    LEDOneShotEffect * const _offToOnEffect;
    ///Effect to be used when the output transitions fron CYCLE_ON to CYCLE_OFF
    LEDOneShotEffect * const _onToOffEffect;
    ///Effect to be used when the output is active
    LEDCyclicEffect * const _onEffect;

    /**
      @brief Turns the output off.
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

      @returns false as long as the effect has not finished executing
    */
    bool lightOffToOn();

    /**
      @brief Sets the brightness of the out when transitioning from CYCLE_ON to CYCLE_OFF

      Call this method when transitioning from CYCLE_ON to CYCLE_OFF.
      This method checks if _onToOffEffect is set and fetch the current brightness value
      from the effect class.

      @returns false as long as the effect has not finished executing
    */
    bool lightOnToOff();

    /**
      @brief Resets both the _offToOnEffect and _onToOffEffect to setup the next effect execution cycle.
    */
    void resetTransitions();
};

/**
  @brief This class watches a trigger variable to determine the on and off state of the LED

  A reference to an unsigned char variable is monitored to determine the target state of the output.
  A value of 0 deactivates the output, any other value will active the output.

  A range for random activation or deactivation delays can be specified.
*/
class LEDTriggeredCycle : public LEDStaticLighting {
  private:
    ///reference to the trigger variable
    const unsigned char & _trigger;

  protected:
    ///time for the next switch in ms
    unsigned long _nextSwitchTimeMs;
    ///minimum activation delay in ms
    const unsigned long _onDelayMinMs;
    ///maximum activation delay in ms
    const unsigned long _onDelayMaxMs;
    ///minimum deactivation delay in ms
    const unsigned long _offDelayMinMs;
    ///maximum deactivation delay in ms
    const unsigned long _offDelayMaxMs;

  public:
    /**
      @brief creates a new LEDTriggeredCycle instance

      A value of 0 on the trigger variable deactivates the output, any other values active the output.

      @param ledPin number of the pin to be used. Arduino defines like LED_BUILTIN are allowed
      @param brightness sets the PWM duty cycle from 0 (off) to 255 (full brightness)
      @param onDelayMinMs minimum activation delay in ms
      @param onDelayMaxMs maximum activation delay in ms
      @param offDelayMinMs minimum deactivation delay in ms
      @param offDelayMaxMs maximum deactivation delay in ms
      @param trigger reference to the trigger variable
      @param onEffect sets the effect class to use when the output is active
      @param offToOnEffect set the effect class to use when the output state transitions from CYCLE_OFF to CYCLE_ON
      @param onToOffEffect set the effect class to use when the output state transitions from CYCLE_ON to CYCLE_OFF
    */
    LEDTriggeredCycle(unsigned char const ledPin, unsigned char const brightness,
                      unsigned long const onDelayMinMs, unsigned long const onDelayMaxMs,
                      unsigned long const offDelayMinMs, unsigned long const offDelayMaxMs, unsigned char & trigger,
                      LEDCyclicEffect * const onEffect = new LEDCyclicEffect(), LEDOneShotEffect * const offToOnEffect = 0, LEDOneShotEffect * const onToOffEffect = 0);

    virtual void execute();
};

/**
  @brief This class can be used to tie room lighting to other rooms

  This class monitors its master lighting cycle to determine whether it should be on or off.
  This is intended to for rooms that are only accessable from other rooms in the building where
  the light should also be turned on.
*/
class LEDChainedCycle : public LEDStaticLighting {
  private:
    ///Master lighting cycle governing this lighting cycle
    LEDStaticLighting const * const _masterCycle;
    ///variable to remember whether this cycle was already active for the current master cycle active state
    bool _outputWasOn;

    ///time for the next switch in ms
    unsigned long _nextSwitchTimeMs;
    ///minimum activation delay in ms
    const unsigned long _onDelayMinMs;
    ///maximum activation delay in ms
    const unsigned long _onDelayMaxMs;
    ///Minimum on (active) time in ms
    unsigned long _onTimeMinMs;
    ///Maximum on (active) time in ms
    unsigned long _onTimeMaxMs;
    ///Minimum off (inactive) time in ms

  public:
    /**
      @brief Creates a new LEDChainedCycle instance
      
      The cycle behaviour can be tuned with the #onDelayMinMs and #onDelayMinMs parameters to delay the start of
      the active cycle.

      The parameters #onTimeMinMs and #onTimeMaxMs determine how long the light should stay active once activated.
      If the master cycle turns inactive this cycle will also go inactive regardless of these parameters.

      @param ledPin number of the pin to be used. Arduino defines like LED_BUILTIN are allowed
      @param brightness sets the PWM duty cycle from 0 (off) to 255 (full brightness)
      @masterCycle Master cycle to enable the active state of this cycle
      @param onDelayMinMs minimum activation delay in ms
      @param onDelayMaxMs maximum activation delay in ms
      @param onTimeMinMs Minimum on (active) time in ms
      @param onTimeMaxMs Maximum on (active) time in ms
      @param onEffect sets the effect class to use when the output is active
      @param offToOnEffect set the effect class to use when the output state transitions from CYCLE_OFF to CYCLE_ON
      @param onToOffEffect set the effect class to use when the output state transitions from CYCLE_ON to CYCLE_OFF
    */
    LEDChainedCycle(const unsigned char ledPin, const unsigned char brightness, LEDStaticLighting const * const  masterCycle,
                    const unsigned long onDelayMinMs, const unsigned long onDelayMaxMs,
                    const unsigned long onTimeMinMs, const unsigned long onTimeMaxMs,
                    LEDCyclicEffect * const onEffect = new LEDCyclicEffect(), LEDOneShotEffect * const offToOnEffect = 0, LEDOneShotEffect * const onToOffEffect = 0);

    virtual void execute();
};


/**
   @brief This class toggles between on and off state based on the minimum and maximum duration for each state.

   The times required for the #_offToOnEffect and #_onToOffEffect eat into the the on time and off time respectively.
   The transition effects will always play out in full, even when they take longer than the total time alotted for the
   on or off state they are assigned to.
*/
class LEDRandomLightingCycle: public LEDStaticLighting {
  protected:
    ///Minimum on (active) time in ms
    unsigned long _onTimeMinMs;
    ///Maximum on (active) time in ms
    unsigned long _onTimeMaxMs;
    ///Minimum off (inactive) time in ms
    unsigned long _offTimeMinMs;
    ///Maximum off (inactive) time in ms
    unsigned long _offTimeMaxMs;
    ///Time for the next transition from off to on or vice versa
    unsigned long _timeOfNextSwitchMs;

  public:
    /**
      @brief Creates a new LEDRandomLightingCycle object.

      @param ledPin number of the pin to be used. Arduino defines like LED_BUILTIN are allowed
      @param brightness sets the PWM duty cycle from 0 (off) to 255 (full brightness)
      @param onTimeMinMs Minimum on (active) time in ms
      @param onTimeMaxMs Maximum on (active) time in ms
      @param offTimeMinMs Minimum off (inactive) time in ms
      @param offTimeMaxMs Maximum off (inactive) time in ms
      @param onEffect sets the effect class to use when the output is active
      @param offToOnEffect set the effect class to use when the output state transitions from CYCLE_OFF to CYCLE_ON
      @param onToOffEffect set the effect class to use when the output state transitions from CYCLE_ON to CYCLE_OFF
    */
    LEDRandomLightingCycle(unsigned char const ledPin, unsigned char const brightness,
                           unsigned long const onTimeMinMs, unsigned long const onTimeMaxMs,
                           unsigned long const offTimeMinMs, unsigned long const offTimeMaxMs,
                           LEDCyclicEffect * const onEffect = new LEDCyclicEffect(),
                           LEDOneShotEffect * const offToOnEffect = 0,
                           LEDOneShotEffect * const onToOffEffect = 0);

    /**
      @brief Executes the output cycle code.
    */
    void execute();
};

/**
  @brief This class cycles between the on and off state with a fixed timing for each state.
*/
class LEDLightingCycle: public LEDRandomLightingCycle {
  public:
    /**
      @brief Creates a new LEDLightingCycle object.

      @param ledPin number of the pin to be used. Arduino defines like LED_BUILTIN are allowed
      @param brightness sets the PWM duty cycle from 0 (off) to 255 (full brightness)
      @param onTimeMs on (active) time in ms
      @param offTimeMs off (inactive) time in ms
      @param onEffect sets the effect class to use when the output is active
      @param offToOnEffect set the effect class to use when the output state transitions from CYCLE_OFF to CYCLE_ON
      @param onToOffEffect set the effect class to use when the output state transitions from CYCLE_ON to CYCLE_OFF
    */
    LEDLightingCycle(unsigned char const ledPin, unsigned char const brightness,
                     unsigned long const onTimeMs, unsigned long const offTimeMs,
                     LEDCyclicEffect * const onEffect = new LEDCyclicEffect(),
                     LEDOneShotEffect * const offToOnEffect = 0,
                     LEDOneShotEffect * const onToOffEffect = 0);
};
#endif
