#include "configuration.hpp"

#include "EEPROM.h"

sensor::configuration::configuration() :
    sensor_id(INVALID_SENSOR_ID)
{
}

unsigned char sensor::configuration_version()
{
    return EEPROM[0];
}

void sensor::configuration_reset()
{
    EEPROM[0] = CONFIGURATION_VERSION;
    configuration config;
    EEPROM.put(1, config);
}

void sensor::configuration_save(const configuration& config)
{
    EEPROM.put(1, config);
}

void sensor::configuration_load(configuration& out)
{
    EEPROM.get(1, out);
}
