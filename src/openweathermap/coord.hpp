#ifndef AETHER_OPENWEATHERMAP_COORD_HPP
#define AETHER_OPENWEATHERMAP_COORD_HPP

#include "styx/object.hpp"

namespace aether
{
    namespace openweathermap
    {
        /*!
         * \brief A pair lof latitude/longitude coordinates.
         */
        struct coord :
            public styx::object
        {
            coord(styx::element& e) :
                styx::object(e)
            {
            }

            double& lat()
            {
                return get_double("lat");
            }

            double& lon()
            {
                return get_double("lon");
            }
        };
    }
}

#endif

