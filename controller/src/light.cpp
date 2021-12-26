#include "light.h"

// constructor
LIGHT::LIGHT() {
  lightTaskInitialized = false;
}

void LIGHT::init() {
  FastLED.addLeds<WS2811, LED_SIGNAL_PIN, COLOR_ORDER>(leds, NUM_LEDS); 
}

void LIGHT::setHSV(uint8_t hue, uint8_t sat, uint8_t bright) {
  leds[0] = CHSV(hue, sat, bright);
  FastLED.show();
}

void runBlink(void *pvParameters) {
  LIGHT *light = (LIGHT *) pvParameters;
  uint8_t theta = 0;
  
  while (true) {
    light->leds[0] = CHSV(
      light->currentColor.h,
      light->currentColor.s, 
      map(80, light->currentColor.v, 0, 255, sin8(theta))
    );
    FastLED.show();
    theta += 1;

    vTaskDelay(6);
  }
}

void LIGHT::blink(CHSV color) {
  stop();
  currentColor = color;
  xTaskCreate(runBlink, "BlinkTask", 5000, (void *) this, 1, &lightTask);
  lightTaskInitialized = true;
}

void shortFlash(LIGHT *light, CRGB color) {
  light->leds[0] = color;
  FastLED.show();
  vTaskDelay(250);
  light->leds[0] = CRGB(0,0,0);
  FastLED.show();
  vTaskDelay(250);
}

void doubleFlash(LIGHT *light, CRGB color) {
  shortFlash(light, color);
  shortFlash(light, color);
  vTaskDelay(500);
}

void longFlash(LIGHT *light, CRGB color) {
  light->leds[0] = color;
  FastLED.show();
  vTaskDelay(1000);
  light->leds[0] = CRGB(0,0,0);
  FastLED.show();
  vTaskDelay(500);
}

void runDisplayIP(void *pvParameters) {
  LIGHT *light = (LIGHT *) pvParameters;

  for (auto &ch : light->currentIPAddress) {
    switch (ch) {
      case '.': shortFlash(light, white); vTaskDelay(500); break;
      case '0': doubleFlash(light, red); break;
      case '1': longFlash(light, red); break;
      case '2': doubleFlash(light, orange); break;
      case '3': longFlash(light, orange); break;
      case '4': doubleFlash(light, yellow); break;
      case '5': longFlash(light, yellow); break;
      case '6': doubleFlash(light, green); break;
      case '7': longFlash(light, green); break;
      case '8': doubleFlash(light, blue); break;
      case '9': longFlash(light, blue); break;
    }    
  }
  runBlink(pvParameters);
}

void LIGHT::displayIP(String ip) {
  stop();
  currentIPAddress = ip;
  xTaskCreate(runDisplayIP, "DisplayIPTask", 5000, (void *) this, 1, &lightTask);
  lightTaskInitialized = true;
}

void LIGHT::stop() {
  if (lightTaskInitialized) {
    vTaskDelete(lightTask);
    lightTaskInitialized = false;
  }
}
