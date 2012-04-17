#ifndef PNEUMATIC_SYSTEM_H__
#define PNEUMATIC_SYSTEM_H__

#include "pneumatic.h"

class PneumaticSystem {
public:
  bool active;
  int systemNumber; // number of system
  int pressure; // current pressure of system
  int goalPressure; // pressure that needs to be attained
  int lowThreshold;
  int highThreshold;

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
  void printStatus();
  void printPressure();
  void printNumber(int number);
  void printMessage(char const *msg);
  void debugStatus();
};


extern PneumaticSystem systems[4];



#endif /* PNEUMATIC_SYSTEM_H__ */