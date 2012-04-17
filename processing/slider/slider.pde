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

import processing.serial.*;
import rwmidi.*;
import controlP5.*;

PneumaticSketch sketch = new PneumaticSketch(this);

void setup() {
   sketch.setup();
}

void draw() {
   sketch.draw();
}

void controlEvent(ControlEvent theEvent) {
   sketch.controlEvent(theEvent);
}