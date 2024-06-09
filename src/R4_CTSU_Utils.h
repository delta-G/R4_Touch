/*

R4_CTSU_Utils.h  --  Backing functions for the TouchSensor class
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

#ifndef R4_TOUCH_UTILS_H
#define R4_TOUCH_UTILS_H

#if !defined(ARDUINO_UNOR4_WIFI) && !defined(ARDUINO_UNOR4_MINIMA)
#error Sorry, this library only works on the Arduino UNO-R4 Minima and Arduino UNO-R4 WiFi
#endif

#include "Arduino.h"

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

struct ctsu_pin_settings_t
{
  ctsu_clock_div_t div;
  ctsu_ico_gain_t gain;
  uint8_t ref_current;
  uint16_t offset;
  uint8_t count;
};

void stopTouchMeasurement();

void startTouchMeasurement(bool fr = true);
bool touchMeasurementReady();
void setTouchMode(const uint8_t);
uint16_t touchRead(const uint8_t);
uint16_t touchReadReference(const uint8_t);

void setTouchPinClockDiv(const uint8_t, const ctsu_clock_div_t);
void setTouchPinIcoGain(const uint8_t, const ctsu_ico_gain_t);
void setTouchPinReferenceCurrent(const uint8_t, const uint8_t);
void setTouchPinMeasurementCount(const uint8_t, const uint8_t);
void setTouchPinSensorOffset(const uint8_t, const uint16_t);
void applyTouchPinSettings(const uint8_t, const ctsu_pin_settings_t &);
ctsu_pin_settings_t getTouchPinSettings(const uint8_t);

void attachMeasurementEndCallback(fn_callback_ptr_t);

#endif // R4_TOUCH_UTILS_H