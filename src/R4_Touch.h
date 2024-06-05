/*

R4_CTSU.h  --  Capacitive Touch Sensing for Arduino UNO-R4
     Copyright (C) 2023  David C.

     This program is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>.

     */

#ifndef R4_TOUCH_H
#define R4_TOUCH_H

#if !defined(ARDUINO_UNOR4_WIFI) && !defined(ARDUINO_UNOR4_MINIMA)
#error Sorry, this library only works on the Arduino UNO-R4 Minima and Arduino UNO-R4 WiFi
#endif

#include "Arduino.h"
// #include "EventLinkInterrupt.h"
#include "r_dtc.h"
#include "IRQManager.h"

#define NOT_A_TOUCH_PIN 255

struct ctsu_pin_info_t
{
  uint8_t ts_num;
  uint8_t chac_idx;
  uint8_t chac_val;
};

void setupCTSU();
void setupDTC();
void startCTSUmeasure();

void startTouchMeasurement(bool fr = true);
bool touchMeasurementReady();
void setTouchMode(int);
uint16_t touchRead(int);

#define DEFAULT_TOUCH_THRESHOLD 19000

class TouchSensor {
private:
  uint8_t _pin;
  uint16_t _threshold;
  TouchSensor();

public:
  TouchSensor(uint8_t aPin, uint16_t aThresh) : _pin(aPin), _threshold(aThresh) {}
  void begin();
  bool read();
  uint16_t readRaw();

  void setThreshold(uint16_t t) { _threshold = t; }
  uint16_t getThreshold() { return _threshold; }

  static void start();
  static void startSingle();
  static bool ready();
};

#endif // R4_TOUCH_H
