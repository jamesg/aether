#include "sketch.hpp"

#include "Arduino.h"
#include "EEPROM.h"
#include "ArduinoJson.h"

#include "lcd.hpp"
#include "measure.hpp"
#include "radio.hpp"
#include "timer.hpp"

// Network-unique id of this sensor.
int sensor_id = 0;

// HD44780 connections:
// RS - 6
// E  - 5
// D4 - 4
// D5 - 3
// D6 - 2
// D7 - 9
LiquidCrystal lcd(6, 5, 4, 3, 2, 9);

timer callback_timer;

float ds18b20_temperature;

const char lcd_startup_s[] = "Aether";

char lcd_temperature_s[LCD_LINE_SIZE];
char lcd_phase_s[LCD_LINE_SIZE];
char lcd_variety_s[LCD_LINE_SIZE];
char lcd_location_s[LCD_LINE_SIZE];

void setup()
{
    reset_callbacks();
    ds18b20_temperature = NAN;

    lcd_temperature_s[0] = '\0';
    lcd_phase_s[0] = '\0';
    lcd_variety_s[0] = '\0';
    lcd_location_s[0] = '\0';

    lcd.begin(LCD_LINE_SIZE, LCD_LINE_COUNT);

    if(config_mode())
    {
        reset_eeprom();
        return;
    }

    callback_timer.after(0, &print_startup);

    // Set up the DS18B20 sensor.
    if(set_ds18b20_mode())
        request_ds18b20_temperature();

    // Start the state machine.
    callback_timer.after(100, &send_status);
}

void reset_eeprom()
{
    EEPROM[0] = 255;
}

void store_ds18b20_temperature()
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
    callback_timer.update();
    check_incoming_message();
}
