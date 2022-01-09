#include <Arduino.h>
#include <WiFi.h>
#include <FS.h>
#include <LITTLEFS.h>
#include <ESPmDNS.h>
#include "passwords.h"
#include "wifi_connect.h"

#define WIFI_DEFAULT_START_AP

// constructor
WIFI::WIFI(const char *configPath) {
  configFile = configPath;
  WiFi.setSleep(false);
}

// public methods
bool WIFI::startWIFI() {
  WIFIConfig config;
  if (readConfig(&config) && connectToAccessPoint(&config)) {
    Serial.print("Connected with IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  Serial.println("Connection to AP failed");
  return connectDefault();
}

bool WIFI::writeWIFIConfig(const char* ssid, const char* password, const char* hostname) {
  Serial.println("writing wifi config");
  WIFIConfig config;
  strlcpy(config.ssid, ssid, 30);
  strlcpy(config.password, password, 30);
  strlcpy(config.hostname, hostname, 30);

  File f = LITTLEFS.open(configFile, "w");
  if (!f) {
    Serial.printf("Failed to open %s for writing.", configFile);
    return false;
  }

  f.write((uint8_t *) &config, sizeof(WIFIConfig));
  f.close();
  Serial.println("wrote wifi config");
  return true;
}

bool WIFI::isOnline() {
  return online;
}

bool WIFI::isAccessPoint() {
  return accessPoint;
}

// private methods
bool WIFI::readConfig(WIFIConfig *config) {
  if (!LITTLEFS.exists(configFile)) {
    return false;
  }
  Serial.println("Config file found");

  File f = LITTLEFS.open(configFile, "r");
  if (!f) {
    Serial.println("Couldn't open config file");
    return false;
  }

  f.readBytes((char *) config, sizeof(WIFIConfig));
  f.close();
  Serial.println("Config file read");

  return true;
}

bool WIFI::connectToAccessPoint(WIFIConfig *config) {
  Serial.printf("Connecting to access point %s.\n", config->ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(config->ssid, config->password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed!...");
    return false;
  }
  Serial.printf("Connected.\n");

  if (!MDNS.begin(config->hostname)) {
    Serial.println("Could not set up mdns.");
  }
  Serial.printf("MDNS set to %s.local.\n", config->hostname);

  online = true;
  return true;
}

bool WIFI::connectDefault() {
  #ifdef WIFI_CONNECT_STASSID
    Serial.printf("Connecting to access point %s.\n", WIFI_CONNECT_STASSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_CONNECT_STASSID, WIFI_CONNECT_STAPSK);
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      Serial.print("Connected at IP: ");
      Serial.println(WiFi.localIP());

      if (!MDNS.begin(WIFI_DEFAULT_MDNS)) {
        Serial.println("Could not set up mdns.");
      }
      online = true;
      return true;
    }
    Serial.printf("Could not connect to access point %s.\n", WIFI_CONNECT_STASSID);
  #endif

  Serial.printf("Starting access point at %s.\n", WIFI_START_STASSID);
  WiFi.softAPConfig(IPAddress(1,2,3,4), IPAddress(1,2,3,4), IPAddress(255,255,255,0));
  if (!WiFi.softAP(WIFI_START_STASSID, WIFI_START_STAPSK)) {
    return false;
  }

  accessPoint = true;

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  online = true;
  return true;
}

IPAddress WIFI::IP() {
  return accessPoint ? WiFi.softAPIP() : WiFi.localIP();
}
