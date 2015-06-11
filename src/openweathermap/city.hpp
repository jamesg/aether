#ifndef AETHER_OPENWEATHERMAP_CITY_HPP
#define AETHER_OPENWEATHERMAP_CITY_HPP

#include "styx/object.hpp"

#include "openweathermap/coord.hpp"

namespace aether
{
    namespace openweathermap
    {
        struct city :
            public styx::object
        {
            city(styx::element& e) :
                styx::object(e)
            {
            }

            int& id()
            {
                return get_int("id");
            }

            std::string& name()
            {
                return get_string("name");
            }

            /*!
             * \brief Two-letter country code.
             */
            std::string& country()
            {
                return get_string("country");
            }

            int& population()
            {
                return get_int("population");
            }

            openweathermap::coord coord()
            {
                return openweathermap::coord(get_element("coord"));
            };
        };
    }
}

#endif

