#ifndef WIFI_H
#define WIFI_H

// types
typedef struct WIFIConfig {
  char ssid[30];
  char password[30];
  char hostname[30];
} WIFIConfig;

class WIFI {
  const char *configFile;
  bool online = false;
  bool accessPoint = false;

public:
  WIFI(const char *configPath);
  bool startWIFI();
  bool writeWIFIConfig(const char* ssid, const char* password, const char* hostname);
  bool isOnline();
  bool isAccessPoint();
  IPAddress IP();

private:
  bool readConfig(WIFIConfig *config);
  bool connectToAccessPoint(WIFIConfig *config);
  bool connectDefault();
};

#endif