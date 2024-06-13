
#include "R4_Touch.h"

// Create an instance of the TouchSensor class.
TouchSensor mySensor;

// set tuning values for the sensor  You will to determine these settins for your particular sensor.
// These settings worked for the LOVE pin on my Minima but yours may be different. 
ctsu_pin_settings_t mySettings = {.div=CTSU_CLOCK_DIV_18, .gain=CTSU_ICO_GAIN_100, .ref_current=0, .offset=75, .count=3};
// Threshold value is determined during tuning
unsigned int myThreshold = 1500;

// variable for the pin number
int mySensorPin = 20;
// variable for loop to remember last state
bool lastTouch = false;


void setup() {

  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n\n *** Simple_Touch_Example.ino ***\n\n");

  // Call begin with the pin number and threshold.
  // You will have to determine this threshold by experimentation or by tuning your sensor.
  // using 20 for the pin number selects the love pin.
  mySensor.begin(mySensorPin, myThreshold);
  // apply settings
  mySensor.applyPinSettings(mySettings);
  // start the capcitive touch unit
  TouchSensor::start();
}

void loop() {
  // read() compares the sensor raw value to the threshold
  // and returns true if touched otherwise false. 
  bool touch = mySensor.read();
  // Regular edge detection just like with digitalRead.
  if (touch != lastTouch) {
    if (touch) {
      Serial.println("Touched");
    } else {
      Serial.println("Released");
    }
    lastTouch = touch;
  }
}
