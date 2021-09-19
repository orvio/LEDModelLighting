# LEDModelLighting
A small library to control LEDs lighting effects in building models on model railroads an similar purposes.

## Installation
Simple copy all the .cpp and .h files into a subdirectory of your Arduino library folder.
On windows that folder is usually located under Documents/Arduino/libraries.

After copying the files into your library folder you need to restart the Arduino IDE.

## Usage
After installing the library files simply put
```#include <LEDLightingCycle.h>```
at the start of you sketch to be able to use the library functions.

The easiest way to get started is to create a new sketch and replace the entire content of the sketch with this code:
```
#include <LEDLightingCycle.h>

//define number of LEDs
#define LED_COUNT 7

//LED setup
LEDStaticLighting * ledSetups[LED_COUNT];

void setup() {
  ledSetups[0] = new LEDRandomLightingCycle(LED_BUILTIN, 255, 500, 1000, 1000, 2000, new LEDCyclicEffect(), new FluorescentStartEffect(100, 500), new FadeEffect(100, FADE_OUT));
}

void loop() {
  // put your main code here, to run repeatedly:
  for (unsigned char ledIndex = 0; ledIndex < LED_COUNT; ledIndex++) {
    ledSetups[ledIndex]->execute();
  }
}
```

This will control the on board LED of your Arduino, so you don't need any additional hardware. This code will cycle between on and off with a random duration for the on and off state.
The LED will turn on for a time between 500ms ad 1000ms and turn off for a time between 1000 ad 2000 ms.
When the light is activated a fluorescent startup flicker simulation executes for a time between 100 and 500ms.
Then the light is deactivated it will fade from bright to dark within 100ms.
