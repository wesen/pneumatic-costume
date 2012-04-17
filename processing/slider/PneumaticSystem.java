import processing.core.*;
import processing.serial.*;
import controlP5.*;

class PneumaticSystem {
  public static final int P_ATMOSPHERE = 199;

  public int    number;
  public int    maxPressure;
  public String name;

  protected PApplet   applet;
  protected ControlP5 controlP5;
  protected Serial    serial;
  protected Textlabel pressureLabel;
  protected Slider    slider;
  protected Slider    pwmSlider;

  protected int xPos;
  protected int top;
  protected int bottom;
  protected int height;
  protected static final int LEFT = 170;

  protected int prevPressure;
  private int prevPwmPressure;

  public PneumaticSystem(PApplet _applet, ControlP5 _controlP5, Serial _serial, String _name, int _number, int _maxPressure) {
    applet = _applet;
    controlP5 = _controlP5;
    serial = _serial;
    number = _number;
    name = _name;
    maxPressure = _maxPressure;

    xPos = LEFT + 140;
    height = 130;

    prevPressure = 0;

    slider = controlP5.addSlider(name, P_ATMOSPHERE, maxPressure, LEFT + 30, 30 + number * height, 20, 100);
    pressureLabel = controlP5.addTextlabel("DRUCK_L_" + number, "200", LEFT, 140 + number * height);
    slider.setSliderMode(Slider.FLEXIBLE);
    slider.setNumberOfTickMarks(21);
    slider.showTickMarks(true);
    slider.snapToTickMarks(false);
    slider.setDecimalPrecision(0);

    pwmSlider = controlP5.addSlider(name, 0, 127, LEFT + 80, 30 + number * height, 20, 100);
    pwmSlider.setSliderMode(Slider.FLEXIBLE);
    pwmSlider.setNumberOfTickMarks(21);
    pwmSlider.showTickMarks(true);
    pwmSlider.snapToTickMarks(false);
    pwmSlider.setDecimalPrecision(0);

    top = 20 + number * height;
    bottom = 140 + number * height;
  }

  public void draw() {
    applet.fill(0);
    applet.rect(LEFT, top, 140, height);

    int pressure = applet.round(slider.value());
    if (pressure != prevPressure) {
      setPressure(pressure);
      applet.println(pressure);
      prevPressure = pressure;
    }

    int pwmPressure = applet.round(pwmSlider.value());
    if (pwmPressure != prevPwmPressure) {
      setPwmPressure(pwmPressure);
      applet.println("pwm: " + pwmPressure);
      prevPwmPressure = pwmPressure;
    }
  }

  private void setPwmPressure(int pwmPressure) {
    if (serial != null) {
      serial.write(0x86);
      serial.write(number);
      serial.write(pwmPressure & 0x7F);
    } else {
      applet.println("No serial interface selected");
    }
    pwmSlider.setValue(pwmPressure);
  }

  public void setPressure(int pressure) {
    if (serial != null) {
      serial.write(0x81);
      serial.write(number);
      serial.write((pressure >> 7) & 0x7f);
      serial.write((pressure & 0x7f));
    } else {
      applet.println("No serial interface selected");
    }

    slider.setValue(pressure);
  }

  public void printPressure(int pressure) {
    //        System.out.println("system " + systemNumber + " pressure: " + pressure);
    pressureLabel.setValue("" + pressure);

    int y = (int) applet.constrain(applet.map(pressure, P_ATMOSPHERE, maxPressure, 0, 98), 0, 98);

    // draw the line:
    applet.stroke(127, 34, 255);
    applet.line(xPos, bottom, xPos, bottom - y);

    // at the edge of the screen, go back to the beginning:
    if (xPos >= applet.width) {
      xPos = LEFT + 140;
      applet.rect(xPos, top, applet.width, height);
    } else {
      // increment the horizontal position:
      xPos++;
    }
  }

  public void printNumber(int _number) {
    System.out.println("system " + number + " number: " + _number);
  }

  public void printMessage(String message) {
    System.out.println("system " + number + " debug: " + message);
  }

  public void printStatus(boolean isRising, boolean inflateVentil, boolean deflateVentil) {
    if (isRising) {
      applet.stroke(14, 250, 14);
      applet.line(xPos, bottom - 100, xPos, bottom - 103);
    }
    if (inflateVentil) {
      applet.stroke(250, 14, 14);
      applet.line(xPos, bottom - 104, xPos, bottom - 107);
    }

    if (deflateVentil) {
      applet.stroke(14, 14, 250);
      applet.line(xPos, bottom - 108, xPos, bottom - 111);
    }

    //        System.out.println("system " + systemNumber + " status " + (isRising ? "rising" : "") + " "
    //                 + (inflateVentil ? "inflate" : "") + " " + (deflateVentil ? "deflate" : ""));
  }

  public void setSerial(Serial serial) {
    this.serial = serial;
  }
}
