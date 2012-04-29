#ifndef PNEUMATIC_SYSTEM_H__
#define PNEUMATIC_SYSTEM_H__

#include "pneumatic.h"

class PneumaticSystem {
public:
  bool active;
  byte systemNumber; // number of system
  unsigned long pressure; // current pressure of system
  unsigned int goalPressure; // pressure that needs to be attained
  unsigned int pwmPressure; // pwm pressure
  unsigned int lowThreshold;
  unsigned int highThreshold;

  unsigned long startRamp;

  bool safetyTime;

  bool isReaching; // are we currently inflating
  bool inflateVentil; // status of the inflate ventil
  bool deflateVentil; // status of the deflate ventil

  int inflatePin, deflatePin, pressurePin;

  PneumaticSystem(const int _num, const int _inflatePin, const int _deflatePin, const int _pressurePin) :
    systemNumber(_num), inflatePin(_inflatePin), deflatePin(_deflatePin), pressurePin(_pressurePin) {
    active = false;
  }

  void init();
  void tick();
  void setGoalPressure(const int _goalPressure);
  void setPwmPressure(const int _pwmPressure);
  void printStatus();
  void printPressure();
  void printNumber(int number);
  void printMessage(char const *msg);
  void debugStatus();
};


extern PneumaticSystem systems[6];



#endif /* PNEUMATIC_SYSTEM_H__ */
