#ifndef AETHER_ROUTER_HPP
#define AETHER_ROUTER_HPP

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

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
        router(
                boost::shared_ptr<boost::asio::io_service>,
                hades::connection&
                );
    };
}

#endif

