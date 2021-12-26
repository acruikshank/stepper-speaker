#include <Arduino.h>

#ifndef CONFIG_H
#define CONFIG_H

// types
class CONFIG {
  const char *configFile;

public:
  CONFIG(const char *configFile);
  bool writeConfig();
  bool readConfig();
};

#endif
