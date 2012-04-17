import controlP5.ControlEvent;
import controlP5.ControlGroup;
import controlP5.ControlP5;
import controlP5.DropdownList;
import processing.core.PApplet;
import processing.serial.Serial;
import rwmidi.RWMidi;

public class PneumaticSketch extends PApplet {
  public final int P_REIFEN = 220;

  PneumaticProtocolDecoder decoder;
  PneumaticSystem          systems[];
  PneumaticMidiDispatcher  midiDispatcher;
  Serial                   serial;
  PApplet                  parent;

  boolean serialInitialized = false;
  DropdownList serialList;
  boolean midiInitialized = false;
  DropdownList midiList;

  public PneumaticSketch(PApplet parent) {
    this.parent = parent;
  }

  public void setup() {
    // Draw the GUI window
    parent.size(880, 350);
    parent.background(0);

    ControlP5 controlP5 = new ControlP5(parent);
    serialList = controlP5.addDropdownList("serial-list", 20, 20, 150, 120);
    int i = 0;
    for (String name : Serial.list()) {
      serialList.addItem(name, i);
      i++;
    }

    midiList = controlP5.addDropdownList("midi-list", 20, 120, 150, 120);
    i = 0;
    for (String name : RWMidi.getInputDeviceNames()) {
      midiList.addItem(name, i);
      i++;
    }

    serial = null;

    systems = new PneumaticSystem[2];
    systems[0] = new PneumaticSystem(parent, controlP5, serial, "ARME", 0, P_REIFEN);
    systems[1] = new PneumaticSystem(parent, controlP5, serial, "KIEMEN", 1, P_REIFEN);
    decoder = new PneumaticProtocolDecoder(new PneumaticCommandDispatcher(systems));

    midiDispatcher = new PneumaticMidiDispatcher(systems);
  }

  public void draw() {
    parent.fill(0);
    parent.rect(0, 0, 170, parent.height);

    for (PneumaticSystem s : systems) {
      if (s != null) {
        s.draw();
      }
    }

    if (serial != null) {
      //  background(0);
      while (serial.available() > 0) {
        byte b = (byte) serial.read();
        decoder.handleByte(b);
      }
    }
  }

  void controlEvent(ControlEvent theEvent) {
    if (theEvent.isGroup()) {
      ControlGroup group = theEvent.group();
      if (group.name().equals("midi-list")) {
        midiDispatcher.setPort((int)group.value());
      } else if (group.name().equals("serial-list")) {
        if (serial != null) {
          serial.dispose();
          serial = null;
        }

        serial = new Serial(parent, group.stringValue(), 115200);
        for (PneumaticSystem sys : systems) {
          sys.setSerial(serial);
        }
      }
    }
  }


}
