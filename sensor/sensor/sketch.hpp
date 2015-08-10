#ifndef AETHER_SENSOR_SKETCH_HPP
#define AETHER_SENSOR_SKETCH_HPP

#include "ArduinoJson.h"

#include "timer.hpp"

namespace sensor
{
    const int SOIL_MOISTURE_PIN = -1;

    const int TEMPERATURE_SENSOR_DIVIDER_R = 1000;
    const int TEMPERATURE_SENSOR_PIN = 9;

    const int DS18B20_PIN = 11;

    extern unsigned char sensor_id;
    extern timer callback_timer;

    // The last temperature measured from the DS18B20 sensor.
    extern float ds18b20_temperature;

    // String to be printed as the sensor is starting in configuration mode.
    extern const char lcd_reset_s[];

    // String to be printed as the sensor is starting for normal operations.
    extern const char lcd_startup_s[];
}

extern "C"
{
    void setup();
    void loop();
}

#endif
