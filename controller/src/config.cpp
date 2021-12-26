#include "config.h"
#include <FS.h>
#include <LITTLEFS.h>

// constructor
CONFIG::CONFIG(const char *cf) {
  configFile = cf;
}

// public methods
bool CONFIG::writeConfig() {
  Serial.printf("Writing config: %s\n", configFile);

  File f = LITTLEFS.open(configFile, "w");
  if (!f) {
    Serial.printf("Failed to open %s for writing.", configFile);
    return false;
  }

  f.write((uint8_t *) this, sizeof(CONFIG));
  f.close();
  Serial.println("wrote config");
  return true;
}

bool CONFIG::readConfig() {
  if (!LITTLEFS.exists(configFile)) {
    return false;
  }
  Serial.printf("Config file found: %s\n", configFile);

  File f = LITTLEFS.open(configFile, "r");
  if (!f) {
    Serial.printf("Couldn't open config file: %s", configFile);
    return false;
  }

  f.readBytes((char *) this, sizeof(CONFIG));
  f.close();

  return true;
}
