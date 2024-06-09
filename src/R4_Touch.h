/*

R4_Touch.h  --  Capacitive Touch Sensing for Arduino UNO-R4
     Copyright (C) 2024  David C.

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
#include "R4_CTSU_Utils.h"

#define DEFAULT_TOUCH_THRESHOLD 19000

class TouchSensor
{
private:
  uint8_t _pin;
  uint16_t _threshold;

public:
  void begin(const uint8_t aPin, const uint16_t aThresh);
  bool read();
  uint16_t readRaw();
  uint16_t readReference();

  void setThreshold(const uint16_t t);
  uint16_t getThreshold();

  void setClockDiv(const ctsu_clock_div_t s);
  void setIcoGain(const ctsu_ico_gain_t s);
  void setReferenceCurrent(const uint8_t s);
  void setMeasurementCount(const uint8_t s);
  void setSensorOffset(const uint16_t s);
  void applyPinSettings(const ctsu_pin_settings_t);
  ctsu_pin_settings_t getPinSettings();

  static void start();
  static void stop();
  static void startSingle();
  static void attachCallback(fn_callback_ptr_t cb);
};

#endif // R4_TOUCH_H
