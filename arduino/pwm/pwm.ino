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

class System {
public:
  int systemNumber; // number of system
  int pressure; // current pressure of system
  int goalPressure; // pressure that needs to be attained
  bool isRising; // are we currently inflating
  bool inflateVentil; // status of the inflate ventil
  bool deflateVentil; // status of the deflate ventil

  int inflatePin, deflatePin, pressurePin;

  System(const int _num, const int _inflatePin, const int _deflatePin, const int _pressurePin) :
    systemNumber(_num), inflatePin(_inflatePin), deflatePin(_deflatePin) {
  }

  void init() {
    // control for input ventil (pump air into system)
    pinMode(inflatePin, OUTPUT);
    // control for output ventil (let air out of system)
    pinMode(deflatePin, OUTPUT);
    // pressure sensor analog input
    pinMode(pressurePin, INPUT);
    goalPressure = pressure = analogRead(pressurePin);
    isRising = false;
    inflateVentil = false;
    deflateVentil = false;
  }

  /**
   * Read in the current pressure, and adjust ventil according to goal pressure.
   * This is done via hysteresis.
   **/
  void tick() {
    pressure = analogRead(pressurePin);
    if (isRising) {
      if (pressure < goalPressure) {
        inflateVentil = true;
      } else {
        isRising = false;
        inflateVentil = false;
      }
    } else {
      inflateVentil = false;
    }


    if (!isRising) {
      if (pressure < (goalPressure - PRESSURE_HYSTERESIS)) {
        deflateVentil = false;
        isRising = true;
      } else {
        deflateVentil = true;
      }
    } else {
      deflateVentil = false;
    }

    // set the status of the ventils
    digitalWrite(inflatePin, inflateVentil);
    digitalWrite(deflatePin, deflateVentil);
  }

  /**
   * Set the goal pressure and sample to adjust the system immediately
   **/
  void setGoalPressure(const int _goalPressure) {
    goalPressure = _goalPressure;
    isRising = true;
    tick();
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
        if (msgBytesLeft == 0) {
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
    switch (cmd) {
    case COMMAND_SET_GOAL_PRESSURE:
      {
        int pressure = buf[1] << 7 | buf[2];
        byte systemNumber = buf[0];
        if (systemNumber >= countof(systems)) {
          return;
        }
        systems[systemNumber].setGoalPressure(pressure);
      }

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
}

static int cnt = 0;
void loop() {
  if (cnt++ > 1000) {
    cnt = 0;
    for (byte i = 0; i < countof(systems); i++) {
      systems[i].printPressure();
    }
  }

  while (Serial.available()) {
    decoder.handleByte(Serial.read());
  }

  for (byte i = 0; i < countof(systems); i++) {
    systems[i].tick();
  }
}
