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

    int newPressure = analogRead(pressurePin);
    pressure = ((pressure * 5) + newPressure) / 6;

  again:
    if (isReaching) {
      if (pressure == goalPressure) {
        isReaching = false;
        inflateVentil = false;
        deflateVentil = false;
      } else if (pressure < goalPressure) {
        inflateVentil = true;
        deflateVentil = false;
      } else if (pressure > goalPressure) {
        inflateVentil = false;
        deflateVentil = true;
      }
    } else {
      if (pressure > lowThreshold || pressure > highThreshold) {
        isReaching = true;
        goto again;
      }
    }

    // set the status of the ventils
    digitalWrite(inflatePin, inflateVentil);
    digitalWrite(deflatePin, deflateVentil);
  }

  /**
   * Set the goal pressure
   **/
  void PneumaticSystem::setGoalPressure(const int _goalPressure) {
    goalPressure = _goalPressure;
    if (goalPressure >= pressure) {
      highThreshold = goalPressure;
      lowThreshold = goalPressure + 3;
    } else {
      highThreshold = goalPressure - 3;
      lowThreshold = goalPressure;
    }
    isReaching = true;
  }

  void PneumaticSystem::debugStatus() {
    if (!active) {
      printMessage("inactive");
      return;
    }

    printMessage("pressure");
    printNumber(pressure);
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
    Serial.write((pressure >> 7) & 0x7F);
    Serial.write(pressure & 0x7F);
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