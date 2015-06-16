#ifndef AETHER_REQUEST_FORECAST_HPP
#define AETHER_REQUEST_FORECAST_HPP

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "openweathermap/forecast.hpp"

namespace aether
{
    namespace openweathermap
    {
        /*!
         * \brief Request a forecast from the Open Weather Map API.
         *
         * \param lat Latitude of the location for which to request the
         * forecast.
         * \param lon Longitude of the location for which to request the
         * forecast.
         * \param success Function to call, with the forecast as a parameter,
         * when the forecast is retrieved successfully.
         * \param failure Function to call, with an error message as a
         * parameter, when an error is encountered.
         */
        void retrieve_forecast(
                boost::shared_ptr<boost::asio::io_service> io,
                float lat,
                float lon,
                boost::function<void(openweathermap::forecast&)> success,
                boost::function<void(const std::string&)> failure
                );
    }
}

#endif

