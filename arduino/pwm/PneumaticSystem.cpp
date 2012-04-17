#include "PneumaticSystem.h"

void PneumaticSystem::init() {
  // control for input ventil (pump air into system)
  pinMode(inflatePin, OUTPUT);
  // control for output ventil (let air out of system)
  pinMode(deflatePin, OUTPUT);
  // pressure sensor analog input
  pinMode(pressurePin, INPUT);

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

  if (_pressure > 240) {
    printMessage("safety deflation");
    digitalWrite(inflatePin, false);
    digitalWrite(deflatePin, true);
    return;
  }

  if (isPWM) {
    if (pwmPressure >= 64) {
      digitalWrite(deflatePin, false);
      analogWrite(inflatePin, (pwmPressure - 64) * 4);
    } else {
      digitalWrite(inflatePin, false);
      analogWrite(deflatePin, (63 - pwmPressure) * 4);
    }
  } else {
  again:
    if (isReaching) {
      printMessage("is Reaching");
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
    } else {
      printMessage("low, high, _pressure");
      printNumber(lowThreshold);
      printNumber(highThreshold);
      printNumber(_pressure);
      if (_pressure < lowThreshold || _pressure > highThreshold) {
        printMessage("adjust");
        isReaching = true;
        goto again;
      }
    }

    if (inflateVentil) {
      // set the status of the ventils
      // digitalWrite(inflatePin, inflateVentil);
      unsigned int pwm = 145 + (goalPressure - _pressure) * 4;
      analogWrite(inflatePin, pwm);
      //      analogWrite(inflatePin, pwmPressure);
    } else {
      digitalWrite(inflatePin, false);
    }
    digitalWrite(deflatePin, deflateVentil);
  }
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
  isPWM = false;
}

void PneumaticSystem::setPwmPressure(const int _pwmPressure) {
  pwmPressure = _pwmPressure;
  //  isPWM = true;
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
