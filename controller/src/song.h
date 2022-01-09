#include <Arduino.h>

#ifndef SONG_H
#define SONG_H

#include "motor_control.h"
#include "light.h"

#define MAX_NOTES 5000

using namespace std;

typedef struct Note {
  uint32_t duration;
  uint32_t period;
} Note;

#pragma pack(1)
typedef struct SongStatus {
  bool songLoaded;
  bool songPlaying;
  bool songLooping;
} SongStatus;

// types
class SONG {
  MOTOR *motor;
  LIGHT *light;

  bool looping;
  bool playing;
  uint16_t noteCount;
  int16_t noteIndex = 0;

  Note notes[MAX_NOTES];

  TaskHandle_t songTask;
  bool songTaskInitialized = false;

public:
  bool loaded;

  SONG();
  void init(MOTOR *, LIGHT *);

  // song file i/o
  bool writeSong();
  bool readSong();

  // playing songs
  void setLoop(bool loop);
  void start();
  void stop();

  // iterate song
  Note nextNote();
  bool hasNextNote();

  // introspection
  SongStatus getStatus();
  void debugSong();
};

#endif

