#ifndef HTTP_H
#define HTTP_H

#include <ESPAsyncWebServer.h>
#include "motor_control.h"
#include "light.h"
#include "song.h"
#include <LITTLEFS.h>

using namespace std;

#pragma pack(1)
typedef bool(*updateWifiHandler)(const char*, const char*, const char*);
typedef void(*songLoading)();
typedef void(*songUpdatedHandler)();
typedef void(*playNote)(uint32_t);
typedef SongStatus(*getSongStatus)();
typedef void(*startSong)();
typedef void(*stopSong)();
typedef void(*setLoop)(bool);

#pragma pack(1)
typedef struct ControlChange {
  uint32_t period;
} ControlChange;

class HTTP {
  AsyncWebServer *server;
  AsyncWebSocket *ws;
  File song_file;
  LIGHT *light;

public:
  HTTP(int port);
  void setupWebServer(LIGHT *light, 
    updateWifiHandler wifiHandler, 
    songLoading songLoading,
    songUpdatedHandler songUpdated, 
    playNote playNote,
    getSongStatus getSongStatus,
    startSong startSong,
    stopSong stopSong,
    setLoop setLoop
  );
};

#endif