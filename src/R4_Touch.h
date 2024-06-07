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

#define NUM_ARDUINO_PINS 21
#if defined(ARDUINO_UNOR4_MINIMA)
#define NUM_CTSU_PINS 11
#elif defined(ARDUINO_UNOR4_WIFI)
#define NUM_CTSU_PINS 12
#endif
#define NOT_A_TOUCH_PIN 255

typedef void (*fn_callback_ptr_t)();

typedef enum e_ctsu_ico_gain
{
  CTSU_ICO_GAIN_100 = 0,
  CTSU_ICO_GAIN_66 = 1,
  CTSU_ICO_GAIN_50 = 2,
  CTSU_ICO_GAIN_40 = 3
} ctsu_ico_gain_t;

typedef enum e_ctsu_clock_div
{
  CTSU_CLOCK_DIV_2 = 0,
  CTSU_CLOCK_DIV_4 = 1,
  CTSU_CLOCK_DIV_6 = 2,
  CTSU_CLOCK_DIV_8 = 3,
  CTSU_CLOCK_DIV_10 = 4,
  CTSU_CLOCK_DIV_12 = 5,
  CTSU_CLOCK_DIV_14 = 6,
  CTSU_CLOCK_DIV_16 = 7,
  CTSU_CLOCK_DIV_18 = 8,
  CTSU_CLOCK_DIV_20 = 9,
  CTSU_CLOCK_DIV_22 = 10,
  CTSU_CLOCK_DIV_24 = 11,
  CTSU_CLOCK_DIV_26 = 12,
  CTSU_CLOCK_DIV_28 = 13,
  CTSU_CLOCK_DIV_30 = 14,
  CTSU_CLOCK_DIV_32 = 15,
  CTSU_CLOCK_DIV_34 = 16,
  CTSU_CLOCK_DIV_36 = 17,
  CTSU_CLOCK_DIV_38 = 18,
  CTSU_CLOCK_DIV_40 = 19,
  CTSU_CLOCK_DIV_42 = 20,
  CTSU_CLOCK_DIV_44 = 21,
  CTSU_CLOCK_DIV_46 = 22,
  CTSU_CLOCK_DIV_48 = 23,
  CTSU_CLOCK_DIV_50 = 24,
  CTSU_CLOCK_DIV_52 = 25,
  CTSU_CLOCK_DIV_54 = 26,
  CTSU_CLOCK_DIV_56 = 27,
  CTSU_CLOCK_DIV_58 = 28,
  CTSU_CLOCK_DIV_60 = 29,
  CTSU_CLOCK_DIV_62 = 30,
  CTSU_CLOCK_DIV_64 = 31
} ctsu_clock_div_t;

struct ctsu_pin_info_t
{
  uint8_t ts_num;
  uint8_t chac_idx;
  uint8_t chac_val;
};

void setupCTSU();
void setupDTC();
void startCTSUmeasure();
void stopCTSU();

void startTouchMeasurement(bool fr = true);
bool touchMeasurementReady();
void setTouchMode(int);
uint16_t touchRead(int);
uint16_t getReferenceCount(int);

void setTouchPinClockDiv(int, ctsu_clock_div_t);
void setTouchPinIcoGain(int, ctsu_ico_gain_t);
void setTouchPinIcoCurrentAdjust(int, uint8_t);
void setTouchPinMeasurementCount(int, uint8_t);
void setTouchPinSensorOffset(int, uint16_t);

void attachMeasurementEndCallback(fn_callback_ptr_t);

#define DEFAULT_TOUCH_THRESHOLD 19000

class TouchSensor
{
private:
  uint8_t _pin;
  uint16_t _threshold;

public:
  void begin(int aPin, uint16_t aThresh)
  {
    _pin = aPin;
    _threshold = aThresh;
    setTouchMode(_pin);
  }
  bool read() { return (touchRead(_pin) > _threshold); }
  uint16_t readRaw() { return touchRead(_pin); }
  uint16_t readReference() { return getReferenceCount(_pin); }

  void setThreshold(uint16_t t) { _threshold = t; }
  uint16_t getThreshold() { return _threshold; }

  void setClockDiv(ctsu_clock_div_t s) { setTouchPinClockDiv(_pin, s); }
  void setIcoGain(ctsu_ico_gain_t s) { setTouchPinIcoGain(_pin, s); }
  void setIcoCurrentAdjust(uint8_t s) { setTouchPinIcoCurrentAdjust(_pin, s); }
  void setMeasurementCount(uint8_t s) { setTouchPinMeasurementCount(_pin, s); }
  void setSensorOffset(uint16_t s) { setTouchPinSensorOffset(_pin, s); }

  static void start() { startTouchMeasurement(); }
  static void stop() { stopCTSU(); }
  static void startSingle()
  {
    startTouchMeasurement(false);
    while (!touchMeasurementReady())
      ;
  }
  static void attachCallback(fn_callback_ptr_t cb) { attachMeasurementEndCallback(cb); };
};

#endif // R4_TOUCH_H
