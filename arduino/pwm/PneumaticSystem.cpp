#include "PneumaticSystem.h"
#include <Arduino.h>

void PneumaticSystem::init() {
  // control for input ventil (pump air into system)
  pinMode(inflatePin, OUTPUT);
  // control for output ventil (let air out of system)
  pinMode(deflatePin, OUTPUT);
  // pressure sensor analog input
  pinMode(pressurePin, INPUT);

  startRamp = 0;
  safetyTime = false;

  digitalWrite(inflatePin, false);
  digitalWrite(deflatePin, false);

  goalPressure = pressure = analogRead(pressurePin);
  lowThreshold = goalPressure - 3;
  highThreshold = goalPressure;
  isReaching = false;
  inflateVentil = false;
  deflateVentil = false;
}

/**
 * Read in the current pressure, and adjust ventil according to goal pressure.
 **/
void PneumaticSystem::tick() {
  if (!active) {
    return;
  }

  unsigned long newPressure = analogRead(pressurePin);
  if (newPressure < 170) { // atmospheric pressure
    // ERROR, open deflateVentil
    printMessage("lost pressure measure");
    digitalWrite(inflatePin, false);
    digitalWrite(deflatePin, true);
    return;
  } else {
    /*
      printMessage("pressure ");
      printNumber(pressure >> 3);
      printMessage("new pressure");
      printNumber(newPressure);
    */
  }
  newPressure <<= 3;
  pressure = ((pressure * 63) + newPressure) / 64;

  unsigned int _pressure = pressure >> 3;

  if (!safetyTime) {
  if (_pressure > 245) {
    printMessage("safety deflation");
    digitalWrite(inflatePin, false);
    digitalWrite(deflatePin, true);
    return;
  }
  }

  static const long pwmLength = 1500;

 again:
  if (isReaching) {
    if (!safetyTime) {
      bool prev_inflateVentil = inflateVentil;

      if (_pressure == goalPressure) {
        isReaching = false;
        inflateVentil = false;
        deflateVentil = false;
      } else if (_pressure < goalPressure) {
        inflateVentil = true;
        deflateVentil = false;
      } else if (_pressure > goalPressure) {
        inflateVentil = false;
        deflateVentil = true;
      }

      if (!prev_inflateVentil && inflateVentil) {
        // wechsel auf ventil muss luft reinlassen -> rampen
        printMessage("start ramp");
        startRamp = millis();
        safetyTime = true;
      }
    }
  } else {
    if (_pressure < lowThreshold || _pressure > highThreshold) {
      isReaching = true;
      goto again;
    }
  }

  if (safetyTime) {
    int safetyDuration = 1000;
    if (systemNumber == 1) {
      safetyDuration = 200;
    }
    if ((millis() - startRamp) > safetyDuration) {
      printMessage("leaving safety time");
      printNumber(millis() - startRamp);
      safetyTime = false;
    }
  }

  if (inflateVentil) {
    static const long startPWM = 240;
    static const long startPWM_ARME = 480;
    static const long endPWM = 512;
    static const long endPWM_ARME = 900;
    static const float slices = 10.0;
    static const long range = (endPWM - startPWM) / slices;
    static const long range_ARME = (endPWM_ARME - startPWM_ARME) / slices;
    static const long interval = (float)pwmLength / slices;

    static int count = 0;

    if (millis() < (startRamp + pwmLength)) {

      long pwm = startPWM +  ((millis() - startRamp) * range) / interval;
      if (systemNumber == 0) {
        pwm = startPWM_ARME + ((millis() - startRamp) * range_ARME) / interval;
      }
      analogWrite(inflatePin, pwm);
      if ((count++) % 50 == 0) {
        printMessage("ramp pwm");
        printNumber(millis() - startRamp);
        printNumber(pwm);
      }
    } else {
      printMessage("open full");
      if (systemNumber == 0) {
        analogWrite(inflatePin, endPWM_ARME);
      } else {
        analogWrite(inflatePin, endPWM);
      }
      //      digitalWrite(inflatePin, true);
    }
  } else {
    digitalWrite(inflatePin, false);
  }

  digitalWrite(deflatePin, deflateVentil);
}

/**
 * Set the goal pressure
 **/
void PneumaticSystem::setGoalPressure(const int _goalPressure) {
  goalPressure = _goalPressure;
  unsigned long _pressure = pressure >> 3;
  highThreshold = goalPressure + 1;
  lowThreshold = goalPressure - 1;
  //  pwmPressure = 110 + (goalPressure - _pressure) * 5;
  isReaching = true;
  /*
    if (systemNumber == 1) {
    int pwm = (goalPressure - 200) * 20;
    analogWrite(inflatePin, pwm);
    printMessage("set pwm");
    printNumber(pwm);
    }
  */
}

void PneumaticSystem::setPwmPressure(const int _pwmPressure) {
  pwmPressure = _pwmPressure;
}

void PneumaticSystem::debugStatus() {
  if (!active) {
    printMessage("inactive");
    return;
  }

  printMessage("pressure");
  printNumber(pressure >> 3);
  printMessage("goalPressure");
  printNumber(goalPressure);
  if (isReaching) {
    printMessage("reaching");
  }

  if (inflateVentil) {
    printMessage("inflate");
  }
  if (deflateVentil) {
    printMessage("deflate");
  }
}

void PneumaticSystem::printStatus() {
  Serial.write(COMMAND_PRINT_STATUS);
  Serial.write(systemNumber);
  byte _status = 0;
  if (isReaching) {
    _status |= 1;
  }
  if (inflateVentil) {
    _status |= (1 << 1);
  }
  if (deflateVentil) {
    _status |= (1 << 2);
  }
  Serial.write(_status);
}

/**
 * Send the current pressure
 **/
void PneumaticSystem::printPressure() {
  Serial.write(COMMAND_PRINT_PRESSURE);
  Serial.write(systemNumber);
  unsigned int _pressure = pressure >> 3;
  Serial.write((_pressure >> 7) & 0x7F);
  Serial.write(_pressure & 0x7F);
}

void PneumaticSystem::printNumber(int number) {
  Serial.write(COMMAND_PRINT_NUMBER);
  Serial.write(systemNumber);
  Serial.write((number >> 7) & 0x7F);
  Serial.write(number & 0x7F);
}

void PneumaticSystem::printMessage(char const *msg) {
  Serial.write(COMMAND_PRINT_MESSAGE);
  Serial.write(systemNumber);
  char const *ptr = msg;
  while (*ptr != 0) {
    Serial.write(*ptr++);
  }
  Serial.write((byte)0);
}
