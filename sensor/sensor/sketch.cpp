#include "sketch.hpp"

#include "Arduino.h"
#include "EEPROM.h"
#include "ArduinoJson.h"
#include "LiquidCrystal.h"
#include "OneWire.h"

#include "timer.hpp"

#define INCOMING_SIZE 200
#define INVALID_LAST_REQUEST_ID -1

const int LCD_LINE_SIZE = 16;
const int LCD_LINE_COUNT = 2;

const bool USE_ARDUINO_RADIO_SHIELD = false;

const int RADIO_ENABLE_PIN = 10;
const int RADIO_BAUDRATE = 9600;

const int SOIL_MOISTURE_PIN = -1;

const int TEMPERATURE_SENSOR_DIVIDER_R = 1000;
const int TEMPERATURE_SENSOR_PIN = 3;

const int DS18B20_PIN = 11;

// JSONRPC id of the last outgoing request.
int last_request_id;

// Callback to use when a JSONRPC reply matching the last request is received.
success_callback_type success_callback;

// Callback to use when a JSONRPC request has failed.
error_callback_type error_callback;

// HD44780 connections:
// RS - 6
// E  - 5
// D4 - 4
// D5 - 3
// D6 - 2
// D7 - 9
LiquidCrystal lcd(6, 5, 4, 3, 2, 9);

OneWire ds(DS18B20_PIN);

// Timer to wait for a reply from the server.
timer callback_timer;

char incoming_buffer[INCOMING_SIZE];
int incoming_index;

float ds18b20_temperature;

const char lcd_startup_s[] = "Aether";

char lcd_temperature_s[LCD_LINE_SIZE];
char lcd_phase_s[LCD_LINE_SIZE];
char lcd_variety_s[LCD_LINE_SIZE];
char lcd_location_s[LCD_LINE_SIZE];

void reset_callbacks()
{
    last_request_id = INVALID_LAST_REQUEST_ID;
    success_callback = 0;
    error_callback = 0;
}

