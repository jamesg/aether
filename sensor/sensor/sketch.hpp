#ifndef AETHER_SENSOR_SKETCH_HPP
#define AETHER_SENSOR_SKETCH_HPP

#include "ArduinoJson.h"

#include "timer.hpp"

#define INCOMING_SIZE 200
#define INVALID_LAST_REQUEST_ID -1

namespace sensor
{
    enum error_type {TIMEOUT, JSONRPC};

    const bool USE_ARDUINO_RADIO_SHIELD = false;

    const int RADIO_ENABLE_PIN = 10;
    const int RADIO_BAUDRATE = 9600;

    const int SOIL_MOISTURE_PIN = -1;

    const int TEMPERATURE_SENSOR_DIVIDER_R = 1000;
    const int TEMPERATURE_SENSOR_PIN = 3;

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
