/*

R4_Touch.cpp  --  Capacitive Touch Sensing for Arduino UNO-R4
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

#include "R4_Touch.h"

void TouchSensor::begin(const uint8_t pin, const uint16_t threshold)
{
    _pin = pin;
    _threshold = threshold;
    setTouchMode(_pin);
}
bool TouchSensor::read() { return (touchRead(_pin) > _threshold); }
uint16_t TouchSensor::readRaw() { return touchRead(_pin); }
uint16_t TouchSensor::readReference() { return touchReadReference(_pin); }

void TouchSensor::setThreshold(const uint16_t t) { _threshold = t; }
uint16_t TouchSensor::getThreshold() { return _threshold; }

void TouchSensor::setClockDiv(const ctsu_clock_div_t s) { setTouchPinClockDiv(_pin, s); }
void TouchSensor::setIcoGain(const ctsu_ico_gain_t s) { setTouchPinIcoGain(_pin, s); }
void TouchSensor::setReferenceCurrent(const uint8_t s) { setTouchPinReferenceCurrent(_pin, s); }
void TouchSensor::setMeasurementCount(const uint8_t s) { setTouchPinMeasurementCount(_pin, s); }
void TouchSensor::setSensorOffset(const uint16_t s) { setTouchPinSensorOffset(_pin, s); }
void TouchSensor::applyPinSettings(const ctsu_pin_settings_t s) { applyTouchPinSettings(_pin, s); }
ctsu_pin_settings_t TouchSensor::getPinSettings() { return getTouchPinSettings(_pin); }

void TouchSensor::start() { startTouchMeasurement(); }
void TouchSensor::stop() { stopTouchMeasurement(); }
void TouchSensor::startSingle()
{
    startTouchMeasurement(false);
    while (!touchMeasurementReady())
        ;
}
void TouchSensor::attachCallback(fn_callback_ptr_t cb) { attachMeasurementEndCallback(cb); };