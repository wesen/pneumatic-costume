#include "PneumaticProtocolDecoder.h"
#include "PneumaticSystem.h"

static const int cmdBytes[] = {
  0, // dummy, ignore
  3, // SET_PRESSURE
  0 // PRINT_PRESSURE, ignore
};

void PneumaticProtocolDecoder::handleByte(const byte b) {
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

void PneumaticProtocolDecoder::handleCommand() {
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
