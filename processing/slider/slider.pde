/*
 *   Pneumatic Costume Control - Processing
 * ----------------------------------
 *
 * Author:   Manuel Odendahl,
 *   based on slider processing example by Brian D. Wendt
 *   (http://principialabs.com/)
 * Version:  1.0
 * License:  GPLv3
 *   (http://www.fsf.org/licensing/)
 *
 */

import controlP5.*;
import processing.serial.*;
import rwmidi.*;
MidiInput input;

public final int P_REIFEN = 230;

PneumaticProtocolDecoder decoder;
PneumaticSystem systems[];
Serial serial;

void setup() {
  // Draw the GUI window
  size(800,350);
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


  String inputDeviceNames[] = RWMidi.getInputDeviceNames();
  for (int i = 0; i < inputDeviceNames.length; i++) {
    println(i + ": " + inputDeviceNames[i]);
  }

  input = RWMidi.getInputDevices()[0].createInput(this);

  ControlP5 controlP5 = new ControlP5(this);

  systems = new PneumaticSystem[2];
  systems[0] = new PneumaticSystem(this, controlP5, serial, "ARME", 0, P_REIFEN);
  systems[1] = new PneumaticSystem(this, controlP5, serial, "KIEMEN", 1, P_REIFEN);
  decoder = new PneumaticProtocolDecoder(new PneumaticCommandDispatcher(systems));

}

void draw() {
  for (PneumaticSystem s : systems) {
    if (s != null) {
      s.draw();
    }
  }

  //  background(0);
  while (serial.available() > 0) {
    byte b = (byte)serial.read();
    decoder.handleByte(b);
  }
}

void controllerChangeReceived(rwmidi.Controller cc) {
  int _cc = cc.getCC();
  if (_cc < 5) {
    return;
  }
  int _val = cc.getValue();
  int _system = _cc - 5;
  if (_system < systems.length) {
    systems[_system].setPressure((int)(map(_val, 0, 127, PneumaticSystem.P_ATMOSPHERE, systems[_system].maxPressure)));
  }
}
