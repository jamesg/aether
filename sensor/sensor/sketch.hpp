#ifndef AETHER_SENSOR_SKETCH_HPP
#define AETHER_SENSOR_SKETCH_HPP

#include "ArduinoJson.h"
#include "LiquidCrystal.h"

#include "timer.hpp"

#define INCOMING_SIZE 200
#define INVALID_LAST_REQUEST_ID -1

enum error_type {TIMEOUT, JSONRPC};

typedef StaticJsonBuffer<100> json_buffer_type;
typedef void(*success_callback_type)(JsonObject&);
typedef void(*error_callback_type)(error_type, const char*);

extern int sensor_id;
extern timer callback_timer;
extern LiquidCrystal lcd;

const int LCD_LINE_SIZE = 16;
const int LCD_LINE_COUNT = 2;

const bool USE_ARDUINO_RADIO_SHIELD = false;

const int RADIO_ENABLE_PIN = 10;
const int RADIO_BAUDRATE = 9600;

const int SOIL_MOISTURE_PIN = -1;

const int TEMPERATURE_SENSOR_DIVIDER_R = 1000;
const int TEMPERATURE_SENSOR_PIN = 3;

const int DS18B20_PIN = 11;

extern float ds18b20_temperature;

extern const char lcd_startup_s[];

extern char lcd_temperature_s[LCD_LINE_SIZE];
extern char lcd_phase_s[LCD_LINE_SIZE];
extern char lcd_variety_s[LCD_LINE_SIZE];
extern char lcd_location_s[LCD_LINE_SIZE];

void reset_eeprom();

extern "C"
{
    void setup();
    void loop();
}

#endif
