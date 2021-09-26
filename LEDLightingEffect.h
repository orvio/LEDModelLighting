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
    //Start delay for the current execution of the effect in ms
    unsigned short _startDelayMs;
    //Start time of the current execution in ms
    unsigned long _startMs;

  protected:
    /**
      @brief Returns the remaining duration of the current execution of the effect.

      This method uses \p currentTimeMs as the current absolute time of the Arduino as returned by millis()

      @param currentTimeMs current absolute time in ms
      @params remaining duration of the effect in ms
    */
    virtual unsigned short getRemainingDuration(const unsigned long currentTimeMs);

    /**
      @brief Returns the remaining start delay of the current execution of the effect.

      This method uses \p currentTimeMs as the current absolute time of the Arduino as returned by millis()

      @param currentTimeMs current absolute time in ms
      @params remaining start delay of the effect in ms
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
    */
    unsigned short getDurationMs();

    /**
      @brief returns 1 if the effect has finished.

      This method uses #getDurationMs() to determine if there is time left on this effect.
      @return 1 if the effect has finished, 0 if time is remaining
    */
    char isFinished();

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
#define FADE_IN 0
#define FADE_OUT 1
  private:
    unsigned char const _fadeDirection;

  public:
    FadeEffect(unsigned short const durationMs, unsigned char const fadeDirection, const unsigned short maxStartDelayMs = 0);
    unsigned char getBrightness( unsigned char const maxBrightness);
};

/**
  @brief Effect cass for transitions emulating a fluorescent light starting up
*/
class FluorescentStartEffect : public LEDOneShotEffect {
#define START_UNINITIALIZED 0
#define START_OFF 1
#define START_FLICKER 2
#define START_FLOAT 3
#define START_ON 4

#define START_OFF_MIN_DURATION_MS 100
#define START_OFF_MAX_DURATION_MS 1000
#define START_FLICKER_MIN_DURATION_MS 10
#define START_FLICKER_MAX_DURATION_MS 100
#define START_FLOAT_MIN_DURATION_MS 500
#define START_FLOAT_MAX_DURATION_MS 3000
  private:
    unsigned long _currentStageStartTimeMs;
    unsigned short _currentStageDurationMs;
    unsigned char _currentStage;
    unsigned short _currentDurationMs;
    const unsigned short _minDurationMs;

    unsigned char getNextStage(const unsigned long currentTimeMs);
    void setupNextStage(const unsigned long currentTimeMs);

  protected:
    unsigned short getRemainingDuration(const unsigned long currentTimeMs);
  public:
    FluorescentStartEffect(unsigned short const minDurationMs, unsigned short const durationMs, const unsigned short maxStartDelayMs = 0);
    unsigned char getBrightness( unsigned char const maxBrightness);
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
    unsigned int _cycleTimeMs; //16 bit for 65s max cycle time

  public:
    BeaconEffect(unsigned int const cycleTimeMs);
    unsigned char getBrightness( unsigned char const maxBrightness);
};

#endif
