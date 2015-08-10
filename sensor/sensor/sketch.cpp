#include "sketch.hpp"

#include "Arduino.h"
#include "EEPROM.h"
#include "ArduinoJson.h"

#include "configuration.hpp"
#include "measure.hpp"
#include "radio.hpp"
#include "timer.hpp"

// Network-unique id of this sensor.
unsigned char sensor::sensor_id = sensor::configuration::INVALID_SENSOR_ID;

timer sensor::callback_timer;

float sensor::ds18b20_temperature = NAN;

const char sensor::lcd_startup_s[] = "Aether";

void setup()
{
    sensor::reset_callbacks();

    // If the configuration mode jumper is set or the configuration version
    // stored in EEPROM does not match the version required by this program.
    if(
        sensor::config_mode() ||
        sensor::configuration_version() != sensor::CONFIGURATION_VERSION
    )
    {
        // Reset the EEPROM with the default configuration.
        sensor::configuration_reset();
        delay(1000);
    }

    // Set up the DS18B20 sensor.
    if(sensor::set_ds18b20_mode())
        sensor::request_ds18b20_temperature();

    sensor::configuration config;
    sensor::configuration_load(config);
    // The sensor id will be used often; take a copy.
    sensor::sensor_id = config.sensor_id;
    if(sensor::sensor_id == sensor::configuration::INVALID_SENSOR_ID)
        // Register the sensor with the server.
        sensor::start_registration();
    else
        // Start the routine transmission state machine.
        sensor::start_transmission();
}

void sensor::store_ds18b20_temperature()
{
    if(!ds.reset())
    {
        ds18b20_temperature = NAN;
        return;
    }
    ds.skip();
    ds.write(0xBE);
    byte i;
    byte data[9];
    for(i = 0; i < 9; ++i)
        data[i] = ds.read();

    int sign_bit = data[1] & 0x80;
    int reading = ((data[1] & 0x07) << 8) + data[0];

    // On power up, the temperature register is set to 85 degrees.  If this
    // temperature is read, assume it was not read correctly.
    if(data[0] == 0x50 && data[1] == 0x05)
    {
        ds18b20_temperature = NAN;
        return;
    }

    ds18b20_temperature = (sign_bit ? -1.0 : 1.0) * ((float)reading / 16.0);

    callback_timer.after(10000, &request_ds18b20_temperature);
}

void loop()
{
    sensor::callback_timer.update();
    sensor::check_incoming_message();
}
