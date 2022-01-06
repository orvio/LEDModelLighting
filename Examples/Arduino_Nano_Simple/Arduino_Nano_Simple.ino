#include <LEDLightingCycle.h>

//define PWM capable pins
#define PWM_PIN0 3
#define PWM_PIN1 5
#define PWM_PIN2 6
#define PWM_PIN3 9
#define PWM_PIN4 10

//define number of LEDs
#define LED_COUNT 3

//LED setup
LEDStaticLighting * ledSetups[LED_COUNT];

void setup() {
  // put your setup code here, to run once:
  randomSeed(analogRead(A0)*analogRead(A1)*analogRead(A2));

  //outside lights
  ledSetups[0] = new LEDRandomLightingCycle(PWM_PIN0, 255, 5*60*1000ul, 10*60*1000ul, 5*60*1000ul, 10*60*1000ul, new LEDCyclicEffect(), new FadeEffect(500, FadeEffect::FADE_IN), new FadeEffect(500, FadeEffect::FADE_OUT));
  ledSetups[1] = new LEDChainedCycle(PWM_PIN1, 255, ledSetups[0], 30*1000ul, 2*60*1000ul, 2*60*1000ul, 10*60*1000ul, new LEDCyclicEffect(), new FadeEffect(500, FadeEffect::FADE_IN), new FadeEffect(500, FadeEffect::FADE_OUT));
  ledSetups[2] = new LEDChainedCycle(PWM_PIN2, 255, ledSetups[0], 30*1000ul, 2*60*1000ul, 2*60*1000ul, 10*60*1000ul, new LEDCyclicEffect(), new FadeEffect(500, FadeEffect::FADE_IN), new FadeEffect(500, FadeEffect::FADE_OUT));
}

void loop() {
  // put your main code here, to run repeatedly:

  for (unsigned char ledIndex = 0; ledIndex < LED_COUNT; ledIndex++) {
    ledSetups[ledIndex]->execute();
  }
}
