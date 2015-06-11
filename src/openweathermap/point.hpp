#ifndef AETHER_OPENWEATHERMAP_POINT_HPP
#define AETHER_OPENWEATHERMAP_POINT_HPP

#include "styx/object.hpp"

namespace aether
{
    namespace openweathermap
    {
        struct point :
            public styx::object
        {
            point(styx::element& e) :
                styx::object(e)
            {
            }

            /*!
             * \brief Date of this forecast as a Unix time.
             */
            int& dt()
            {
                return get_int("dt");
            }
            double& temp()
            {
                return styx::object(get_element("main"))
                    .get_double("temp");
            }
            double& temp_min()
            {
                return styx::object(get_element("main"))
                    .get_double("temp_min");
            }
            double& temp_max()
            {
                return styx::object(get_element("main"))
                    .get_double("temp_max");
            }
            double& pressure()
            {
                return styx::object(get_element("main"))
                    .get_double("pressure");
            }
            double& humidity()
            {
                return styx::object(get_element("main"))
                    .get_double("humidity");
            }
            std::string& weather_main()
            {
                return styx::object(get_element("weather"))
                    .get_string("main");
            }
            std::string& weather_description()
            {
                return styx::object(get_element("weather"))
                    .get_string("description");
            }
            int& clouds()
            {
                return styx::object(get_element("clouds"))
                    .get_int("all");
            }
            double& wind_speed()
            {
                return styx::object(get_element("wind"))
                    .get_double("speed");
            }
            double& wind_deg()
            {
                return styx::object(get_element("wind"))
                    .get_double("deg");
            }
        };
    }
}

#endif

