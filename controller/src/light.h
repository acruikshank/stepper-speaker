#include <Arduino.h>

#ifndef LIGHT_H
#define LIGHT_H

#include "FastLED.h"
#include "Wire.h"

#define NUM_LEDS 1
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define LED_SIGNAL_PIN 27
#define COLOR_ORDER GRB

const CHSV white = CHSV(70,41,209);
const CHSV red = CHSV(0,175,200);
const CHSV orange = CHSV(24,255,210);
const CHSV yellow = CHSV(55,255,210);
const CHSV green = CHSV(91,200,160);
const CHSV blue = CHSV(145,255,210);
const CHSV purple = CHSV(184,255,180);

enum states { SOLID, BLINKING };

using namespace std;

// types
class LIGHT {
  TaskHandle_t lightTask;
  bool lightTaskInitialized = false;

public:
  CRGB leds[NUM_LEDS];
  CHSV currentColor;
  String currentIPAddress;
  LIGHT();
  void init();
  void setHSV(uint8_t hue, uint8_t sat, uint8_t bright);
  void blink(CHSV color);
  void displayIP(String ip);
  void stop();
};

#endif

