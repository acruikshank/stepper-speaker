#include <Arduino.h>
#include <FS.h>
#include <LITTLEFS.h>
#include "config.h"
#include "http.h"
#include "song.h"
#include "light.h"
#include "motor_control.h"
#include "wifi_connect.h"
#include "ota.h"

/** TODO
 * Listen for config mode
 * Repeat song
 */

#define SWITCH_IN_PIN 25
#define SWITCH_POWER_PIN 26

bool restartNow = false;

CONFIG config("/config");
WIFI wifi("/wifi_config");
HTTP webserver(80);
MOTOR motor;
SONG song;
LIGHT light;

bool updateWifi(const char* ssid, const char* psk, const char* hostname) {
  Serial.println("Calling writeWIFIConfig");
  if (wifi.writeWIFIConfig(ssid, psk, hostname)) {
    restartNow = true;
    return true;
  }
  return false;
}

void songUpdated() {
  song.readSong();
}

void songLoadingHandler() {
  song.stop();
  song.loaded = false;
}

void playNoteHandler(uint32_t period) {
  if (period) {
    light.stop();
    light.setHSV(255.0*fmod(log2((double) period), 1.0), 255, 150);
    motor.setDirection(!motor.direction);
    motor.play(period);
  } else {
    light.blink(light.currentColor);
    motor.stop();
  }
}

SongStatus getSongStatusHandler() {
  return song.getStatus();
}

void startSongHandler() {
  song.start();
}

void stopSongHandler() {
  song.stop();
}

void setLoopHandler(bool loop) {
  song.setLoop(loop);
}

void setup() {
  Serial.begin(115200);

  pinMode(SWITCH_POWER_PIN, OUTPUT);
  digitalWrite(SWITCH_POWER_PIN, HIGH);
  pinMode(SWITCH_IN_PIN, INPUT_PULLDOWN);

  light.init();
  light.blink(orange);

  Serial.println("Booting");

  if(!LITTLEFS.begin(true)){
    Serial.println("LittleFS Mount Failed");
    return;
  }

  Serial.println("Start WIFI");

  if (!wifi.startWIFI()) {
    light.blink(red);
    Serial.println("Could not setup WIFI");
  } else {
    light.blink(wifi.isAccessPoint() ? green : white);
  }


  Serial.println("Start OTA");

  setupOTA();

  Serial.println("Start Web");

  webserver.setupWebServer(&light, 
    &updateWifi, 
    &songLoadingHandler,
    &songUpdated,
    &playNoteHandler, 
    &getSongStatusHandler, 
    &startSongHandler, 
    &stopSongHandler, 
    &setLoopHandler
  );

  motor.init();
  song.init(&motor, &light);
}

void loop() {
  handleOTA();

  if (restartNow)
    ESP.restart();
  
  if (digitalRead(SWITCH_IN_PIN)) {
    light.displayIP(wifi.IP().toString());
  }

  delay(500);
}