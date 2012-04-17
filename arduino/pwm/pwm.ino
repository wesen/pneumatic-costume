/***************************************************************************
 *
 * Pneumatic control for valve costume
 *
 ***************************************************************************/

#include "pneumatic.h"

#include "PneumaticSystem.h"
#include "PneumaticProtocolDecoder.h"

PneumaticSystem systems[4] = {
  PneumaticSystem(0, 2, 3, A0),
  PneumaticSystem(1, 4, 5, A1),
  PneumaticSystem(2, 6, 7, A2),
  PneumaticSystem(3, 8, 9, A3)
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
//  systems[1].active = true;

  systems[0].printMessage("Booting up system");
  systems[1].printMessage("Booting up system");

  systems[0].setGoalPressure(0);
  systems[1].setGoalPressure(1);
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
