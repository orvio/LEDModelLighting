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
#ifndef LEDLIGHTINGEFFECT_H
#define LEDLIGHTINGEFFECT_H

/**
   @brief Base class for all lighting effects
*/
class LEDLightingEffect {
  public:
    /**
      @brief returns the current brightness for the output

      Computes the current brightness of the output based on \p maxBrightness.

      @param maxBrightness max allowed brightness for the output
      @return current output brightness
    */
    virtual unsigned char getBrightness( unsigned char const maxBrightness);
};

/**
   @brief Base class for on/off transitions
*/
class LEDOneShotEffect : public LEDLightingEffect {
  protected:
    ///Duration of the effect in ms
    const unsigned short _durationMs;
    ///Maximum start delay in ms
    const unsigned short _maxStartDelayMs;
    ///Start delay for the current execution of the effect in ms
    unsigned short _startDelayMs;
    ///Start time of the current execution in ms
    unsigned long _startMs;

  protected:
    /**
      @brief Returns the remaining duration of the current execution of the effect.

      This method uses \p currentTimeMs as the current absolute time of the Arduino as returned by millis()

      @param currentTimeMs current absolute time in ms
      @return remaining duration of the effect in ms
    */
    virtual unsigned short getRemainingDuration(const unsigned long currentTimeMs);

    /**
      @brief Returns the remaining start delay of the current execution of the effect.

      This method uses \p currentTimeMs as the current absolute time of the Arduino as returned by millis()

      @param currentTimeMs current absolute time in ms
      @return remaining start delay of the effect in ms
    */
    virtual unsigned short getRemainingStartDelay(const unsigned long currentTimeMs);

  public:
    /**
      @brief Resets the effect for the next execution.

      The default implementation sets #_startMs to now and #_startDelayMs to a random value between 0 and #_maxStartDelayMs
    */
    virtual void reset();

    /**
      @brief returns the value of #_durationMs.

      This method does not take #_startDelayMs into account.
      @return remaining effect duration in ms
    */
    unsigned short getDurationMs();

    /**
      @brief returns 1 if the effect has finished.

      This method uses #getDurationMs() to determine if there is time left on this effect.
      @return true if the effect has finished, false if time is remaining
    */
    bool isFinished();

    /**
      @brief creates a new LEDOneShotEffect object

      @param durationMs duration of the effect in ms
      @param maxStartDelayMs maximum start delay of the effect in ms
    */
    LEDOneShotEffect(const unsigned short durationMs, const unsigned short maxStartDelayMs = 0);
};

/**
  @brief Class for fade in/fade out transitions
*/
class FadeEffect : public LEDOneShotEffect {
  public:
    ///Enumeration for the fade directions
    enum FadeDirections {
      ///Fades from dark to full brightnes
      FADE_IN,
      ///Fades from full brightnes to dark
      FADE_OUT
    };

    /**
      @brief creates a new FadeEffect instance

      @param durationMs fade duration in ms
      @param fadeDirection direction of the fade effect, either FADE_IN or FADE_OUT
      @param maxStartDelayMs maximum possible start delay in ms
    */
    FadeEffect(unsigned short const durationMs, const FadeDirections fadeDirection, const unsigned short maxStartDelayMs = 0);

    unsigned char getBrightness( unsigned char const maxBrightness);
  private:
    ///direction of the fade effect, either FADE_IN or FADE_OUT
    const FadeDirections _fadeDirection;
};

/**
  @brief Effect cass for transitions emulating a fluorescent light starting up
*/
class FluorescentStartEffect : public LEDOneShotEffect {
  private:
    ///Enumeration for the internal effect states
    enum EffectStages {
      ///Initial stage to randomly decide the start stage
      START_UNINITIALIZED,
      ///The light is off for a short period of time
      START_OFF,
      ///The light is on at full brightness for a short period of time
      START_FLICKER,
      ///The light floats at about half brightness
      START_FLOAT,
      ///The light is on at full brightness, no other stages are selected after this stage
      START_ON
    };

    ///start time of the current stage in ms
    unsigned long _currentStageStartTimeMs;
    /**
      @brief duration of the current stage in ms

      The duration of the current stage extend beyond the total duration of the effect. The current stage will be cut short in this case.
    */
    unsigned short _currentStageDurationMs;
    ///type of the current effect stage
    EffectStages _currentStage;
    ///duration of the current execution cycle of the effect in ms
    unsigned short _currentDurationMs;
    ///minimum duration of the effect in ms
    const unsigned short _minDurationMs;

    /**
      @brief returns the next stage of the effects

      The permissible stages can vary depending on the amount of the effect has already been running.
      The next stage is determined randomly out of the pool of permissible stages.

      @param currentTimeMs current time in ms as returned by millis()
      @return the next stage to be executed
    */
    EffectStages getNextStage(const unsigned long currentTimeMs);

    /**
      @brief prepares the next stage execution
    */
    void setupNextStage(const unsigned long currentTimeMs);

  protected:
    /**
      @brief returns the remaining duration of the effect

      This effect does not always use up the entire time alotted in the constructor.

      @param currentTimeMs current time in ms as returned by millis()
      @return the remaining execution time in ms
    */
    unsigned short getRemainingDuration(const unsigned long currentTimeMs);

  public:
    /**
      @brief creates a new FluorescentStartEffect instance

      This effect executes a random startup flicker. The duration of the flicker can further be influenced by \p minDurationMs
      to create startup flickers of a random duration each time the effect executes.

      @param minDurationMs minimum effect duration in ms
      @param maxDurationMs maximum effect duration in ms
      @param maxStartDelayMs maximum possible start delay in ms
    */
    FluorescentStartEffect(unsigned short const minDurationMs, unsigned short const maxDurationMs, const unsigned short maxStartDelayMs = 0);
    unsigned char getBrightness( unsigned char const maxBrightness);

    /**
      @brief resets the effect for the next effect execution cycle

      The duration of the following cycle is also determined in this function call.
    */
    void reset();
};

/**
  @brief Base class for permanent light effects
*/
class LEDCyclicEffect : public LEDLightingEffect {

};

/**
   @brief Cyclic effect class for emulating a rotary beacon
*/
class BeaconEffect : public LEDCyclicEffect {
  private:
    ///cycle time for one beacon rotation in ms
    unsigned int _cycleTimeMs; //16 bit for 65s max cycle time

  public:
    /**
      @brief create a bew BeaconEffect instance

      @param cycleTimeMs beacon cycle time in ms
    */
    BeaconEffect(unsigned int const cycleTimeMs);

    /**
      @brief returns the current brightness for the output

      Computes the current brightness of the output based on \p maxBrightness and the current position in the beacon cycle.
      @param maxBrightness max allowed brightness for the output
      @return current output brightness
    */
    unsigned char getBrightness( unsigned char const maxBrightness);
};

#endif
