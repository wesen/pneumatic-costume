/***************************************************************************
 *
 * Pneumatic control for valve costume
 *
 ***************************************************************************/

#include "pneumatic.h"

#include "PneumaticSystem.h"
#include "PneumaticProtocolDecoder.h"

// Add additional systems here
// PneumaticSystem(number, inflatePin, deflatePin, analog input);
// Make sure that the inflatePin is one of 2, 3, 5, 6, 7, 8, 44, 45, 46 as these
// use tweaked PWM timers
PneumaticSystem systems[6] = {
  PneumaticSystem(0, 2, 4, A0),
  PneumaticSystem(1, 3, 9, A1),
  PneumaticSystem(2, 5, 10, A2),
  PneumaticSystem(3, 6, 11, A3),
  PneumaticSystem(4, 7, 12, A4),
  PneumaticSystem(5, 8, 13, A5)
};

PneumaticProtocolDecoder decoder;


void setup() {
  for (byte i = 0; i < countof(systems); i++) {
    systems[i].init();
  }

  Serial.begin(115200);

  TCCR3B = (TCCR3B & 0xF8) | 0x04; // set to 120 hz
  TCCR4B = (TCCR4B & 0xF8) | 0x04; // set to 120 hz
  TCCR5B = (TCCR5B & 0xF8) | 0x04; // set to 120 hz

  systems[0].active = true;
  systems[0].setGoalPressure(0);

  systems[1].active = true;
  systems[1].setGoalPressure(0);

  systems[2].active = true;
  systems[2].setGoalPressure(0);

  systems[3].active = true;
  systems[3].setGoalPressure(0);

  systems[4].active = true;
  systems[4].setGoalPressure(0);

  systems[5].active = true;
  systems[5].setGoalPressure(0);
}

static int cnt = 0;

void loop() {
  if (cnt++ >= 10000) {
    cnt = 0;
    for (byte i = 0; i < countof(systems); i++) {
      systems[i].debugStatus();
    }
    delay(2);
  }

  if (cnt % 200 == 0) {
    for (byte i = 0; i < countof(systems); i++) {
      if (systems[i].active) {
        systems[i].printPressure();
        systems[i].printStatus();
      }
    }
  }

  while (Serial.available()) {
    decoder.handleByte(Serial.read());
  }

  for (byte i = 0; i < countof(systems); i++) {
    systems[i].tick();
  }
}
