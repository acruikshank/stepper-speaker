#ifndef MOTOR_H
#define MOTOR_H

using namespace std;

// types
class MOTOR {
  hw_timer_t *timer;
  bool timer_set = false;  

public:
  uint8_t direction = false;

  MOTOR();
  void init();
  void debug();
  void play(uint32_t period);
  void stop();
  void setDirection(uint8_t);
};

#endif

