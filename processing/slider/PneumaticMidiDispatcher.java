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
        SYSTEM_1_ID,
        SYSTEM_2_ID,
        SYSTEM_3_ID,
        SYSTEM_4_ID,
        SYSTEM_5_ID,
        SYSTEM_6_ID
    }

    static Map<Integer, EnumSet<SystemIds>> ccToSystems;

    static {
        ccToSystems = new HashMap<Integer, EnumSet<SystemIds>>();
    }

    /**
     *
     * CC 5 = System 1
     * CC 6 = System 2
     * CC 7 = System 3
     * CC 8 = System 4
     * CC 9 = System 5
     * CC 10 = System 6
     *
     * CC 11 = System 1 + 2
     * CC 12 = System 1 + 3
     * CC 13 = System 1 + 4
     * CC 14 = System 1 + 5
     * CC 15 = System 1 + 6
     *
     * CC 16 = System 2 + 3
     * CC 17 = System 2 + 4
     * CC 18 = System 2 + 5
     * CC 19 = System 2 + 6
     *
     * CC 20 = System 3 + 4
     * CC 21 = System 3 + 5
     * CC 22 = System 3 + 6
     *
     * CC 23 = System 4 + 5
     * CC 24 = System 4 + 6
     *
     * CC 25 = System 5 + 6
     *
     **/
    void controllerChangeReceived(rwmidi.Controller cc) {
        int _cc = cc.getCC();
        if (_cc < 5) {
            return;
        }
        int _val = cc.getValue();
        int _system = _cc - 5;
        if (_system < systems.length) {
            systems[_system].setPressure((int)(PApplet.map(_val, 0, 127, PneumaticSystem.P_ATMOSPHERE, systems[_system].maxPressure)));
        }
    }

}
