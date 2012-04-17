/**
 * User: manuel
 * Date: 17/4/12
 */

import java.util.EnumSet;
import java.util.HashMap;
import java.util.Map;
import processing.core.PApplet;
import rwmidi.MidiInput;
import rwmidi.RWMidi;

public class PneumaticMidiDispatcher {
  MidiInput input;
  private PneumaticSystem[] systems;

  public PneumaticMidiDispatcher(PneumaticSystem[] systems) {
    this.systems = systems;
  }

  public void setPort(int value) {
    if (input != null) {
      input.close();
      input = null;
    }
    input = RWMidi.getInputDevices()[value].createInput(this);
  }

  enum SystemIds {
    SYSTEM_1_ID(0),
    SYSTEM_2_ID(1),
    SYSTEM_3_ID(2),
    SYSTEM_4_ID(3),
    SYSTEM_5_ID(4),
    SYSTEM_6_ID(5);
    public int idx;

    SystemIds(int i) {
      this.idx = i;
    }
  }

  static Map<Integer, EnumSet<SystemIds>> ccToSystems;

  static {
    ccToSystems = new HashMap<Integer, EnumSet<SystemIds>>();
    ccToSystems.put(5, EnumSet.of(SystemIds.SYSTEM_1_ID));
    ccToSystems.put(6, EnumSet.of(SystemIds.SYSTEM_2_ID));
    ccToSystems.put(7, EnumSet.of(SystemIds.SYSTEM_3_ID));
    ccToSystems.put(8, EnumSet.of(SystemIds.SYSTEM_4_ID));
    ccToSystems.put(9, EnumSet.of(SystemIds.SYSTEM_5_ID));
    ccToSystems.put(10, EnumSet.of(SystemIds.SYSTEM_6_ID));

    ccToSystems.put(11, EnumSet.of(SystemIds.SYSTEM_1_ID, SystemIds.SYSTEM_2_ID));
  }

  void controllerChangeReceived(rwmidi.Controller cc) {
    int _cc = cc.getCC();

    if (ccToSystems.containsKey(_cc)) {
      int _val = cc.getValue();
      for (SystemIds id : ccToSystems.get(_cc)) {
        if (id.idx < systems.length) {
          systems[id.idx].setPressure((int) (PApplet.map(_val, 0, 127, PneumaticSystem.P_ATMOSPHERE, systems[id.idx].maxPressure)));
        }
      }
    }
  }
}
