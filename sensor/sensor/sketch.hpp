#ifndef AETHER_SENSOR_SKETCH_HPP
#define AETHER_SENSOR_SKETCH_HPP

#include "ArduinoJson.h"

enum error_type {TIMEOUT, JSONRPC};

typedef StaticJsonBuffer<100> json_buffer_type;
typedef void(*success_callback_type)(JsonObject&);
typedef void(*error_callback_type)(error_type, const char*);

// Look up the temperature indicated by the given resistance across a KTY81/220
// temperature sensor.
float kty81_lookup(int resistance);

void log_string(const char *str);

void send_status();
void send_status_received(JsonObject&);
void send_status_error(error_type, const char*);

void send_ready_to_receive();
void send_ready_to_receive_received(JsonObject&);
void send_ready_to_receive_error(error_type, const char*);

void send_moisture();
void send_moisture_received(JsonObject&);
void send_moisture_error(error_type, const char*);

void send_temperature();
void send_temperature_received(JsonObject&);
void send_temperature_error(error_type, const char*);

void request_location();
void location_received(JsonObject&);
void location_error(error_type, const char*);

void request_cname();
void cname_received(JsonObject&);
void cname_error(error_type, const char*);

void long_wait();
void short_wait();

// Call a JSONRPC method on the server.
void jsonrpc_request(
        JsonObject& request,
        success_callback_type success,
        error_callback_type error
        );

// Process a JSONRPC result that has been received from the server.
void handle_jsonrpc_result(JsonObject&);

// Take action if no JSONRPC reply has been received from the server.
void handle_jsonrpc_timeout();

// Read the temperature from the KTY81 sensor.
float decode_temperature();

int decode_moisture();

extern "C"
{
    void setup();
    void loop();
}

#endif
