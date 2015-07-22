#ifndef SENSOR_RADIO_HPP
#define SENSOR_RADIO_HPP

#include "sketch.hpp"

void reset_callbacks();

void radio_enable();
void radio_disable();

void send_status();
void send_status_received(JsonObject&);
void send_status_error(error_type, const char*);

void send_ready_to_receive();
void send_ready_to_receive_received(JsonObject&);
void send_ready_to_receive_error(error_type, const char*);

// Send the soil moisture reading, or schedule send_temperature if the sensor is
// not ready.
void send_moisture();
void send_moisture_received(JsonObject&);
void send_moisture_error(error_type, const char*);

// Send the temperature reading, or schedule request_location if the sensor is
// no ready.
void send_temperature();
void send_temperature_received(JsonObject&);
void send_temperature_error(error_type, const char*);

void request_location();
void location_received(JsonObject&);
void location_error(error_type, const char*);

void request_cname();
void cname_received(JsonObject&);
void cname_error(error_type, const char*);

void request_phase();
void phase_received(JsonObject&);
void phase_error(error_type, const char *);

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

void check_incoming_message();

#endif
