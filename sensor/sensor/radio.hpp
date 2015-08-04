#ifndef SENSOR_RADIO_HPP
#define SENSOR_RADIO_HPP

#include "ArduinoJson.h"

namespace sensor
{
    enum error_type {TIMEOUT, JSONRPC};

    typedef StaticJsonBuffer<100> json_buffer_type;
    typedef void(*success_callback_type)(JsonObject&);
    typedef void(*error_callback_type)(error_type, const char*);
    typedef void(*transmit_function_type)();

    // Pin to power or a transistor supplying power to the radio.  -1 if the
    // radio is always powered.
    const int RADIO_ENABLE_PIN = 8;
    // Long required for 115200 baud.
    const long RADIO_BAUDRATE = 115200;

    // Time to wait for the radio module to come online after it is powered.
    const constexpr int RADIO_MODULE_POWER_MS = 100;

    // Time to wait after a valid response is received before transmitting the
    // next request.
    const constexpr int RADIO_BETWEEN_REQUESTS_MS = 100;

    // Attempt each transmission a maximum of MAX_TRANSMIT_ATTEMPTS before
    // giving up.
    const constexpr int MAX_TRANSMIT_ATTEMPTS = 5;

    // Time to allow for the server to reply before attempting transmission
    // again.
    const constexpr int TRANSMIT_TIMEOUT_MS = 1500;

    // Time to wait between routine transmissions (15 minutes).
    const constexpr long TRANSMIT_LONG_WAIT_MS = 900000;

    // Time to wait after an aborted transmission (1 minute).
    const constexpr int TRANSMIT_MEDIUM_WAIT_MS = 60000;

    void reset_callbacks();

    void radio_enable();
    void radio_disable();

    void start_registration();
    void start_transmission();

    void send_registration();
    void send_registration_received(JsonObject&);
    void send_registration_error(error_type, const char*);

    void send_status();
    void send_status_received(JsonObject&);
    void send_status_error(error_type, const char*);

    void send_ready_to_receive();
    void send_ready_to_receive_received(JsonObject&);
    void send_ready_to_receive_error(error_type, const char*);

    // Send the soil moisture reading, or schedule send_temperature if the soil
    // moisture sensor is not ready.
    void send_moisture();
    void send_moisture_received(JsonObject&);
    void send_moisture_error(error_type, const char*);

    // Send the temperature reading, or schedule request_location if the sensor is
    // no ready.
    void send_temperature();
    void send_temperature_received(JsonObject&);
    void send_temperature_error(error_type, const char*);

    // Wait for a long period before starting routine data transmission.
    void long_wait();
    // Wait for a short period before retrying a transmission.  Returns true if
    // transmit_function has been scheduled, false if there have already been
    // too many transmission attempts and the transmission will be abandoned.
    bool short_wait(transmit_function_type transmit_function);

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
}

#endif
