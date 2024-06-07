# R4_Touch

This library enables the Capacitive Touch Sensing Unit (CTSU) on the Arduino UNO-R4.  

# Hardware

To use this library you will need to add a capacitor between the TSCAP pin and ground.  The location of the TSCAP pin depends on which board you are using.<br><br>
On the Minima the TSCAP pin is pin 10<br>
On the R4-WiFi the TSCAP pin is pin 7<br><br>
The recommended size of the capacitor is 10nF but isn't critical.  Smaller capacitors tend to give more stable readings but anything up to 100nF should be fine. 

Touch pads should be connected directly to the pins with wires.  Be careful where you run your wires and where you have ground planes in your build.  
<br>

# Supported Pins

Not all pins on the R4 support capacitive sensing.<br>
On the Minima the supported pins are 0, 1, 2, 3, 8, 9, 11, 13, A1, A2<br>
On the WiFi the supported pins are 0, 1, 2, 3, 6, 8, 9, 11, 12, A1, A2<br>
On both boards the LOVE pin is also supported.  To use the LOVE pin, pass 20 for the pin number. 

# Code

The library contains a class called `TouchSensor`.  

The constructor for a `TouchSensor` object takes no arguments.  

There is a `begin(pin, threshold)` function that must be called for each sensor in `setup()` to initialize the sensor.  The arguments are `int pin` which sets the pin to be used, and `uint16_t threshold` which sets the threshold for determining touches.  

The `read()` function returns true if the sensor is touched, otherwise false.  The raw reading from the touch unit will be compared to the threshold value for the sensor to determine if the sensor is touched or not.  Raw values greater than the threshold value indicate a touch.  

The `readRaw()` function will get the raw reading from the unit.  This can be handy to help determine what to use for the threshold if the default values don't work.  

The `readReference()` function will get the raw reading from the reference counter.  This can be needed when tuning sensors. 

The `read()`, `readRaw()`, and `readReference()` functions DO NOT trigger a measurement of the sensor by themselves.  They are only returning the last read values.  In free-running mode this will always be a recent number since the code is restarting itself.  Otherwise you must use `TouchSensor::start()` or `TouchSensor::startSingle()` to trigger a new measurement.  

There are also `setThreshold(uint16_t)` and `getThreshold()` functions for the threshold value if you need to change or access it. 

Start the capacitive touch unit by calling the static method `TouchSensor::start()`.  This will start the unit in free-running mode so that each time the unit finishes a measurement it starts a new one. 

If you would like to run the unit once then call with an argument of false.  The static method `TouchSensor::startSingle()` will start a single measurement for all attached sensors.  The method blocks until all sensors are read.  Each sensor takes around 400 microseconds with default settings.  

There is a static method `TouchSensor::stop()` that will stop the CTSU but retain all settings. 

You can attach a callback function to be called at the end of each measurement cycle with the `attachCallback(callback)` function.  This function must take no arguments and return void.  The function will be called from the CTSU_FN interrupt handler before the next measurement is started.  


# Example 
```
#include "R4_Touch.h"

  //  Two arguments are pin number and threshold
  TouchSensor sensor1();
  TouchSensor sensor2();

void setup() {

  Serial.begin(115200);
  while(!Serial);
  
  // Call begin for each sensor
  sensor1.begin(2, 10000);
  sensor2.begin(3, 10000);

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

# Settings

There are several settings that can be made for each individual pin.

* `setMeasurementCount(uint8_t)` - sets the number of times the measure pulse will be repeated.  Limited to 0-63
* `setSensorOffset(uint16_t)` - sets the sensor offset.  Limited to 0-1023.
* `setIcoCurrentAdjust(uint8_t)` - sets the current adjustment for the ICO.  Limited to 0-255.
* `setIcoGain(ctsu_ico_gain_t)` - set ico gain percent.  Choose from CTSU_ICO_GAIN_100, CTSU_ICO_GAIN_66, CTSU_ICO_GAIN_50, or CTSU_ICO_GAIN_40.
* `setClockDiv(ctsu_clock_div_t)` - set the clock divider.  The CTSU uses PCLCKB which is set to system clock / 2.

Settings for the clock divider are:
  CTSU_CLOCK_DIV_2,
  CTSU_CLOCK_DIV_4,
  CTSU_CLOCK_DIV_6,
  CTSU_CLOCK_DIV_8,
  CTSU_CLOCK_DIV_10,
  CTSU_CLOCK_DIV_12,
  CTSU_CLOCK_DIV_14,
  CTSU_CLOCK_DIV_16,
  CTSU_CLOCK_DIV_18,
  CTSU_CLOCK_DIV_20,
  CTSU_CLOCK_DIV_22,
  CTSU_CLOCK_DIV_24,
  CTSU_CLOCK_DIV_26,
  CTSU_CLOCK_DIV_28,
  CTSU_CLOCK_DIV_30,
  CTSU_CLOCK_DIV_32,
  CTSU_CLOCK_DIV_34,
  CTSU_CLOCK_DIV_36,
  CTSU_CLOCK_DIV_38,
  CTSU_CLOCK_DIV_40,
  CTSU_CLOCK_DIV_42,
  CTSU_CLOCK_DIV_44,
  CTSU_CLOCK_DIV_46,
  CTSU_CLOCK_DIV_48,
  CTSU_CLOCK_DIV_50,
  CTSU_CLOCK_DIV_52,
  CTSU_CLOCK_DIV_54,
  CTSU_CLOCK_DIV_56,
  CTSU_CLOCK_DIV_58,
  CTSU_CLOCK_DIV_60,
  CTSU_CLOCK_DIV_62,
  CTSU_CLOCK_DIV_64