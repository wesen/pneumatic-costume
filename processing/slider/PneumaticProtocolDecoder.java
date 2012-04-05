import java.util.*;

public class PneumaticProtocolDecoder {
  public interface CommandHandler {
    public void handleCommand(int command, byte buf[]);
  }

  protected static final int STATE_IDLE = 0;
  protected static final int STATE_CMD_BYTE_RECVD = 1;
  protected static final int STATE_CMD_PRINT_MESSAGE = 2;
  protected int status;
  protected int msgBytesLeft;
  protected int bufIndex;

  protected CommandHandler handler;

  private static final int CMD_BYTES_LEFT[] = {
    0, // dummy, ignore
    0, // set pressure, ignore
    3, // print pressure
    0, // print message
  };

  public static final int COMMAND_SET_GOAL_PRESSURE = 0x81;
  public static final int COMMAND_PRINT_PRESSURE = 0x82;
  public static final int COMMAND_PRINT_MESSAGE = 0x83;
  public static final int COMMAND_PRINT_NUMBER = 0x84;

  int cmd;
  byte buf[];

  public PneumaticProtocolDecoder(CommandHandler _handler) {
    status = STATE_IDLE;
    bufIndex = 0;
    msgBytesLeft = 0;
    buf = new byte[256];
    this.handler = _handler;
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
        if ((cmd | 0x80) == COMMAND_PRINT_MESSAGE) {
          status = STATE_CMD_PRINT_MESSAGE;
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

      case STATE_CMD_PRINT_MESSAGE:
        buf[bufIndex] = b;
        if (bufIndex >= 1) {
          if (b == 0 || bufIndex >= (buf.length - 1)) {
            buf[bufIndex] = 0;
            handleCommand();
            status = STATE_IDLE;
            return;
          }
        }

        bufIndex++;
        break;

      default:
        status = STATE_IDLE;
        break;
      }
    }
  }

  public void handleCommand() {
    byte _buf[] = new byte[bufIndex];
    for (int i = 0; i < bufIndex; i++) {
      _buf[i] = buf[i];
    }
    handler.handleCommand(cmd, _buf);
  }
};
