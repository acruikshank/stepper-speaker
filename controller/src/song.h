#include <Arduino.h>

#ifndef SONG_H
#define SONG_H

#include "motor_control.h"

#define MAX_NOTES 1000

using namespace std;

typedef struct Note {
  uint32_t duration;
  uint32_t period;
} Note;

// types
class SONG {
  MOTOR *motor;
  TaskHandle_t songTask;
  bool songTaskInitialized = false;
  uint16_t noteCount;
  uint16_t noteIndex = 0;
  Note notes[MAX_NOTES];

public:
  SONG();
  void init(MOTOR *);
  bool writeSong();
  bool readSong();
  void start();
  void stop();
  Note nextNote();
  void debugSong();
};

#endif

