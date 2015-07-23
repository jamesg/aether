#include "radio.hpp"

#include "lcd.hpp"
#include "measure.hpp"

// JSONRPC id of the last outgoing request.
int last_request_id;

namespace
{
    char incoming_buffer[INCOMING_SIZE];
    int incoming_index;

    // Callback to use when a JSONRPC reply matching the last request is received.
    sensor::success_callback_type success_callback;

    // Callback to use when a JSONRPC request has failed.
    sensor::error_callback_type error_callback;

}

void sensor::reset_callbacks()
{
    last_request_id = INVALID_LAST_REQUEST_ID;
    success_callback = 0;
    error_callback = 0;
}

void sensor::radio_enable()
{
    if(RADIO_ENABLE_PIN > -1)
    {
        pinMode(RADIO_ENABLE_PIN, OUTPUT);
        digitalWrite(RADIO_ENABLE_PIN, HIGH);
    }
    Serial.begin(RADIO_BAUDRATE);
}

void sensor::radio_disable()
{
    if(RADIO_ENABLE_PIN > -1)
        digitalWrite(RADIO_ENABLE_PIN, LOW);
}

void sensor::send_status()
{
    // Enable the radio module.
    radio_enable();

    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "status";
    JsonArray& params = root.createNestedArray("params");
    params.add(sensor_id);
    root["id"] = last_request_id = 1;
    jsonrpc_request(root, &send_status_received, &send_status_error);
}

void sensor::send_status_received(JsonObject& result)
{
    const char *status = result["result"];
    callback_timer.after(1000, &send_moisture);
}

void sensor::send_status_error(const error_type err, const char *error)
{
    if(err == TIMEOUT)
        callback_timer.after(0, &short_wait);
    else
        callback_timer.after(0, &long_wait);
}

void sensor::send_ready_to_receive()
{
    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "ready_to_receive";
    JsonArray& params = root.createNestedArray("params");
    params.add(sensor_id);
    root["id"] = 1;
    jsonrpc_request(
        root,
        &send_ready_to_receive_received,
        &send_ready_to_receive_error
    );
}

void sensor::send_ready_to_receive_received(JsonObject& result)
{
    bool ready = result["result"];
    if(ready)
        callback_timer.after(0, &send_moisture);
    else
        callback_timer.after(0, &long_wait);
}

void sensor::send_ready_to_receive_error(error_type err, const char*)
{
    if(err == TIMEOUT)
        callback_timer.after(0, &short_wait);
    else
        callback_timer.after(0, &long_wait);
}

void sensor::send_moisture()
{
    int moisture_val;
    if(decode_moisture(moisture_val))
    {
        json_buffer_type json_buffer;
        JsonObject& root = json_buffer.createObject();
        root["method"] = "record_moisture";
        JsonArray& params = root.createNestedArray("params");
        params.add(sensor_id);
        params.add(moisture_val);
        root["id"] = 2;
        jsonrpc_request(root, &send_moisture_received, &send_moisture_error);
    }
    else
    {
        callback_timer.after(0, &send_temperature);
    }
}

void sensor::send_moisture_received(JsonObject& result)
{
    bool status = result["result"];
    callback_timer.after(1000, &send_temperature);
}

void sensor::send_moisture_error(const error_type err, const char *error)
{
    if(err == TIMEOUT)
        callback_timer.after(0, &short_wait);
    else
        callback_timer.after(0, &long_wait);
}

void sensor::send_temperature()
{
    if(ds18b20_temperature != NAN)
    {
        json_buffer_type json_buffer;
        JsonObject& root = json_buffer.createObject();
        root["method"] = "record_temperature";
        JsonArray& params = root.createNestedArray("params");
        params.add(sensor_id);
        params.add(ds18b20_temperature, 2);
        root["id"] = 3;
        jsonrpc_request(root, &send_temperature_received, &send_temperature_error);
    }
    else
    {
        callback_timer.after(0, &request_location);
    }
}

void sensor::send_temperature_received(JsonObject& result)
{
    bool status = result["result"];
    callback_timer.after(1000, &request_location);
}

void sensor::send_temperature_error(const error_type err, const char *error)
{
    if(err == TIMEOUT)
        callback_timer.after(0, &short_wait);
    else
        callback_timer.after(0, &long_wait);
}

void sensor::request_location()
{
    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "location";
    JsonArray& params = root.createNestedArray("params");
    root["id"] = 4;
    jsonrpc_request(root, &location_received, &location_error);
}

void sensor::location_received(JsonObject& result)
{
    const char *location = result["result"];
    strncpy(lcd_location_s, location, sizeof(lcd_location_s));
    callback_timer.after(1000, &request_cname);
}

void sensor::location_error(error_type err, const char*)
{
    if(err == TIMEOUT)
        callback_timer.after(0, &short_wait);
    else
        callback_timer.after(0, &long_wait);
}

void sensor::request_cname()
{
    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "variety_cname";
    JsonArray& params = root.createNestedArray("params");
    params.add(sensor_id);
    root["id"] = 5;
    jsonrpc_request(root, &cname_received, &cname_error);
}

void sensor::cname_received(JsonObject& result)
{
    const char *cname = result["result"];
    strncpy(lcd_variety_s, cname, sizeof(lcd_variety_s));
    callback_timer.after(0, &request_phase);
}

void sensor::cname_error(error_type err, const char*)
{
    if(err == TIMEOUT)
        callback_timer.after(0, &short_wait);
    else
        callback_timer.after(0, &long_wait);
}

void sensor::request_phase()
{
    json_buffer_type json_buffer;
    JsonObject& root = json_buffer.createObject();
    root["method"] = "phase";
    JsonArray& params = root.createNestedArray("params");
    params.add(sensor_id);
    root["id"] = 6;
    jsonrpc_request(root, &phase_received, &phase_error);
}

void sensor::phase_received(JsonObject& result)
{
    const char *phase_desc = result["result"];
    strncpy(lcd_phase_s, phase_desc, sizeof(lcd_phase_s));
    callback_timer.after(0, &long_wait);
}

void sensor::phase_error(error_type err, const char*)
{
    if(err == TIMEOUT)
        callback_timer.after(0, &short_wait);
    else
        callback_timer.after(0, &long_wait);
}

void sensor::long_wait()
{
    radio_disable();
    // One minute.
    callback_timer.after(60000, &send_status);
}

void sensor::short_wait()
{
    callback_timer.after(1000, &send_ready_to_receive);
}

void sensor::jsonrpc_request(
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

void sensor::handle_jsonrpc_result(JsonObject& result)
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

void sensor::handle_jsonrpc_timeout()
{
    error_callback_type cb = error_callback;
    reset_callbacks();
    if(cb)
        cb(TIMEOUT, 0);
}

void sensor::check_incoming_message()
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
