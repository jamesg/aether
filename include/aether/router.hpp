#ifndef AETHER_ROUTER_HPP
#define AETHER_ROUTER_HPP

#include "atlas/http/server/application_router.hpp"

namespace hades
{
    class connection;
}
namespace aether
{
    class router : public atlas::http::application_router
    {
    public:
        router(hades::connection&);
    };
}

#endif

