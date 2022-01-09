#include "song.h"
#include <FS.h>
#include <LITTLEFS.h>

#define SONG_FILE "/SONG"

// constructor
SONG::SONG() {
  looping = false;
}

void SONG::init(MOTOR *_motor, LIGHT *_light) {
  motor = _motor;
  light = _light;
  songTaskInitialized = false;
  readSong();
}

// public methods
bool SONG::writeSong() {
  Serial.printf("Writing song: %s\n", SONG_FILE);

  File f = LITTLEFS.open(SONG_FILE, "w");
  if (!f) {
    Serial.printf("Failed to open %s for writing.", SONG_FILE);
    return false;
  }

  f.write((uint8_t *) notes, sizeof(Note) * noteCount);
  f.close();
  Serial.println("wrote song");

  if (noteCount > 0) loaded = true;

  return true;
}

bool SONG::readSong() {
  Serial.println("readSong");

  if (!LITTLEFS.exists(SONG_FILE)) {
    Serial.printf("Song file not found: %s\n", SONG_FILE);
    return false;
  }
  Serial.printf("Song file found: %s\n", SONG_FILE);

  File f = LITTLEFS.open(SONG_FILE, "r");
  if (!f) {
    Serial.printf("Couldn't open song file: %s", SONG_FILE);
    return false;
  }

  Serial.println("readSong file opened");

  noteCount = 0;
  Note nextNote;
  size_t noteSize = sizeof(Note);
  while (f.readBytes((char *) &nextNote, noteSize) == noteSize) {
    Serial.printf("read note: %d %d\n", nextNote.duration, nextNote.period);
    notes[noteCount++] = nextNote;
  }
  f.close();

  Serial.printf("readSong: notes read %d\n", noteCount);

  loaded = noteCount > 0;

  return true;
}

Note SONG::nextNote() {  
  noteIndex = (noteIndex + 1) % noteCount;
  Note note = notes[noteIndex];
  light->setHSV(255.0*fmod(log2((double) note.period), 1.0), 255, 150);
  motor->setDirection(!motor->direction);
  motor->play(note.period);
  return note;
}

bool SONG::hasNextNote() {
  return (looping || noteIndex + 1 < noteCount);  
}

void playSong(void *pvParameters) {
  SONG *song = (SONG *) pvParameters;
  while (true) {
    if (!song->hasNextNote()) {
      song->stop();
      return;
    }

    Note note = song->nextNote();

    // Serial.printf("%d: playing {%d, %d}\n", micros(), note.duration, note.period);

    vTaskDelay(note.duration);
  }
}

void SONG::start() {
  stop();
  noteIndex = -1;  // prepare first note (actually max uint16)
  songTaskInitialized = true;
  light->stop();
  playing = true;
  xTaskCreate(playSong, "SongTask", 5000, (void *) this, 1, &songTask);
}

void SONG::stop() {
  playing = false;
  Serial.println("song stopped");
  motor->stop();
  light->blink(light->currentColor);
  if (songTaskInitialized) {
    songTaskInitialized = false;
    vTaskDelete(songTask);
  }
}

SongStatus SONG::getStatus() {
  return SongStatus{
    songLoaded: loaded,
    songPlaying: playing,
    songLooping: looping,
  };
}

void SONG::setLoop(bool loop) {
  looping = loop;
}

void SONG::debugSong() {
  if (noteCount > MAX_NOTES) {
    Serial.printf("ERROR: notes count (%d) exceeds %d\n", noteCount, MAX_NOTES);
    return;
  }

  for (int i=0; i<noteCount; i++) {
    Serial.printf("%d: %d %d\n", i, notes[i].duration, notes[i].period);
  }
}