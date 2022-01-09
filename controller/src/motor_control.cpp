#include <Arduino.h>
#include "motor_control.h"

#define INTERRUPT_ATTR ICACHE_RAM_ATTR

#define ENABLE_PIN 13
#define DIR_PIN 12
#define STEP_PIN 14

extern void IRAM_ATTR __digitalWrite(uint8_t pin, uint8_t val) {
  if (val) {
     if(pin < 32) 
     {
        GPIO.out_w1ts = ((uint32_t)1 << pin);
     } 
     else if(pin < 34) {
        GPIO.out1_w1ts.val = ((uint32_t)1 << (pin - 32));
     }
  }  else {
     if(pin < 32) 
     {
        GPIO.out_w1tc = ((uint32_t)1 << pin);
     } 
     else if(pin < 34) 
     {
        GPIO.out1_w1tc.val = ((uint32_t)1 << (pin - 32));
     }
  }
}

bool dir;
bool step;

IRAM_ATTR void update() {
  __digitalWrite(STEP_PIN, step);
  step = !step;
}

// constructor
MOTOR::MOTOR() {
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, HIGH);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
}

void MOTOR::debug() {
  Serial.printf("motor timer %d %d %d\n", &timer, timer, timer_set);
}

void MOTOR::setDirection(uint8_t dir) {
  __digitalWrite(DIR_PIN, dir);
  direction = dir;  
}

void MOTOR::init() {
  timer = timerBegin(2  , 80, true);
  timerAttachInterrupt(timer, &update, true);
  timer_set = true;
}

void MOTOR::play(uint32_t period) {
  digitalWrite(ENABLE_PIN, LOW);
  if (period) {
    timerAlarmWrite(timer, period, true);
   if (!timerAlarmEnabled(timer)) timerAlarmEnable(timer);
  } else if (timerAlarmEnabled(timer)) {
    timerAlarmDisable(timer);
  }
}

void MOTOR::stop() {
  digitalWrite(ENABLE_PIN, HIGH);
  if (timerAlarmEnabled(timer))
    timerAlarmDisable(timer);
}