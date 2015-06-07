#ifndef AETHER_SKETCH_HPP
#define AETHER_SKETCH_HPP

#include "ArduinoJson.h"

typedef StaticJsonBuffer<100> json_buffer_type;
typedef void(*success_callback_type)(JsonObject&);
typedef void(*error_callback_type)(const char*);

void log_string(const char *str);

void send_status();
void send_moisture();
void send_temperature();
void send_status_received(JsonObject&);
void send_status_error(const char*);
void send_moisture_received(JsonObject&);
void send_moisture_error(const char*);
void send_temperature_received(JsonObject&);
void send_temperature_error(const char*);

/*
 * \brief Call a JSONRPC method on the server.
 */
void jsonrpc_call(
        JsonObject& request,
        success_callback_type *success,
        error_callback_type *error
        );

void handle_jsonrpc_result(JsonObject&);
void handle_jsonrpc_error();

/*
 * \brief Read the temperature from the DS18B20 sensor.
 *
 * \note This function takes at least 100ms to execute.
 *
 * \param out Pointer to temperature value.
 *
 * \return 0 on success, 1 on failure.
 */
int decode_temperature(float *out);

/*
 * \brief Get the moisture value.
 *
 * \return A value between 0 and 1023 indicating the current moisture level.
 */
int decode_moisture();

extern "C"
{
    void setup();
    void loop();
}

#endif

