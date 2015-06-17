#ifndef AETHER_WEATHER_ROUTER_HPP
#define AETHER_WEATHER_ROUTER_HPP

#include "atlas/http/server/router.hpp"

namespace hades
{
    class connection;
}
namespace aether
{
    /*!
     * \brief Router for all requests to /api/weather, including requests for
     * weather forecasts.
     */
    class weather_router : public atlas::http::router
    {
    public:
        explicit weather_router(hades::connection&);
    };
}

#endif

