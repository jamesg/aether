#ifndef SENSOR_MEASURE_HPP
#define SENSOR_MEASURE_HPP

#include "OneWire.h"

extern OneWire ds;

// Look up the temperature indicated by the given resistance across a KTY81/220
// temperature sensor.
float kty81_lookup(int resistance);

bool decode_moisture(int& out);

// Get the resistance across the temperature sensor pins.
bool kty81_resistance(int& out);

// Check if the sensor should start in configuration/reset mode (there is a
// jumper installed on the temperature sensor pins).
bool config_mode();

// Read the temperature from the KTY81 sensor.
bool decode_temperature(float& out);

bool set_ds18b20_mode();
void request_ds18b20_temperature();
void store_ds18b20_temperature();

#endif
