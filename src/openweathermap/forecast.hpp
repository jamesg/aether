#ifndef AETHER_OPENWEATHERMAP_FORECAST_HPP
#define AETHER_OPENWEATHERMAP_FORECAST_HPP

#include "styx/object.hpp"

#include "openweathermap/city.hpp"

namespace aether
{
    namespace openweathermap
    {
        struct forecast :
            public styx::object
        {
            forecast()
            {
            }

            forecast(const styx::element& e) :
                styx::object(e)
            {
            }

            openweathermap::city city()
            {
                return openweathermap::city(get_element("city"));
            };

            /*!
             * \brief HTTP-like status code.
             *
             * 404 corresponds to forecast/location not found.
             */
            int& cod()
            {
                return get_int("cod");
            }

            /*!
             * \brief Status message.  Normally empty.
             */
            std::string& message()
            {
                return get_string("message");
            }

            /*!
             * \brief Count; number of data points (?).
             */
            int& cnt()
            {
                return get_int("cnt");
            }

            /*!
             * \brief List of forecast points.
             */
            styx::list& list()
            {
                return get_list("list");
            }
        };
    }
}

#endif

