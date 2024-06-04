# R4_Touch

This library enables the Capacitive Touch Sensing Unit (CTSU) on the Arduino UNO-R4.  

# Hardware

To use this library you will need to add a capacitor between the TSCAP pin and ground.  The location of the TSCAP pin depends on which board you are using.<br><br>
On the Minima the TSCAP pin is pin 10<br>
On the R4-WiFi the TSCAP pin is pin 7<br><br>
The recommended size of the capacitor is 10nF but isn't critical.  Smaller capacitors tend to give more stable readings but anything up to 100nF should be fine. 

Touch pads should be connected directly to the pins with wires.  Be careful where you run your wires and where you have ground planes in your build.  
<br>

# Code

The library contains a class called `TouchSensor`.  

The constructor for a `TouchSensor` object takes two arguments.  The first is the pin number to be used and the second is a threshold value for determining a touch.     

There is a `begin()` function that must be called for each pin in `setup()`.

The `read()` function returns true if the sensor is touched, otherwise false.  The raw reading from the touch unit will be compared to the threshold value for the sensor to determine if the sensor is touched or not.  Raw values greater than the threshold value indicate a touch.  

The `readRaw()` function will get the raw reading from the unit.  This can be handy to help determine what to use for the threshold if the default values don't work.  

Start the capacitive touch unit by calling `TouchSensor::start()`.  This will start the unit in free-running mode so that each time the unit finishes a measurement it starts a new one. 

If you would like to run the unit once then call with an argument of false.  `TouchSensor::startSingle()` will start a single measurement for all attached sensors.  The method blocks until all sensors are read.  Each sensor takes around 400 microseconds with default settings.  

The `read()` and `readRaw()` functions DO NOT trigger a measurement of the sensor.  They are only returning the last read values.  In free-running mode this will always be a recent number since the code is restarting itself.  Otherwise you must use `TouchSensor::start()` or `TouchSensor::startSingle()` to trigger a new measurement.  

There are also `setThreshold(uint16_t)` and `getThreshold()` functions for the threshold value if you need to change or access it. 

# Example 
```
#include "R4_Touch.h"

  //  Two arguments are pin number and threshold
  TouchSensor sensor1(2, 19000);
  TouchSensor sensor2(3, 19000);

void setup() {

  Serial.begin(115200);
  while(!Serial);
  
  // Call begin for each sensor
  sensor1.begin();
  sensor2.begin();

  TouchSensor::start();  // start the unit in free-running mode
  Serial.println("End Setup");

}

void loop() {

  static int count = 0;

  Serial.println();
  Serial.print("-- -");
  Serial.print(count++);
  Serial.print("  ");
  Serial.print(sensor1.read()); 
  Serial.print("   ");
  Serial.print(sensor2.read());

  delay(500);

}
```

<br><br>