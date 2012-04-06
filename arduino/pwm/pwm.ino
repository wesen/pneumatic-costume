/***************************************************************************
 *
 * Pneumatic control for valve costume
 *
 ***************************************************************************/

/** Helper macros **/
#define countof( array ) ( sizeof( array )/sizeof( array[0] ) )


#define PRESSURE_HYSTERESIS 5

/************************************************************************************/

/**
 * Command packets are similar to the midi format. High bit set signals a new message. The lower 7 bits
 * give the command.
 **/
#define COMMAND_BYTE(x) (0x80 | (x))

#define IS_COMMAND_BYTE(x) ((x) & 0x80)
#define GET_COMMAND(x) ((x & 0x7F))

/**
 * SET PRESSURE
 *
 * [0x81] // Command 1
 * [systemNumber] // destination system
 * [high nibble goal pressure] // higher 7 bits of pressure that needs to be attained
 * [low nibble goal pressure] // lower 7 bits of pressure that needs to be attained
 *
 * goal pressure is between 0 and 1023
 **/
#define COMMAND_SET_GOAL_PRESSURE COMMAND_BYTE(1)

/**
 * PRINT PRESSURE
 *
 * [0x82] // Command 2
 * [systemNumber] // destination system
 * [high nibble goal pressure] // higher 7 bits of current pressure
 * [low nibble goal pressure] // lower 7 bits of current pressure
 *
 * pressure is between 0 and 1023
 **/
#define COMMAND_PRINT_PRESSURE COMMAND_BYTE(2)

#define COMMAND_PRINT_MESSAGE COMMAND_BYTE(3)

#define COMMAND_PRINT_NUMBER COMMAND_BYTE(4)

#define COMMAND_PRINT_STATUS COMMAND_BYTE(5)

class System {
public:
  bool active;
  int systemNumber; // number of system
  int pressure; // current pressure of system
  int goalPressure; // pressure that needs to be attained
  int lowThreshold;
  int highThreshold;
  bool isRising; // are we currently inflating
  bool inflateVentil; // status of the inflate ventil
  bool deflateVentil; // status of the deflate ventil

  int inflatePin, deflatePin, pressurePin;

  System(const int _num, const int _inflatePin, const int _deflatePin, const int _pressurePin) :
    systemNumber(_num), inflatePin(_inflatePin), deflatePin(_deflatePin), pressurePin(_pressurePin) {
    active = false;
  }

  void init() {
    // control for input ventil (pump air into system)
    pinMode(inflatePin, OUTPUT);
    // control for output ventil (let air out of system)
    pinMode(deflatePin, OUTPUT);
    // pressure sensor analog input
    pinMode(pressurePin, INPUT);
    goalPressure = pressure = analogRead(pressurePin);
    lowThreshold = goalPressure - 3;
    highThreshold = goalPressure;
    isRising = false;
    inflateVentil = false;
    deflateVentil = false;
  }

  /**
   * Read in the current pressure, and adjust ventil according to goal pressure.
   **/
  void tick() {
    if (!active) {
      return;
    }

    int newPressure = analogRead(pressurePin);
    pressure = ((pressure * 5) + newPressure) / 6;

    if (pressure > lowThreshold) {
      isRising = false;
      inflateVentil = false;
      deflateVentil = true;
    } else if (highThreshold > pressure) {
      isRising = true;
      inflateVentil = true;
      deflateVentil = false;
    } else {
      isRising = false;
      inflateVentil = false;
      deflateVentil = false;
    }

    // set the status of the ventils
    digitalWrite(inflatePin, inflateVentil);
    digitalWrite(deflatePin, deflateVentil);
  }

  /**
   * Set the goal pressure
   **/
  void setGoalPressure(const int _goalPressure) {
    goalPressure = _goalPressure;
    if (goalPressure >= pressure) {
      highThreshold = goalPressure;
      lowThreshold = goalPressure + 3;
    } else {
      highThreshold = goalPressure - 3;
      lowThreshold = goalPressure;
    }
    isRising = true;
  }

  void debugStatus() {
    if (!active) {
      printMessage("inactive");
      return;
    }

    printMessage("pressure");
    printNumber(pressure);
    printMessage("goalPressure");
    printNumber(goalPressure);
    if (isRising) {
      printMessage("rising");
    }

    if (inflateVentil) {
      printMessage("inflate");
    }
    if (deflateVentil) {
      printMessage("deflate");
    }
  }

  void printStatus() {
    Serial.write(COMMAND_PRINT_STATUS);
    Serial.write(systemNumber);
    byte _status = 0;
    if (isRising) {
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
  void printPressure() {
    Serial.write(COMMAND_PRINT_PRESSURE);
    Serial.write(systemNumber);
    Serial.write((pressure >> 7) & 0x7F);
    Serial.write(pressure & 0x7F);
  }

  void printNumber(int number) {
    Serial.write(COMMAND_PRINT_NUMBER);
    Serial.write(systemNumber);
    Serial.write((number >> 7) & 0x7F);
    Serial.write(number & 0x7F);
  }

  void printMessage(char const *msg) {
    Serial.write(COMMAND_PRINT_MESSAGE);
    Serial.write(systemNumber);
    char const *ptr = msg;
    while (*ptr != 0) {
      Serial.write(*ptr++);
    }
    Serial.write((byte)0);
  }
};

System systems[4] = {
  System(0, 2, 3, A0),
  System(1, 4, 5, A1),
  System(2, 6, 7, A2),
  System(3, 8, 9, A3)
};

static const int cmdBytes[] = {
  0, // dummy, ignore
  3, // SET_PRESSURE
  0 // PRINT_PRESSURE, ignore
};

class PneumaticProtocolDecoder {
private:
public:
  enum {
    IDLE = 0,
    CMD_BYTE_RECVD
  } status;
  int msgBytesLeft;
  int bufIndex;

  byte cmd;
  byte buf[8];

  PneumaticProtocolDecoder() :
    status(IDLE), msgBytesLeft(0), bufIndex(0), cmd(0) {
  }

  void handleByte(const byte b) {
    if (IS_COMMAND_BYTE(b)) {
      cmd = GET_COMMAND(b);
      status = CMD_BYTE_RECVD;
      bufIndex = 0;
      if (cmd >= countof(cmdBytes)) {
        // unknown command, ignore
        status = IDLE;
      } else {
        msgBytesLeft = cmdBytes[cmd];
        if (msgBytesLeft == 0 || msgBytesLeft > sizeof(buf)) {
          // handle ignored commands
          status = IDLE;
        }
      }
    } else {
      switch (status) {
      case IDLE:
        // do nothing, spurious byte
        return;

      case CMD_BYTE_RECVD:
        buf[bufIndex++] = b;
        if (--msgBytesLeft == 0) {
          // command successfully received, handle
          status = IDLE;
          handleCommand();
        }
        break;

      default:
        status = IDLE;
        break;
      }
    }
  }

  void handleCommand() {
    switch (cmd | 0x80) {
    case COMMAND_SET_GOAL_PRESSURE:
      {
        int pressure = buf[1] << 7 | buf[2];
        byte systemNumber = buf[0];
        if (systemNumber >= countof(systems)) {
          return;
        }
        systems[systemNumber].setGoalPressure(pressure);
      }
      break;

    default:
      // ignore unknown commands
      break;
    }
  }
};

PneumaticProtocolDecoder decoder;


void setup() {
  for (byte i = 0; i < countof(systems); i++) {
    systems[i].init();
  }

  Serial.begin(115200);

  systems[0].active = true;
  systems[1].active = true;

  systems[0].printMessage("Booting up system");
  systems[1].printMessage("Booting up system");

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
