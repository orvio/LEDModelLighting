#include <LEDLightingCycle.h>

//define PWM capable pins
#define PWM_PIN0 3
#define PWM_PIN1 5
#define PWM_PIN2 6
#define PWM_PIN3 9
#define PWM_PIN4 10
#define PWM_PIN5 11
#define PWM_PIN6 LED_BUILTIN //13 on micro

//define number of LEDs
#define LED_COUNT 6

//define voltage sensing pins
#define VREF 5.0
#define V5_SENSE_PIN A5
#define V5_SENSE_FACTOR 2.0 //factor for even devider, e.g. 2x10kOhm between 5V and ground
#define VIN_SENSE_PIN A4
#define VIN_SENSE_FACTOR 3.2 //factor for voltage devider with 22kOhm to Vin and 10kOhm to ground

//define voltage status LED pins
#define V5_LOW_PIN A2
#define V5_OK_PIN A3
#define VIN_LOW_PIN 7
#define VIN_OK_PIN 8
#define VIN_HIGH_PIN 12

//define voltage limits
#define V5_LOW_VOLTAGE 4.8
#define VIN_LOW_VOLTAGE 6.5
#define VIN_HIGH_VOLTAGE 8.0

//LED setup
LEDStaticLighting * ledSetups[LED_COUNT];

void setup() {
  // put your setup code here, to run once:
  randomSeed(analogRead(A0)*analogRead(A1)*analogRead(A2));

  //outside lights
  ledSetups[0] = new LEDStaticLighting(PWM_PIN0, 255);
  ledSetups[1] = new LEDStaticLighting(PWM_PIN1, 255);
  ledSetups[2] = new LEDStaticLighting(PWM_PIN2, 255);
  
  //office lights
  ledSetups[3] = new LEDRandomLightingCycle(PWM_PIN3, 255, 5*60*1000ul, 10*60*1000ul, 5*60*1000ul, 10*60*1000ul, new LEDCyclicEffect(), new FluorescentStartEffect(500, 4000), new FadeEffect(50, FadeEffect::FADE_OUT));
  ledSetups[4] = new LEDRandomLightingCycle(PWM_PIN4, 255, 5*60*1000ul, 10*60*1000ul, 5*60*1000ul, 10*60*1000ul, new LEDCyclicEffect(), new FluorescentStartEffect(1000, 4000), new FadeEffect(50, FadeEffect::FADE_OUT));
  ledSetups[5] = new LEDChainedCycle(PWM_PIN5, 255, ledSetups[4], 30*1000ul, 2*60*1000ul, 2*60*1000ul, 10*60*1000ul, new LEDCyclicEffect(), new FluorescentStartEffect(500, 2000), new FadeEffect(50, FadeEffect::FADE_OUT) );

  pinMode(V5_LOW_PIN, OUTPUT);
  pinMode(V5_OK_PIN, OUTPUT);
  pinMode(VIN_LOW_PIN, OUTPUT);
  pinMode(VIN_OK_PIN, OUTPUT);
  pinMode(VIN_HIGH_PIN, OUTPUT);
  analogReference(EXTERNAL);
}

void loop() {
  // put your main code here, to run repeatedly:

  for (unsigned char ledIndex = 0; ledIndex < LED_COUNT; ledIndex++) {
    ledSetups[ledIndex]->execute();
  }

  //read input voltages
  const float voltageV5 = (analogRead(V5_SENSE_PIN) * V5_SENSE_FACTOR * VREF) / 1024.0;
  const float voltageVin = (analogRead(VIN_SENSE_PIN) * VIN_SENSE_FACTOR * VREF ) / 1024.0;

  if (voltageV5 < V5_LOW_VOLTAGE) { //V5 low
    digitalWrite(V5_LOW_PIN, HIGH);
    digitalWrite(V5_OK_PIN, LOW);
  }
  else { //V5 OK
    digitalWrite(V5_LOW_PIN, LOW);
    digitalWrite(V5_OK_PIN, HIGH);
  }

  if (voltageVin < VIN_LOW_VOLTAGE) { //Vin low
    digitalWrite(VIN_LOW_PIN, HIGH);
    digitalWrite(VIN_OK_PIN, LOW);
    digitalWrite(VIN_HIGH_PIN, LOW);
  }
  else if (voltageVin > VIN_HIGH_VOLTAGE) { //Vin high
    digitalWrite(VIN_LOW_PIN, LOW);
    digitalWrite(VIN_OK_PIN, LOW);
    digitalWrite(VIN_HIGH_PIN, HIGH);
  }
  else { //Vin OK
    digitalWrite(VIN_LOW_PIN, LOW);
    digitalWrite(VIN_OK_PIN, HIGH);
    digitalWrite(VIN_HIGH_PIN, LOW);
  }
}
