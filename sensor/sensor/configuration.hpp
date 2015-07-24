#ifndef SENSOR_CONFIGURATION_HPP
#define SENSOR_CONFIGURATION_HPP

namespace sensor
{
    struct configuration
    {
        static const constexpr unsigned char INVALID_SENSOR_ID = 0;

        // This sensor's network-unique id, assigned by the server.
        unsigned char sensor_id;

        configuration();
    };

    const unsigned char CONFIGURATION_VERSION = 0;

    unsigned char configuration_version();

    void configuration_reset();
    void configuration_save(const configuration&);
    void configuration_load(configuration&);
}


#endif
