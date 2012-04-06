class PneumaticCommandDispatcher implements PneumaticProtocolDecoder.CommandHandler {
  protected PneumaticSystem systems[];

  PneumaticCommandDispatcher(PneumaticSystem _systems[]) {
    systems = _systems;
  }


  public void handleCommand(int cmd, byte buf[]) {
    switch (cmd | 0x80) {
    case PneumaticProtocolDecoder.COMMAND_SET_GOAL_PRESSURE:
      break;

    case PneumaticProtocolDecoder.COMMAND_PRINT_PRESSURE:
      {
        int pressure = buf[1] << 7 | buf[2];
        byte systemNumber = buf[0];
        if (systemNumber < systems.length && systems[systemNumber] != null) {
          systems[systemNumber].printPressure(pressure);
        }
      }
      break;

    case PneumaticProtocolDecoder.COMMAND_PRINT_NUMBER:
      {
        int number = buf[1] << 7 | buf[2];
        byte systemNumber = buf[0];
        if (systemNumber < systems.length && systems[systemNumber] != null) {
          systems[systemNumber].printNumber(number);
        }
      }
      break;

    case PneumaticProtocolDecoder.COMMAND_PRINT_MESSAGE:
      {
        byte systemNumber = buf[0];
        byte _str[] = new byte[buf.length - 1];
        for (int i = 1; i < buf.length; i++) {
          _str[i - 1] = buf[i];
        }
        if (systemNumber < systems.length && systems[systemNumber] != null) {
          systems[systemNumber].printMessage(new String(_str));
        }
      }
      break;

    case PneumaticProtocolDecoder.COMMAND_PRINT_STATUS:
      {
        byte systemNumber = buf[0];
        boolean isRising = (buf[1] & 1) != 0;
        boolean inflateVentil = (buf[1] & (1 << 1)) != 0;
        boolean deflateVentil = (buf[1] & (1 << 2)) != 0;
        if (systemNumber < systems.length && systems[systemNumber] != null) {
          systems[systemNumber].printStatus(isRising, inflateVentil, deflateVentil);
        }

      }
      break;

    default:
      break;
    }
  }
}
