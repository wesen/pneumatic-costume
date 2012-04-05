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

void setup() {
  decoder = new PneumaticProtocolDecoder();
  // Draw the GUI window
  size(400,350);

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
  controlP5.addSlider("LED",200,230, 50, 190,50, 20,200);
  // Configure the slider properties
  Slider s1 = (Slider)controlP5.controller("LED");
  s1.setSliderMode(Slider.FLEXIBLE);
  s1.setNumberOfTickMarks(21);
  s1.showTickMarks(true);
  s1.snapToTickMarks(false);
}


void draw() {
  background(0);
  while (serial.available() > 0) {
    byte b = (byte)serial.read();
    decoder.handleByte(b);
  }
}

int prevValue = 0;
void LED(float LEDvalue) {
  // Grab slider value (0-100) and send to Arduino
  int LEDbrightness = round(LEDvalue);
  if (prevValue != LEDbrightness) {
    serial.write(0xF0);
    serial.write(1);
    serial.write((LEDbrightness >> 7) & 0x7F);
    serial.write((LEDbrightness & 0x7F));
    println(LEDbrightness);
    prevValue = LEDbrightness;
  }
}