void setup()
{
    reset_callbacks();
    ds18b20_temperature = NAN;

    lcd_temperature_s[0] = '\0';
    lcd_phase_s[0] = '\0';
    lcd_variety_s[0] = '\0';
    lcd_location_s[0] = '\0';

    lcd.begin(LCD_LINE_SIZE, LCD_LINE_COUNT);
    callback_timer.after(0, &print_startup);

    // Set up the DS18B20 sensor.
    if(set_ds18b20_mode())
        request_ds18b20_temperature();

    // Start the state machine.
    callback_timer.after(100, &send_status);
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

void radio_enable()
{
    if(RADIO_ENABLE_PIN > -1)
    {
        pinMode(RADIO_ENABLE_PIN, OUTPUT);
        digitalWrite(RADIO_ENABLE_PIN, HIGH);
    }
    Serial.begin(RADIO_BAUDRATE);
}

void radio_disable()
{
    if(RADIO_ENABLE_PIN > -1)
        digitalWrite(RADIO_ENABLE_PIN, LOW);
}

void print_startup()
{
    lcd.clear();
    lcd.print(lcd_startup_s);
    callback_timer.after(4000, &print_location);
}

void print_location()
{
    lcd.clear();
    lcd.print("Location");
    lcd.setCursor(0, 1);
    lcd.print(lcd_location_s);
    callback_timer.after(4000, &print_phase);
}

void print_phase()
{
    lcd.clear();
    lcd.print("Phase");
    lcd.setCursor(0, 1);
    lcd.print(lcd_phase_s);
    callback_timer.after(4000, &print_temperature);
}

void print_temperature()
{
    if(ds18b20_temperature != NAN)
        dtostrf(ds18b20_temperature, 2, 1, lcd_temperature_s);
    lcd.clear();
    lcd.print("Temperature");
    lcd.setCursor(0, 1);
    lcd.print(lcd_temperature_s);
    callback_timer.after(4000, &print_variety);
}

void print_variety()
{
    lcd.clear();
    lcd.print("Variety");
    lcd.setCursor(0, 1);
    lcd.print(lcd_variety_s);
    callback_timer.after(4000, &print_location);
}

bool decode_moisture(int& out)
{
    if(SOIL_MOISTURE_PIN < 0)
        return false;
    out = analogRead(SOIL_MOISTURE_PIN) / (1023.0/100.0);
}

bool decode_temperature(float& out)
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
    // Compute the resistance of the temperature sensor in Ohms.  Will be
    // roughly 1000-3000 Ohms.
    int resistance = ((float)TEMPERATURE_SENSOR_DIVIDER_R * ratio) /
        (1.0f - ratio);
    out = kty81_lookup(resistance);
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

void send_status()
{
    // Enable the radio module.
    radio_enable();

    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "status";
    JsonArray& params = root.createNestedArray("params");
    root["id"] = last_request_id = 1;
    jsonrpc_request(root, &send_status_received, &send_status_error);
}

void send_status_received(JsonObject& result)
{
    const char *status = result["result"];
    callback_timer.after(1000, &send_moisture);
}

void send_status_error(const error_type err, const char *error)
{
    if(err == TIMEOUT)
        callback_timer.after(0, &short_wait);
    else
        callback_timer.after(0, &long_wait);
}

void send_ready_to_receive()
{
    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "ready_to_receive";
    JsonArray& params = root.createNestedArray("params");
    root["id"] = 1;
    jsonrpc_request(
        root,
        &send_ready_to_receive_received,
        &send_ready_to_receive_error
    );
}

void send_ready_to_receive_received(JsonObject& result)
{
    bool ready = result["result"];
    if(ready)
        callback_timer.after(0, &send_moisture);
    else
        callback_timer.after(0, &long_wait);
}

void send_ready_to_receive_error(error_type err, const char*)
{
    if(err == TIMEOUT)
        callback_timer.after(0, &short_wait);
    else
        callback_timer.after(0, &long_wait);
}

void send_moisture()
{
    int moisture_val;
    if(decode_moisture(moisture_val))
    {
        json_buffer_type json_buffer;
        JsonObject& root = json_buffer.createObject();
        root["method"] = "record_moisture";
        JsonArray& params = root.createNestedArray("params");
        params.add(moisture_val);
        root["id"] = 2;
        jsonrpc_request(root, &send_moisture_received, &send_moisture_error);
    }
    else
    {
        callback_timer.after(0, &send_temperature);
    }
}

void send_moisture_received(JsonObject& result)
{
    bool status = result["result"];
    callback_timer.after(1000, &send_temperature);
}

void send_moisture_error(const error_type err, const char *error)
{
    if(err == TIMEOUT)
        callback_timer.after(0, &short_wait);
    else
        callback_timer.after(0, &long_wait);
}

void send_temperature()
{
    if(ds18b20_temperature != NAN)
    {
        json_buffer_type json_buffer;
        JsonObject& root = json_buffer.createObject();
        root["method"] = "record_temperature";
        JsonArray& params = root.createNestedArray("params");
        params.add(ds18b20_temperature, 2);
        root["id"] = 3;
        jsonrpc_request(root, &send_temperature_received, &send_temperature_error);
    }
    else
    {
        callback_timer.after(0, &request_location);
    }
}

void send_temperature_received(JsonObject& result)
{
    bool status = result["result"];
    callback_timer.after(1000, &request_location);
}

void send_temperature_error(const error_type err, const char *error)
{
    if(err == TIMEOUT)
        callback_timer.after(0, &short_wait);
    else
        callback_timer.after(0, &long_wait);
}

void request_location()
{
    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "location";
    JsonArray& params = root.createNestedArray("params");
    root["id"] = 4;
    jsonrpc_request(root, &location_received, &location_error);
}

void location_received(JsonObject& result)
{
    const char *location = result["result"];
    strncpy(lcd_location_s, location, sizeof(lcd_location_s));
    callback_timer.after(1000, &request_cname);
}

void location_error(error_type err, const char*)
{
    if(err == TIMEOUT)
        callback_timer.after(0, &short_wait);
    else
        callback_timer.after(0, &long_wait);
}

void request_cname()
{
    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "variety_cname";
    JsonArray& params = root.createNestedArray("params");
    root["id"] = 5;
    jsonrpc_request(root, &cname_received, &cname_error);
}

void cname_received(JsonObject& result)
{
    const char *cname = result["result"];
    strncpy(lcd_variety_s, cname, sizeof(lcd_variety_s));
    callback_timer.after(0, &request_phase);
}

void cname_error(error_type err, const char*)
{
    if(err == TIMEOUT)
        callback_timer.after(0, &short_wait);
    else
        callback_timer.after(0, &long_wait);
}

void request_phase()
{
    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "phase";
    JsonArray& params = root.createNestedArray("params");
    root["id"] = 6;
    jsonrpc_request(root, &phase_received, &phase_error);
}

void phase_received(JsonObject& result)
{
    const char *phase_desc = result["result"];
    strncpy(lcd_phase_s, phase_desc, sizeof(lcd_phase_s));
    callback_timer.after(0, &long_wait);
}

void phase_error(error_type err, const char*)
{
    if(err == TIMEOUT)
        callback_timer.after(0, &short_wait);
    else
        callback_timer.after(0, &long_wait);
}

void long_wait()
{
    radio_disable();
    // One minute.
    callback_timer.after(60000, &send_status);
}

void short_wait()
{
    callback_timer.after(1000, &send_ready_to_receive);
}

void jsonrpc_request(
        JsonObject& request,
        success_callback_type success,
        error_callback_type error
        )
{
    reset_callbacks();
    last_request_id = request["id"];
    success_callback = success;
    error_callback = error;
    request.printTo(Serial);
    Serial.println();
    // Allow five seconds for the server to reply.
    callback_timer.after(5000, &handle_jsonrpc_timeout);
}

void handle_jsonrpc_result(JsonObject& result)
{
    int jsonrpc_id = result["id"];
    // Ignore responses that do not match the last request sent.
    if(jsonrpc_id == last_request_id)
    {
        success_callback_type sc = success_callback;
        error_callback_type ec = error_callback;
        reset_callbacks();
        // Check for errors in the JSONRPC result.
        if((const char*)(result["error"]) == 0) {
            // Success.
            if(sc)
                sc(result);
        }
        else
        {
            // Error.
            const char *error_str = result["error"];
            if(ec)
                ec(JSONRPC, error_str);
        }
    }
}

void handle_jsonrpc_timeout()
{
    error_callback_type cb = error_callback;
    reset_callbacks();
    if(cb)
        cb(TIMEOUT, 0);
}

void check_incoming_message()
{
    while(Serial.available())
    {
        incoming_buffer[incoming_index] = Serial.read();
        // Incoming data from the radio will be conveniently null-terminated.
        if(incoming_buffer[incoming_index] == '\0')
        {
            json_buffer_type buffer;
            JsonObject& result = buffer.parseObject(incoming_buffer);
            handle_jsonrpc_result(result);
            incoming_index = 0;
        }
        else
        {
            incoming_index++;
            if(incoming_index >= INCOMING_SIZE)
            {
                incoming_index = 0;
                // If there is any serial input left, it will be nonsense.
                while(Serial.available())
                    Serial.read();
            }
        }
    }
}

void loop()
{
    callback_timer.update();
    check_incoming_message();
}
