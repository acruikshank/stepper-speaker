#include "http.h"

HTTP::HTTP(int port) {
  server = new AsyncWebServer(port);
  ws = new AsyncWebSocket("/ws");
}

const char* PARAM_PAGE = "page";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void HTTP::setupWebServer(LIGHT *l, updateWifiHandler updateWifi, songUpdatedHandler songUpdated) {
  light = l;

  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {    
    request->send(LITTLEFS, "/index.html", "text/html");
  });

  server->on("/song", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("downloading upload.html");
    request->send(LITTLEFS, "/upload.html", "text/html");
  });


  ///////////////////
  //  SONG UPLOAD
  ///////////////////

  server->on("/song/upload", HTTP_POST, [](AsyncWebServerRequest *request) {      
    request->send(200, "text/plain; charset=utf-8", "OK");
  }, [this, songUpdated](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      Serial.println("start writing song");
      song_file = LITTLEFS.open("/SONG", "w");
    }
    Serial.printf("write %d bytes\n", len);
    for (size_t i = 0; i < len; i++) {
      Serial.printf("%d\t", data[i]);
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

  server->on("/update_song", HTTP_POST, 
      [](AsyncWebServerRequest *request){}, NULL, 
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {

    // uint8_t buf[8];
    // for (int i=0; i<8; i++)
    //   buf[i] = data[i];

    // ColorUpdate *u = (ColorUpdate *) &buf;
    // Serial.printf("Update pattern[%d] to %x\n", u->position, u->color);

    // state = u->color;

    // if (u->position >= 0 && u->position < NUM_COLORS) {
    //   pattern[u->position] = u->color;
    // }

    // for(int i = 0; i < NUM_LEDS; i++) {
    //   leds[i] = CRGB(pattern[i%NUM_COLORS]);
    // }

    // FastLED.show();

    request->send(200);
  });


  server->addHandler(ws);
  server->onNotFound(notFound);

  server->begin();
  Serial.println("HTTP ready");
}
