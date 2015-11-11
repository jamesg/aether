#ifndef AETHER_OPENWEATHERMAP_CITY_TO_LOCATION_HPP
#define AETHER_OPENWEATHERMAP_CITY_TO_LOCATION_HPP

#include "atlas/http/client.hpp"
#include "styx/element.hpp"

namespace hades
{
    class connection;
}

namespace aether
{
    struct location;

    namespace openweathermap
    {
        /*!
         * \brief Use the Open Weather Map API to convert a city name to
         * latitude/longitude location.
         */
        void city_to_location(
                hades::connection& conn,
                boost::shared_ptr<boost::asio::io_service> io,
                const std::string& city_name,
                boost::function<void(location)> success,
                boost::function<void(std::string)> failure
                );
    }
}

#endif

