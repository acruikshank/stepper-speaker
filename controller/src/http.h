#ifndef HTTP_H
#define HTTP_H

#include <ESPAsyncWebServer.h>
#include "motor_control.h"
#include "light.h"
#include <LITTLEFS.h>

using namespace std;

#pragma pack(1)
typedef bool(*updateWifiHandler)(const char*, const char*, const char*);
typedef void(*songUpdatedHandler)();

class HTTP {
  AsyncWebServer *server;
  AsyncWebSocket *ws;
  File song_file;
  LIGHT *light;

public:
  HTTP(int port);
  void setupWebServer(LIGHT *light, updateWifiHandler wifiHandler, songUpdatedHandler songUpdated);
};

#endif