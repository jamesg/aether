#ifndef SENSOR_MEASURE_HPP
#define SENSOR_MEASURE_HPP

#include "OneWire.h"

namespace sensor
{
    extern OneWire ds;

    // Look up the temperature indicated by the given resistance across a KTY81/220
    // temperature sensor.
    float kty81_lookup(int resistance);

    // Get the moisture level as an arbitrary reading in the range [0, 1023].
    bool decode_moisture(int& out);

    // Get the resistance across the temperature sensor pins.
    bool kty81_resistance(int& out);

    // Check if the sensor should start in configuration/reset mode (there is a
    // jumper installed on the temperature sensor pins).
    bool config_mode();

    // Read the temperature from the KTY81 sensor.
    bool decode_temperature(float& out);

    // Configure the DS18B20 sensor to measure with the required precision.
    // Returns true if the sensor was configured, false if it could not be
    // detected.
    bool set_ds18b20_mode();

    // Start a small state machine requesting the temperature at the DS18B20
    // sensor every few seconds.
    void request_ds18b20_temperature();
    void store_ds18b20_temperature();
}

#endif
