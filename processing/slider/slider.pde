/*
 * ----------------------------------
 *   Slider Serial - Processing
 * ----------------------------------
 *
 * Uses the Processing serial library and the
 * ControlP5 GUI library to control the brightness
 * of an LED connnected to an Arduino.
 *
 * DEPENDENCIES:
 *   Processing Serial Library (installed by default)
 *     http://processing.org/reference/libraries/serial/
 *   Processing ControlP5 Library (must be installed)
 *     http://www.sojamo.de/libraries/controlP5/
 *   A connected Arduino running "Slider Serial - Arduino"
 *     http://principialabs.com/arduino-processing-serial-communication
 *
 * GET HELP:
 *   Installing libraries in Processing
 *     http://www.learningprocessing.com/tutorials/libraries/
 *
 * Created:  6 April 2011
 * Author:   Brian D. Wendt
 *   (http://principialabs.com/)
 * Version:  1.0
 * License:  GPLv3
 *   (http://www.fsf.org/licensing/)
 *
 */

import controlP5.*;
import processing.serial.*;

ControlP5 controlP5;
Serial serial;

PneumaticProtocolDecoder decoder;

static final int P_ATMOSPHERE = 200; // atmospheric pressure
static final int P_REIFEN = 230;

class CommandHandler implements PneumaticProtocolDecoder.CommandHandler {
  public int xPos;

  CommandHandler() {
    xPos = 0;
  }

  public void handleCommand(int cmd, byte buf[]) {
    switch (cmd | 0x80) {
    case PneumaticProtocolDecoder.COMMAND_SET_GOAL_PRESSURE:
      break;

    case PneumaticProtocolDecoder.COMMAND_PRINT_PRESSURE:
      {
        int pressure = buf[1] << 7 | buf[2];
        byte systemNumber = buf[0];
        System.out.println("system " + systemNumber + " pressure: " + pressure);

        int y = (int)map(pressure, P_ATMOSPHERE, P_REIFEN, 0, 100);

        // draw the line:
        stroke(127,34,255);
        line(xPos, height, xPos, height - y);

        // at the edge of the screen, go back to the beginning:
        if (xPos >= width) {
          xPos = 0;
          background(0);
        } else {
          // increment the horizontal position:
          xPos++;
        }

      }
      break;

    case PneumaticProtocolDecoder.COMMAND_PRINT_NUMBER:
      {
        int pressure = buf[1] << 7 | buf[2];
        byte systemNumber = buf[0];
        System.out.println("system " + systemNumber + " number: " + pressure);

      }
      break;

    case PneumaticProtocolDecoder.COMMAND_PRINT_MESSAGE:
      {
        byte systemNumber = buf[0];
        byte _str[] = new byte[buf.length - 1];
        for (int i = 1; i < buf.length; i++) {
          _str[i - 1] = buf[i];
        }
        System.out.println("system " + systemNumber + " debug: " + new String(_str));
      }
      break;

    default:
      break;
    }
  }
}

CommandHandler handler;

void setup() {
  handler = new CommandHandler();
  decoder = new PneumaticProtocolDecoder(handler);
  // Draw the GUI window
  size(400,350);
  background(0);

  // =======================================================
  //            SERIAL COMMUNICATION SETUP:
  // =======================================================

  // CHANGE THE FOLLOWING VARIABLE to match the port
  // to which your Arduino is connected.

  // SEE THE LIST of available ports in the black debugging
  // section at the bottom of the Processing window. (It will
  // appear after the first time you run the sketch.

  // THE LIST LOOKS LIKE THIS on Windows:
  //   [0] "COM1"
  //   [1] "COM3"
  //   [2]  etc...
  // or like this on a Mac:
  //   [0] "/dev/tty.usbserial-somenumbers"
  //   [1] "/dev/tty.usbserial-othernumbers"
  //   [3]  etc...

  // TYPE THE NUMBER (inside the brackets) of the desired
  // port after the equals sign.

  int serialPortNumber = 0;

  // =======================================================

  println(Serial.list());
  String port = Serial.list()[serialPortNumber];
  serial = new Serial(this, port, 115200);

  // Add a vertical slider control
  controlP5 = new ControlP5(this);
  //("SLIDERNAME", min,max, startpos, xpos,ypos, width,height);
  controlP5.addSlider("DRUCK", P_ATMOSPHERE, P_REIFEN, 50, 190,50, 20,200);

  // Configure the slider properties
  Slider s1 = (Slider)controlP5.controller("DRUCK");
  s1.setSliderMode(Slider.FLEXIBLE);
  s1.setNumberOfTickMarks(21);
  s1.showTickMarks(true);
  s1.snapToTickMarks(false);
}

void draw() {
  fill(0);
  rect(190, 50, 100, 200);

  //  background(0);
  while (serial.available() > 0) {
    byte b = (byte)serial.read();
    decoder.handleByte(b);
  }
}

void setPressure(int system, int pressure) {
  serial.write(0x81);
  serial.write(system);
  serial.write((pressure >> 7) & 0x7F);
  serial.write((pressure & 0x7F));
}

int prevValue = 0;
void LED(float LEDvalue) {
  // Grab slider value (0-100) and send to Arduino
  int LEDbrightness = round(LEDvalue);
  if (prevValue != LEDbrightness) {
    setPressure(0, LEDbrightness);
    println(LEDbrightness);
    prevValue = LEDbrightness;
  }
}
