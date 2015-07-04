#include "sketch.hpp"

#include "Arduino.h"
#include "EEPROM.h"
#include "ArduinoJson.h"
#include "LiquidCrystal.h"

#include "timer.hpp"

#define INCOMING_SIZE 200
#define INVALID_LAST_REQUEST_ID -1

const int TEMPERATURE_SENSOR_DIVIDER_R = 4700;

// JSONRPC id of the last outgoing request.
int last_request_id;

// Callback to use when a JSONRPC reply matching the last request is received.
success_callback_type success_callback;

// Callback to use when a JSONRPC request has failed.
error_callback_type error_callback;

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
// OneWire ds(10);

// Timer to wait for a reply from the server.
timer callback_timer;

char incoming_buffer[INCOMING_SIZE];
int incoming_index;

void reset_callbacks()
{
    callback_timer.reset();

    last_request_id = INVALID_LAST_REQUEST_ID;
    success_callback = 0;
    error_callback = 0;
}

void setup()
{
    last_request_id = INVALID_LAST_REQUEST_ID;
    success_callback = 0;
    incoming_index = 0;

    lcd.begin(16,2);
    lcd.print("Plant Monitor");

    // Enable the radio module.
    pinMode(8, OUTPUT);
    digitalWrite(8, HIGH);
    Serial.begin(9600);

    // Start the state machine.
    callback_timer.after(100, &send_status);
}

int decode_moisture()
{
    int moisture_val = analogRead(0);
    return moisture_val;
}

float decode_temperature()
{
    int reading = analogRead(5);

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

    float ratio = (float)reading / 1024.0f;
    // Compute the resistance of the temperature sensor in Ohms.  Will be
    // roughly 1000-3000 Ohms.
    int resistance = ((float)TEMPERATURE_SENSOR_DIVIDER_R * ratio) /
        (1.0f - ratio);
    return kty81_lookup(resistance);
}

float kty81_lookup(int resistance) {
    int lookup_table[] = {
        // -50 to 50 degrees C in 10 degree increments
        1030, 1135, 1247, 1367, 1495, 1630, 1772, 1922, 2080, 2245, 2417
    };

    // Extreme cases.
    if(resistance > lookup_table[10])
        return 50.0f;
    if(resistance < lookup_table[0])
        return -50.0f;

    int i = 0;
    while(i < sizeof(lookup_table) - 1 && resistance > lookup_table[i + 1])
        ++i;

    // resistance is between lookup_table[i] and lookup_table[i + 1]
    return -50.0f + (i * 10.0f) +
            (
                (float)(resistance - lookup_table[i]) /
                (float)(lookup_table[i + 1] - lookup_table[i])
            ) * 10.0f;
}

void log_string(const char *str)
{
    lcd.clear();
    lcd.print(str);
}

void send_status()
{
    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "status";
    JsonArray& params = root.createNestedArray("params");
    root["id"] = last_request_id = 1;
    jsonrpc_request(root, &send_status_received, &send_status_error);
}

void send_status_received(JsonObject& result)
{
    log_string("rec status");
    const char *status = result["result"];
    if(status)
        log_string(status);
    else
        log_string("invalid status");
    callback_timer.after(1000, &send_moisture);
}

void send_status_error(const error_type err, const char *error)
{
    log_string("send status err");

    callback_timer.after(0, &send_status);
}

void send_ready_to_receive()
{
    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "ready_to_receive";
    JsonArray& params = root.createNestedArray("params");
    root["id"] = 1;
    jsonrpc_request(root, &send_status_received, &send_status_error);
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
    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "record_moisture";
    JsonArray& params = root.createNestedArray("params");
    int moisture_val = decode_moisture();
    params.add(moisture_val);
    root["id"] = 2;
    jsonrpc_request(root, &send_moisture_received, &send_moisture_error);
}

void send_moisture_received(JsonObject& result)
{
    log_string("rec moisture");
    bool status = result["result"];
    if(status)
        log_string("moisture success");
    else
        log_string("moisture error");
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
    float temperature = decode_temperature();
    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "record_temperature";
    JsonArray& params = root.createNestedArray("params");
    params.add(temperature, 1);
    root["id"] = 3;
    jsonrpc_request(root, &send_temperature_received, &send_temperature_error);
}

void send_temperature_received(JsonObject& result)
{
    log_string("rec temp");
    bool status = result["result"];
    if(status)
        log_string("temp success");
    else
        log_string("temp error");
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
    if(location)
        log_string(location);
    else
        log_string("location error");
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
    if(cname)
        log_string(cname);
    else
        log_string("cname unknown");
    callback_timer.after(0, &long_wait);
}

void cname_error(error_type err, const char*)
{
    if(err == TIMEOUT)
        callback_timer.after(0, &short_wait);
    else
        callback_timer.after(0, &long_wait);
}

void long_wait()
{
    callback_timer.after(5000, &send_status);
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
            log_string("no match");
            // Error.
            reset_callbacks();
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
    char buf[16];
    snprintf(buf, sizeof(buf), "err %lu", millis());
    log_string(buf);
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
                log_string("buffer full");
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
