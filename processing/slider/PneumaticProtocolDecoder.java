public class PneumaticProtocolDecoder {
  protected static final int STATE_IDLE = 0;
  protected static final int STATE_CMD_BYTE_RECVD = 1;
  protected int status;
  protected int msgBytesLeft;
  protected int bufIndex;

  private static final int CMD_BYTES_LEFT[] = {
    0, // dummy, ignore
    0, // set pressure, ignore
    3, // print pressure
  };

  public static final int COMMAND_SET_GOAL_PRESSURE = 0xf1;
  public static final int COMMAND_PRINT_PRESSURE = 0xf2;

  int cmd;
  byte buf[];

  public PneumaticProtocolDecoder() {
    status = STATE_IDLE;
    bufIndex = 0;
    msgBytesLeft = 0;
    buf = new byte[8];
  }

  public void handleByte(final byte b) {
    // check if command byte
    if ((b & (byte)0x80) != 0) {
      cmd = b & 0x7F;
      bufIndex = 0;
      status = STATE_CMD_BYTE_RECVD;
      if (cmd >= CMD_BYTES_LEFT.length) {
        // unknown command, ignore
        status = STATE_IDLE;
      } else {
        msgBytesLeft = CMD_BYTES_LEFT[cmd];
        if (msgBytesLeft == 0 || msgBytesLeft > buf.length) {
          // handle ignored commands
          status = STATE_IDLE;
        }
      }
    } else {
      switch (status) {
      case STATE_IDLE:
        // do nothing, spurious byte
        break;

      case STATE_CMD_BYTE_RECVD:
        buf[bufIndex++] = b;
        if (--msgBytesLeft == 0) {
          // command successfully received, handle
          status = STATE_IDLE;
          handleCommand();
        }
        break;

      default:
        status = STATE_IDLE;
        break;
      }
    }
  }

  public void handleCommand() {
    switch (cmd | 0xF0) {
    case COMMAND_SET_GOAL_PRESSURE:
      break;

    case COMMAND_PRINT_PRESSURE:
      {
        int pressure = buf[1] << 7 | buf[2];
        byte systemNumber = buf[0];
        System.out.println("system " + systemNumber + " pressure: " + pressure);
      }
      break;

    default:
      break;
    }
  }
};
