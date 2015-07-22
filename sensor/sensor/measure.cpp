#include "measure.hpp"

#include "OneWire.h"

#include "sketch.hpp"

OneWire ds(DS18B20_PIN);

bool decode_moisture(int& out)
{
    if(SOIL_MOISTURE_PIN < 0)
        return false;
    out = analogRead(SOIL_MOISTURE_PIN) / (1023.0/100.0);
}

bool kty81_resistance(int& resistance)
{
    if(TEMPERATURE_SENSOR_PIN < 0)
        return false;

    int reading = analogRead(TEMPERATURE_SENSOR_PIN);

    // Voltage Divider Diagram.
    // Vin -- Rconst -(input)- Rtemp -- Gnd
    // The temperature sensor is between the input pin and ground.

    // Starting with the voltage divider equation, work out the resistance of
    // the sensor.

    // Vm = Vin * (Rtemp / (Rtemp + Rconst) )
    // Vm / Vin = Rtemp / (Rtemp + Rconst)                        Divide by Vin
    // (Vm / Vin) * (Rtemp + Rconst) = Rtemp       Multiply by (Rtemp + Rconst)
    // Rtemp * (Vm / Vin) + Rconst * (Vm / Vin) = Rtemp               Factorise
    // Rtemp - Rtemp * (Vm / Vin) = Rconst * (Vm / Vin)     Collect Rtemp terms
    // Rtemp * ( 1 - (Vm / Vin) ) = Rconst * (Vm / Vin)               Factorise
    // Rtemp = (Rconst * (Vm / Vin) ) / (1 - (Vm / Vin) )   Divide to get Rtemp

    float ratio = (float)reading / 1023.0f;

    // A ratio close to 1 indicates and open circuit.  The resistance is too
    // large to estimate with any accuracy.
    if(ratio > 0.99f)
        return false;
    // Compute the resistance of the temperature sensor in Ohms.  Will be
    // roughly 1000-3000 Ohms.
    resistance = ((float)TEMPERATURE_SENSOR_DIVIDER_R * ratio) /
        (1.0f - ratio);
    return true;
}

bool config_mode()
{
    int resistance = 0;
    if(!kty81_resistance(resistance))
        return false;
    // If the resistance is less than 100 Ohms, take it that a jumper has been
    // installed.
    return (resistance < 100);
}

bool decode_temperature(float& out)
{
    int resistance = 0;
    if(!kty81_resistance(resistance))
        return false;
    out = kty81_lookup(resistance);
    return true;
}

float kty81_lookup(int resistance) {
    int lookup_table[] = {
        // -50 to 50 degrees C in 10 degree increments
        1030, 1135, 1247, 1367, 1495, 1630, 1772, 1922, 2080, 2245, 2417
    };

    // Extreme cases.
    if(resistance >= lookup_table[10])
        return 50.0f;
    if(resistance < lookup_table[0])
        return -50.0f;

    int i = 0;
    while(i < sizeof(lookup_table) - 1 && resistance >= lookup_table[i + 1])
        ++i;

    // resistance is between lookup_table[i] and lookup_table[i + 1]
    return -50.0f + (i * 10.0f) +
            (
                (float)(resistance - lookup_table[i]) /
                (float)(lookup_table[i + 1] - lookup_table[i])
            ) * 10.0f;
}

bool set_ds18b20_mode()
{
    byte ds18b20_addr[8];
    ds.reset_search();
    bool ds18b20_found = ds.search(ds18b20_addr);
    return ds18b20_found;
}

void request_ds18b20_temperature()
{
    if(!ds.reset())
        return;
    ds.skip();
    // Instruct the DS18B20 to start the conversion.
    ds.write(0x44, 1);
    // The conversion can take 750ms.
    callback_timer.after(1000, &store_ds18b20_temperature);
}
