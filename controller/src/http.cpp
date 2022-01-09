#include "http.h"

HTTP::HTTP(int port) {
  server = new AsyncWebServer(port);
  ws = new AsyncWebSocket("/ws");
}

const char* PARAM_PAGE = "page";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void HTTP::setupWebServer(LIGHT *l, 
    updateWifiHandler updateWifi, 
    songLoading songLoading,
    songUpdatedHandler songUpdated,
    playNote playNote,
    getSongStatus getSongStatus,
    startSong startSong,
    stopSong stopSong,
    setLoop setLoop    
) {
  light = l;

  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {    
    request->send(LITTLEFS, "/index.html", "text/html");
  });

  server->on("/midi.js", HTTP_GET, [](AsyncWebServerRequest *request) {    
    request->send(LITTLEFS, "/midi.js", "text/javascript");
  });

  server->on("/live", HTTP_GET, [](AsyncWebServerRequest *request) {    
    request->send(LITTLEFS, "/live.html", "text/html");
  });

  ///////////////////
  //  SONG CONTROL
  ///////////////////

  server->on("/song/upload", HTTP_POST, [](AsyncWebServerRequest *request) {      
    request->send(200, "text/plain; charset=utf-8", "OK");
  }, [this, songLoading, songUpdated](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      Serial.println("start writing song");
      songLoading();
      song_file = LITTLEFS.open("/SONG", "w");
    }
    Serial.printf("write %d bytes\n", len);
    for (size_t i = 0; i < len; i++) {
      song_file.write(data[i]);
    }
    if (final) {
      song_file.flush();
      song_file.close();
      Serial.printf("end writing song. %d bytes written\n", index+len);

      if (songUpdated) {
        songUpdated();
      }
    }
  });

  server->on("/song/start", HTTP_POST, [startSong](AsyncWebServerRequest *request){
    startSong();
    request->send(200);
  });

  server->on("/song/stop", HTTP_POST, [stopSong](AsyncWebServerRequest *request){
    stopSong();
    request->send(200);
  });

  server->on("/song/loop", HTTP_POST, [setLoop](AsyncWebServerRequest *request){
    if (request->hasParam("loop", true)) {
      Serial.println("setting loop to true");
      setLoop(true);
    } else {
      Serial.println("setting loop to false");
      setLoop(false);
    }
    request->send(200);
  });

  server->on("/song/status", HTTP_GET, [getSongStatus](AsyncWebServerRequest *request){
    SongStatus status = getSongStatus();
    std::string buf("{\"loaded\":");
    buf.append(status.songLoaded ? "true" : "false");
    buf.append(",\"playing\":");
    buf.append(status.songPlaying ? "true" : "false");
    buf.append(",\"looping\":");
    buf.append(status.songLooping ? "true" : "false");
    buf.append("}");
    request->send(200, "application/json", buf.c_str());
  });

  //////////////////////////
  //       WS CONTROL
  //////////////////////////

  ws->onEvent([playNote](AsyncWebSocket *server, AsyncWebSocketClient *c, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch(type) {
    case WS_EVT_CONNECT:
      Serial.printf("ws[%s][%u] connect\n", server->url(), c->id());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("ws[%s][%u] disconnect\n", server->url(), c->id());
      break;
    case WS_EVT_PONG:
      Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), c->id(), len, (len)?(char*)data:"");
      break;
    case WS_EVT_ERROR:
      Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), c->id(), *((uint16_t*)arg), (char*)data);
      break;
    case WS_EVT_DATA:
      // Serial.printf("ws[%s][%u] data[%d]: explen:%d id:%d\n", server->url(), c->id(), len, sizeof(ControlChange), data[0]);
      if (len == sizeof(ControlChange)) {
        playNote(((ControlChange*) data)->period);
        c->text("");
      }
      break;
    }
  });

  ///////////////////
  //     LIGHT
  ///////////////////

  server->on("/light_test", HTTP_POST, [updateWifi, this](AsyncWebServerRequest *request){
    Serial.println("Processing light request");
    uint8_t hue = 0;
    uint8_t sat = 0;
    uint8_t bri = 0;

    if (request->hasParam("hue", true)) {
      hue = atoi(request->getParam("hue", true)->value().c_str());
    }
    if (request->hasParam("sat", true)) {
      sat = atoi(request->getParam("sat", true)->value().c_str());
    }
    if (request->hasParam("bri", true)) {
      bri = atoi(request->getParam("bri", true)->value().c_str());
    }

    Serial.printf("updating light %d, %d, %d\n", hue, sat, bri);
    light->setHSV(hue, sat, bri);
    request->send(200);
  });


  ///////////////////
  //     WIFI
  ///////////////////

  server->on("/wifi_config", HTTP_GET, [](AsyncWebServerRequest *request) {    
        request->send(LITTLEFS, "/config_wifi.html", "text/html");
  });

  server->on("/update_wifi_config", HTTP_POST, [updateWifi](AsyncWebServerRequest *request){
    Serial.println("Processing WIFI config request");
    const char *ssid = NULL;
    const char *psk = NULL;
    const char *hostname = NULL;

    if (request->hasParam("ssid", true)) {
      ssid = request->getParam("ssid", true)->value().c_str();
    }
    if (request->hasParam("psk", true)) {
      psk = request->getParam("psk", true)->value().c_str();
    }
    if (request->hasParam("hostname", true)) {
      hostname = request->getParam("hostname", true)->value().c_str();
    }

    if (ssid == NULL || psk == NULL) {
      request->send(400);
      return;
    }

    Serial.println("request handled");
    if (updateWifi(ssid, psk, hostname)) {
      request->send(201);
    } else {
      request->send(500);
    }
  });

  server->addHandler(ws);
  server->onNotFound(notFound);

  server->begin();
  Serial.println("HTTP ready");
}
