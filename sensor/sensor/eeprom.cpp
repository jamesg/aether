#include "eeprom.hpp"

#include <Arduino.h>
#include "Wire.h"

i2c_eeprom::i2c_eeprom(const int i2c_id) :
    m_id(i2c_id)
{
}

void i2c_eeprom::write(
        unsigned int eeaddress,
        char *data,
        unsigned int data_len
        )
{
    unsigned int address = eeaddress;
    unsigned int page_space = int(((eeaddress/64) + 1)*64)-eeaddress;
    unsigned int num_writes;
    unsigned char first_write_size;
    unsigned char last_write_size;
    unsigned char write_size;


    if(page_space>16)
    {
        first_write_size=page_space-((page_space/16)*16);
        if (first_write_size==0) first_write_size=16;
    }
    else
        first_write_size=page_space;

    if (data_len>first_write_size)
        last_write_size = (data_len-first_write_size)%16;

    if (data_len>first_write_size)
        num_writes = ((data_len-first_write_size)/16)+2;
    else
        num_writes = 1;

    int i = 0;
    for(int page = 0; page < num_writes; ++page)
    {
        int write_size =
            (page == 0) ? first_write_size :
            (page == (num_writes - 1)) ? last_write_size :
            16;

        Wire.beginTransmission(m_id);
        Wire.write((int)((address) >> 8));
        Wire.write((int)((address) & 0xFF));

        int counter = 0;
        do
        {
            Wire.write((byte) data[i]);
            i++;
            counter++;
        } while((counter<write_size));
        Wire.endTransmission();
        address+=write_size;
        delay(6);
    }
}

void i2c_eeprom::read(
        unsigned int eeaddress,
        char *data,
        unsigned int data_len
        )
{
    Wire.beginTransmission(m_id);
    Wire.write((int)(eeaddress >> 8));   // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();

    Wire.requestFrom(m_id, data_len);

    unsigned char i = 0;
    while(Wire.available())
        data[i++] = Wire.read();
}

i2c_map::i2c_map(id_type id) :
    i2c_eeprom(id)
{
}

