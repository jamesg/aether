#include "sketch.hpp"

#include "Arduino.h"
#include "EEPROM.h"
#include "ArduinoJson.h"
#include "LiquidCrystal.h"
#include "OneWire.h"

#include "timer.hpp"

#define INCOMING_SIZE 200
#define INVALID_LAST_REQUEST_ID -1

// JSONRPC id of the last outgoing request.
int last_request_id;

// Callback to use when a JSONRPC reply matching the last request is received.
success_callback_type success_callback;

// Callback to use when a JSONRPC request has failed.
error_callback_type error_callback;

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
OneWire ds(10);

// Timer to wait for a reply from the server.
timer callback_timer;

char incoming_buffer[INCOMING_SIZE];
int incoming_index;

bool ds18b20_found;
byte ds18b20_addr[8];

void reset_callbacks()
{
    callback_timer.reset();

    last_request_id = INVALID_LAST_REQUEST_ID;
    success_callback = 0;
    error_callback = 0;
}

void set_ds18b20_mode()
{
    // Set the conversion to 9 bits.

    if(!ds.reset())
        return;
    ds.skip();
    ds.write(0x4e, 1);
    // Write first two bytes (temperature).
    ds.write(0, 1);
    ds.write(0, 1);
    // Write the configuration register.
    ds.write(0x1f, 1);
}

void setup()
{
    last_request_id = INVALID_LAST_REQUEST_ID;
    success_callback = 0;
    incoming_index = 0;

    ds.reset_search();
    ds18b20_found = ds.search(ds18b20_addr);

    if(ds18b20_found)
        set_ds18b20_mode();

    lcd.begin(16,2);
    lcd.print("Plant Monitor");

    // Enable the radio module.
    pinMode(8, OUTPUT);
    digitalWrite(8, HIGH);
    Serial.begin(9600);

    // Start the state machine.
    callback_timer.after(0, &send_status);
}

int decode_moisture()
{
    int moisture_val = analogRead(0);
    return moisture_val;
}

int decode_temperature(float *out)
{
    if(!ds.reset())
        return 1;
    ds.skip();
    // Start the temperature conversion.
    ds.write(0x44, 1);

    // The datasheet specifies 93.75ms to do the conversion.
    delay(100);

    if(!ds.reset())
        return 1;
    ds.skip();
    // Read the scratchpad.
    ds.write(0xBE);

    byte i;
    byte data[9];

    // The scratchpad is 9 bytes.  The last byte is a CRC.
    for(i = 0; i < 9; i++) {
        data[i] = ds.read();
    }

    int sign_bit = data[1] & 0x80;
    int reading = ((data[1] & 0x07) << 5) | ((data[0] >> 3) & 0x1f);

    // On power up, the temperature register is set to 85 degrees.  If this
    // temperature is read, assume it was not read correctly.
    if(data[0] == 0x50 && data[1] == 0x05)
        return 1;

    *out = (sign_bit ? -1.0 : 1.0) * 0.5 * (float)reading;

    return 0;
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
    float temperature;
    if(decode_temperature(&temperature) == 0)
    {
        json_buffer_type json_buffer;
        JsonObject& root = json_buffer.createObject();
        root["method"] = "record_temperature";
        JsonArray& params = root.createNestedArray("params");
        params.add(temperature, 1);
        root["id"] = 3;
        jsonrpc_request(root, &send_temperature_received, &send_temperature_error);
    }
    else
    {
        callback_timer.after(0, &long_wait);
    }
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
